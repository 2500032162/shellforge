#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/wait.h>

#include "builtin.h"
#include "../include/jobs.h"

/* =========================================================
   BUILTIN: cd
   ========================================================= */

static int builtin_cd(Command *cmd)
{
    const char *directory;

    if (cmd->argc == 1) {
        directory = getenv("HOME");
        if (directory == NULL) {
            fprintf(stderr, "cd: HOME not set\n");
            return -1;
        }
    }
    else if (cmd->argc == 2) {
        directory = cmd->argv[1];
    }
    else {
        fprintf(stderr, "cd: too many arguments\n");
        return -1;
    }

    if (chdir(directory) != 0) {
        perror("cd");
        return -1;
    }
    return 0;
}

/* =========================================================
   BUILTIN: pwd
   ========================================================= */

static int builtin_pwd(Command *cmd)
{
    char current_directory[4096];

    if (cmd->argc > 1) {
        fprintf(stderr, "pwd: too many arguments\n");
        return -1;
    }

    if (getcwd(current_directory, sizeof(current_directory)) == NULL) {
        perror("pwd");
        return -1;
    }

    printf("my self declared pwd\n");
    printf("Student ID: 2500032162\n");
    printf("%s\n", current_directory);
    return 0;
}

/* =========================================================
   BUILTIN: echo
   ========================================================= */

static int builtin_echo(Command *cmd)
{
    for (int i = 1; i < cmd->argc; i++) {
        printf("%s", cmd->argv[i]);
        if (i < cmd->argc - 1)
            printf(" ");
    }
    printf("\n");
    printf("my self declared pwd\n");
    printf("Student ID: 2500032162\n");
    printf("\n");
    return 0;
}

/* =========================================================
   BUILTIN: exit
   ========================================================= */

static int builtin_exit(Command *cmd)
{
    if (cmd->argc > 1) {
        fprintf(stderr, "exit: too many arguments\n");
        return -1;
    }
    return 1;
}

/* =========================================================
   BUILTIN: jobs
   ========================================================= */

static int builtin_jobs(Command *cmd)
{
    (void)cmd;
    jobs_print();
    return 0;
}

/* =========================================================
   BUILTIN: bg
   ========================================================= */

static int builtin_bg(Command *cmd)
{
    if (cmd->argc < 2) {
        fprintf(stderr, "bg: usage: bg <job_id>\n");
        return -1;
    }

    int job_id = atoi(cmd->argv[1]);
    job_t *job = job_find(job_id);
    if (!job) {
        fprintf(stderr, "bg: no such job: %d\n", job_id);
        return -1;
    }

    kill(-job->pgid, SIGCONT);
    job_continue(job->pgid);

    printf("[%d] %s &\n", job->job_id, job->command);
    return 0;
}

/* =========================================================
   BUILTIN: fg
   ========================================================= */

static int builtin_fg(Command *cmd)
{
    if (cmd->argc < 2) {
        fprintf(stderr, "fg: usage: fg <job_id>\n");
        return -1;
    }

    int job_id = atoi(cmd->argv[1]);
    job_t *job = job_find(job_id);
    if (!job) {
        fprintf(stderr, "fg: no such job: %d\n", job_id);
        return -1;
    }

    printf("%s\n", job->command);

    kill(-job->pgid, SIGCONT);
    job_continue(job->pgid);

    int status;
    waitpid(-job->pgid, &status, WUNTRACED);

    if (WIFEXITED(status) || WIFSIGNALED(status))
        job_remove(job_id);

    return 0;
}

/* =========================================================
   CHECK WHETHER COMMAND IS A BUILTIN
   ========================================================= */

int is_builtin(const Command *cmd)
{
    if (cmd == NULL || cmd->argc == 0)
        return 0;

    if (strcmp(cmd->argv[0], "cd") == 0)   return 1;
    if (strcmp(cmd->argv[0], "pwd") == 0)  return 1;
    if (strcmp(cmd->argv[0], "echo") == 0) return 1;
    if (strcmp(cmd->argv[0], "exit") == 0) return 1;
    if (strcmp(cmd->argv[0], "jobs") == 0) return 1;
    if (strcmp(cmd->argv[0], "fg") == 0)   return 1;
    if (strcmp(cmd->argv[0], "bg") == 0)   return 1;

    return 0;
}

/* =========================================================
   EXECUTE BUILTIN
   ========================================================= */

int execute_builtin(Command *cmd)
{
    if (cmd == NULL || cmd->argc == 0)
        return -1;

    int saved_stdout = -1;
    int saved_stdin = -1;

    if (cmd->output) {
        int flags = O_CREAT | O_WRONLY |
                    (cmd->append ? O_APPEND : O_TRUNC);
        int fd = open(cmd->output, flags, 0644);
        if (fd < 0) {
            perror("open output");
            return -1;
        }
        saved_stdout = dup(STDOUT_FILENO);
        dup2(fd, STDOUT_FILENO);
        close(fd);
    }

    if (cmd->input) {
        int fd = open(cmd->input, O_RDONLY);
        if (fd < 0) {
            perror("open input");
            if (saved_stdout != -1) {
                dup2(saved_stdout, STDOUT_FILENO);
                close(saved_stdout);
            }
            return -1;
        }
        saved_stdin = dup(STDIN_FILENO);
        dup2(fd, STDIN_FILENO);
        close(fd);
    }

    int result = -1;

    if (strcmp(cmd->argv[0], "cd") == 0)
        result = builtin_cd(cmd);
    else if (strcmp(cmd->argv[0], "pwd") == 0)
        result = builtin_pwd(cmd);
    else if (strcmp(cmd->argv[0], "echo") == 0)
        result = builtin_echo(cmd);
    else if (strcmp(cmd->argv[0], "exit") == 0)
        result = builtin_exit(cmd);
    else if (strcmp(cmd->argv[0], "jobs") == 0)
        result = builtin_jobs(cmd);
    else if (strcmp(cmd->argv[0], "fg") == 0)
        result = builtin_fg(cmd);
    else if (strcmp(cmd->argv[0], "bg") == 0)
        result = builtin_bg(cmd);

    if (saved_stdout != -1) {
        dup2(saved_stdout, STDOUT_FILENO);
        close(saved_stdout);
    }
    if (saved_stdin != -1) {
        dup2(saved_stdin, STDIN_FILENO);
        close(saved_stdin);
    }

    return result;
}
