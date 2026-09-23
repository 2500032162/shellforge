#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <readline/history.h>
#include <readline/readline.h>
#include "history.h"
#include "token.h"
#include "lexer.h"
#include "parser.h"
#include "expand.h"
#include "builtin.h"
#include "executor.h"

int main(void)
{
    printf("=====================================\n");
    printf("      Shellforge \n");
    printf(" A Unix Style Shell written in C\n");
    printf("=====================================\n");

    char *line;

    while (1)
    {
        line = readline("shellforge$ ");
        if (line == NULL) {
            printf("\nGoodbye!\n");
            break;
        }
        if (strlen(line) == 0) {
            free(line);
            continue;
        }

        /* history command */
        if (strcmp(line, "history") == 0) {
            printf("Command history feature\n");
            free(line);
            continue;
        }

        add_history(line);

        /* exit command */
        if (strcmp(line, "exit") == 0) {
            TokenList *tokens = tokenize(line);
            if (tokens) {
                print_tokens(tokens);
                Command *cmd = parse_tokens(tokens);
                if (cmd) {
                    print_command(cmd);
                    execute_builtin(cmd);
                    free_pipeline(cmd);
                    free_token_list(tokens);
                    free(line);
                    printf("Exiting...\n");
                    return 0;
                }
                free_token_list(tokens);
            }
            free(line);
            printf("Exiting...\n");
            break;
        }

        /* Tokenize */
        TokenList *tokens = tokenize(line);
        if (tokens) {
            print_tokens(tokens);

            /* Parse into pipeline list */
            Command *head = parse_tokens(tokens);
            if (head) {
                print_command(head);

                /* Check if the FIRST command is builtin exit */
                int is_exit = (head->argc > 0 &&
                               strcmp(head->argv[0], "exit") == 0);

                /* Execute pipeline */
                execute_pipeline(head);

                free_pipeline(head);

                if (is_exit) {
                    free_token_list(tokens);
                    free(line);
                    printf("Exiting...\n");
                    return 0;
                }
            }
            free_token_list(tokens);
        }

        free(line);
    }
    return 0;
}
