#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../include/lexer.h"
#include "../include/token.h"
#include "../include/parser.h"
#include "../include/expand.h"
#include <readline/readline.h>
#include <readline/history.h>

int main(void)
{
    // Display a welcome banner when the shell starts
    printf("=====================================\n");
    printf("Shellforge \n");
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
        add_history(line);
        
        if (strcmp(line, "exit") == 0)
        {
            free(line);
            printf("Exiting...\n");
            break;
        }
        
        // Expand variables
        char *expanded = expand_variables(line);
        if (!expanded) {
            free(line);
            continue;
        }
        
        // Tokenize the expanded input
        TokenList *tokens = lexer_tokenize(expanded);
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
        
        free(expanded);
        free(line);
    }
    return 0;
}
