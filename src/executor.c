#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>

#include "executor.h"
#include "builtin.h"

/*
 * Execute one command (builtin or external).
 * Returns: 0 success, -1 error, 1 exit shell
 */
int execute_command(Command *cmd)
{
    pid_t pid;
    int status;

    if (cmd == NULL || cmd->argc == 0)
        return -1;

    /* Builtin? Handle in-process */
    if (is_builtin(cmd))
        return execute_builtin(cmd);

    /* External command */
    pid = fork();
    if (pid < 0) {
        perror("fork");
        return -1;
    }

    if (pid == 0) {
        /* Child: handle input redirection */
        if (cmd->input) {
            int fd = open(cmd->input, O_RDONLY);
            if (fd < 0) {
                perror("open input");
                _exit(EXIT_FAILURE);
            }
            dup2(fd, STDIN_FILENO);
            close(fd);
        }

        /* Child: handle output redirection */
        if (cmd->output) {
            int flags = O_CREAT | O_WRONLY |
                        (cmd->append ? O_APPEND : O_TRUNC);
            int fd = open(cmd->output, flags, 0644);
            if (fd < 0) {
                perror("open output");
                _exit(EXIT_FAILURE);
            }
            dup2(fd, STDOUT_FILENO);
            close(fd);
        }

        /* Build args array */
        char *args[MAX_ARGS + 1];
        for (int i = 0; i < cmd->argc; i++)
            args[i] = cmd->argv[i];
        args[cmd->argc] = NULL;

        execvp(args[0], args);

        /* If execvp returns, it failed */
        fprintf(stderr, "shellforge: command not found: %s\n", args[0]);
        _exit(127);
    }

    /* Parent: wait for child */
    if (waitpid(pid, &status, 0) == -1) {
        perror("waitpid");
        return -1;
    }

    if (WIFEXITED(status))
        return WEXITSTATUS(status);

    if (WIFSIGNALED(status)) {
        fprintf(stderr, "Process terminated by signal %d\n",
                WTERMSIG(status));
        return -1;
    }

    return 0;
}

/*
 * Execute a pipeline: cmd1 | cmd2 | ... | cmdN
 * Uses pipe(), fork(), dup2(), waitpid().
 */
int execute_pipeline(Command *head)
{
    if (head == NULL)
        return -1;

    int count = pipeline_count(head);

    /* No pipe → single command */
    if (count == 1)
        return execute_command(head);

    /* Multi-command pipeline */
    int prev_read = -1;        /* read end of previous pipe */
    Command *cmd = head;
    pid_t last_pid = -1;
    int status = 0;

    for (int i = 0; i < count; i++) {
        int pipefd[2] = {-1, -1};

        /* Create a pipe for every command except the last */
        if (i < count - 1) {
            if (pipe(pipefd) < 0) {
                perror("pipe");
                return -1;
            }
        }

        pid_t pid = fork();
        if (pid < 0) {
            perror("fork");
            return -1;
        }

        if (pid == 0) {
            /* ---- CHILD ---- */

            /* Input: from previous pipe (unless first command) */
            if (prev_read != -1) {
                dup2(prev_read, STDIN_FILENO);
                close(prev_read);
            }

            /* Output: to current pipe (unless last command) */
            if (i < count - 1) {
                close(pipefd[0]);           /* close read end */
                dup2(pipefd[1], STDOUT_FILENO);
                close(pipefd[1]);
            }

            /* Apply file redirection if any (overrides pipe) */
            if (cmd->input) {
                int fd = open(cmd->input, O_RDONLY);
                if (fd >= 0) {
                    dup2(fd, STDIN_FILENO);
                    close(fd);
                }
            }
            if (cmd->output) {
                int flags = O_CREAT | O_WRONLY |
                            (cmd->append ? O_APPEND : O_TRUNC);
                int fd = open(cmd->output, flags, 0644);
                if (fd >= 0) {
                    dup2(fd, STDOUT_FILENO);
                    close(fd);
                }
            }

            /* Build args and exec */
            char *args[MAX_ARGS + 1];
            for (int k = 0; k < cmd->argc; k++)
                args[k] = cmd->argv[k];
            args[cmd->argc] = NULL;

            execvp(args[0], args);

            fprintf(stderr, "shellforge: command not found: %s\n", args[0]);
            _exit(127);
        }

        /* ---- PARENT ---- */

        /* Close previous read end (already handed to child) */
        if (prev_read != -1)
            close(prev_read);

        /* Close current pipe's write end (child owns it) */
        if (i < count - 1)
            close(pipefd[1]);

        /* Save read end for next command */
        prev_read = (i < count - 1) ? pipefd[0] : -1;

        if (i == count - 1)
            last_pid = pid;

        cmd = cmd->next;
    }

    /* Wait for all children */
    int last_status = 0;
    for (int i = 0; i < count; i++) {
        pid_t w = wait(&status);
        if (w == last_pid && WIFEXITED(status))
            last_status = WEXITSTATUS(status);
    }

    return last_status;
}
