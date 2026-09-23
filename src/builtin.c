#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>

#include "builtin.h"

/* =========================================================
   BUILTIN: cd
   ========================================================= */

static int builtin_cd(Command *cmd)
{
    const char *directory;

    /*
     * cd with no argument
     * changes to the user's HOME directory.
     */
    if (cmd->argc == 1)
    {
        directory = getenv("HOME");

        if (directory == NULL)
        {
            fprintf(stderr,
                    "cd: HOME not set\n");

            return -1;
        }
    }
    else if (cmd->argc == 2)
    {
        /*
         * cd has one directory argument.
         */
        directory = cmd->argv[1];
    }
    else
    {
        /*
         * Too many arguments.
         */
        fprintf(stderr,
                "cd: too many arguments\n");

        return -1;
    }

    /*
     * Change the current working directory.
     */
    if (chdir(directory) != 0)
    {
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

    /*
     * pwd does not need arguments.
     */
    if (cmd->argc > 1)
    {
        fprintf(stderr,
                "pwd: too many arguments\n");

        return -1;
    }

    /*
     * Get current working directory.
     */
    if (getcwd(current_directory,
               sizeof(current_directory)) == NULL)
    {
        perror("pwd");

        return -1;
    }

    /*
     * Display current directory.
     */
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
    /*
     * Start from argv[1].
     *
     * argv[0] contains "echo".
     */
    for (int i = 1; i < cmd->argc; i++)
    {
        printf("%s", cmd->argv[i]);

        /*
         * Print a space between arguments.
         */
        if (i < cmd->argc - 1)
        {
            printf(" ");
        }
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
    /*
     * Basic version:
     *
     * exit
     */
    if (cmd->argc > 1)
    {
        fprintf(stderr,
                "exit: too many arguments\n");

        return -1;
    }

    /*
     * Tell the main shell loop to terminate.
     */
    return 1;
}

/* =========================================================
   CHECK WHETHER COMMAND IS A BUILTIN
   ========================================================= */

int is_builtin(const Command *cmd)
{
    if (cmd == NULL || cmd->argc == 0)
    {
        return 0;
    }

    if (strcmp(cmd->argv[0], "cd") == 0)
        return 1;

    if (strcmp(cmd->argv[0], "pwd") == 0)
        return 1;

    if (strcmp(cmd->argv[0], "echo") == 0)
        return 1;

    if (strcmp(cmd->argv[0], "exit") == 0)
        return 1;

    return 0;
}

/* =========================================================
   EXECUTE BUILTIN (with redirection support)
   ========================================================= */

int execute_builtin(Command *cmd)
{
    if (cmd == NULL || cmd->argc == 0)
        return -1;

    /* Save original stdout/stdin so we can restore after redirection */
    int saved_stdout = -1;
    int saved_stdin  = -1;

    /* Apply output redirection for builtins (echo, pwd) */
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

    /* Apply input redirection */
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

    /* Restore original stdout/stdin */
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
