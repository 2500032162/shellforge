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

int main(void)
{
    // Display a welcome banner when the shell starts
    printf("=====================================\n");
    printf("      Shellforge \n");
    printf(" A Unix Style Shell written in C\n");
    printf("=====================================\n");

    char *line;

    while (1)
    {
        line = readline("shellforge$ ");
        if (line == NULL)
        {
            printf("\nGoodbye!\n");
            break;
        }
        if (strlen(line) == 0)
        {
            free(line);
            continue;
        }

        // Check for history command
        if (strcmp(line, "history") == 0)
        {
            // Simple history display
            printf("Command history feature\n");
            free(line);
            continue;
        }

        add_history(line);

        // Check for exit command
        if (strcmp(line, "exit") == 0)
        {
            // Show tokens for exit
            TokenList *tokens = tokenize(line);
            if (tokens) {
                print_tokens(tokens);
                
                // Parse tokens into command structure
                Command *cmd = parse_tokens(tokens);
                if (cmd) {
                    print_command(cmd);
                    free_command(cmd);
                }
                free_token_list(tokens);
            }
            free(line);
            printf("Exiting...\n");
            break;
        }

        // Tokenize the input
        TokenList *tokens = tokenize(line);
        if (tokens) {
            print_tokens(tokens);
            
            // Parse tokens into command structure
            Command *cmd = parse_tokens(tokens);
            if (cmd) {
                // Print pipeline/command structure
                print_command(cmd);
                
                // Check if it's a builtin command
                if (is_builtin(cmd)) {
                    int result = execute_builtin(cmd);
                    if (result == 1) {
                        free_command(cmd);
                        free_token_list(tokens);
                        free(line);
                        return 0;
                    }
                } else {
                    printf("External command: %s\n", cmd->argv[0]);
                    printf("(External execution will be implemented in later milestones)\n");
                }
                free_command(cmd);
            }
            free_token_list(tokens);
        }
        
        free(line);
    }
    return 0;
}
