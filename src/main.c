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
#include "jobs.h"

int main(void)
{
    printf("=====================================\n");
    printf("      Shellforge \n");
    printf(" A Unix Style Shell written in C\n");
    printf("=====================================\n");

    jobs_init();

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

        if (strcmp(line, "history") == 0) {
            printf("Command history feature\n");
            free(line);
            continue;
        }

        add_history(line);

        if (strcmp(line, "exit") == 0) {
            printf("Exiting...\n");
            free(line);
            break;
        }

        TokenList *tokens = tokenize(line);
        if (tokens) {
            Command *head = parse_tokens(tokens);
            if (head) {
                execute_pipeline(head);
                free_pipeline(head);
            }
            free_token_list(tokens);
        }

        free(line);
    }
    return 0;
}
