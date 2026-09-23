#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <signal.h>

#include "executor.h"
#include "builtin.h"
#include "../include/jobs.h"

int execute_command(Command *cmd)
{
    pid_t pid;
    int status;

    if (cmd == NULL || cmd->argc == 0)
        return -1;

    if (is_builtin(cmd))
        return execute_builtin(cmd);

    pid = fork();
    if (pid < 0) {
        perror("fork");
        return -1;
    }

    if (pid == 0) {
        setpgid(0, 0);

        if (cmd->input) {
            int fd = open(cmd->input, O_RDONLY);
            if (fd < 0) {
                perror("open input");
                _exit(EXIT_FAILURE);
            }
            dup2(fd, STDIN_FILENO);
            close(fd);
        }

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

        char *args[MAX_ARGS + 1];
        for (int i = 0; i < cmd->argc; i++)
            args[i] = cmd->argv[i];
        args[cmd->argc] = NULL;

        execvp(args[0], args);
        fprintf(stderr, "shellforge: command not found: %s\n", args[0]);
        _exit(127);
    }

    setpgid(pid, pid);

    if (cmd->background) {
        char command_string[MAX_JOB_COMMAND];
        int pos = 0;
        for (int i = 0; i < cmd->argc && pos < MAX_JOB_COMMAND - 2; i++) {
            int n = snprintf(command_string + pos,
                             MAX_JOB_COMMAND - pos,
                             "%s%s",
                             cmd->argv[i],
                             (i < cmd->argc - 1) ? " " : "");
            pos += n;
        }
        if (pos < MAX_JOB_COMMAND - 2) {
            command_string[pos++] = ' ';
            command_string[pos++] = '&';
            command_string[pos] = '\0';
        }

        int job_id = job_add(pid, command_string, JOB_RUNNING);
        if (job_id > 0)
            printf("[%d] %d\n", job_id, pid);
        return 0;
    }

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

int execute_pipeline(Command *head)
{
    if (head == NULL)
        return -1;

    int count = pipeline_count(head);

    if (count == 1)
        return execute_command(head);

    int prev_read = -1;
    Command *cmd = head;
    pid_t first_pid = -1;
    int is_background = head->background;
    pid_t pids[MAX_ARGS];
    int status;

    for (int i = 0; i < count; i++) {
        int pipefd[2] = {-1, -1};

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
            if (i == 0)
                setpgid(0, 0);
            else
                setpgid(0, first_pid);

            if (prev_read != -1) {
                dup2(prev_read, STDIN_FILENO);
                close(prev_read);
            }
            if (i < count - 1) {
                close(pipefd[0]);
                dup2(pipefd[1], STDOUT_FILENO);
                close(pipefd[1]);
            }

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

            char *args[MAX_ARGS + 1];
            for (int k = 0; k < cmd->argc; k++)
                args[k] = cmd->argv[k];
            args[cmd->argc] = NULL;

            execvp(args[0], args);
            fprintf(stderr, "shellforge: command not found: %s\n", args[0]);
            _exit(127);
        }

        if (i == 0) {
            first_pid = pid;
            setpgid(pid, pid);
        } else {
            setpgid(pid, first_pid);
        }

        pids[i] = pid;

        if (prev_read != -1) close(prev_read);
        if (i < count - 1)   close(pipefd[1]);
        prev_read = (i < count - 1) ? pipefd[0] : -1;

        cmd = cmd->next;
    }

    if (is_background) {
        char command_string[MAX_JOB_COMMAND];
        snprintf(command_string, sizeof(command_string), "%s ...", head->argv[0]);
        int job_id = job_add(first_pid, command_string, JOB_RUNNING);
        if (job_id > 0)
            printf("[%d] %d\n", job_id, first_pid);
        return 0;
    }

    int last_status = 0;
    for (int i = 0; i < count; i++) {
        waitpid(pids[i], &status, 0);
        if (i == count - 1 && WIFEXITED(status))
            last_status = WEXITSTATUS(status);
    }

    return last_status;
}
