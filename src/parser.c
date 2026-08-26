#define _POSIX_C_SOURCE 200809L

#include "../include/parser.h"
#include "../include/token.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

Command *parse_tokens(const TokenList *tokens) {
    if (!tokens || !tokens->head) return NULL;
    
    Command *cmd = malloc(sizeof(Command));
    if (!cmd) return NULL;
    
    cmd->argv = NULL;
    cmd->argc = 0;
    cmd->input = NULL;
    cmd->output = NULL;
    cmd->append = 0;
    cmd->background = 0;
    cmd->next = NULL;
    
    token_t *curr = tokens->head;
    int word_count = 0;
    
    // Count words first
    while (curr && curr->type != TOKEN_END) {
        if (curr->type == TOKEN_WORD) {
            word_count++;
        }
        curr = curr->next;
    }
    
    // Allocate argv array
    cmd->argv = malloc((word_count + 1) * sizeof(char *));
    if (!cmd->argv) {
        free(cmd);
        return NULL;
    }
    
    curr = tokens->head;
    int idx = 0;
    
    while (curr && curr->type != TOKEN_END) {
        if (curr->type == TOKEN_WORD) {
            cmd->argv[idx] = strdup(curr->value);
            idx++;
        } else if (curr->type == TOKEN_REDIRECT_IN) {
            // Next token should be a word (filename)
            curr = curr->next;
            if (curr && curr->type == TOKEN_WORD) {
                cmd->input = strdup(curr->value);
            }
        } else if (curr->type == TOKEN_REDIRECT_OUT) {
            // Next token should be a word (filename)
            curr = curr->next;
            if (curr && curr->type == TOKEN_WORD) {
                cmd->output = strdup(curr->value);
                cmd->append = 0;
            }
        } else if (curr->type == TOKEN_REDIRECT_APPEND) {
            // Next token should be a word (filename)
            curr = curr->next;
            if (curr && curr->type == TOKEN_WORD) {
                cmd->output = strdup(curr->value);
                cmd->append = 1;
            }
        } else if (curr->type == TOKEN_PIPE) {
            // For now, just skip pipes
        } else if (curr->type == TOKEN_AND) {
            // Skip &&
        } else if (curr->type == TOKEN_SEMICOLON) {
            // Skip ;
        }
        curr = curr->next;
    }
    
    cmd->argc = idx;
    cmd->argv[idx] = NULL;
    
    return cmd;
}

void free_command(Command *cmd) {
    if (!cmd) return;
    
    if (cmd->argv) {
        for (int i = 0; i < cmd->argc; i++) {
            free(cmd->argv[i]);
        }
        free(cmd->argv);
    }
    free(cmd->input);
    free(cmd->output);
    free(cmd);
}

void print_command(const Command *cmd) {
    if (!cmd) {
        printf("Command is NULL\n");
        return;
    }
    
    printf("\n========= PIPELINE ========\n\n");
    printf("Command 1\n\n");
    printf("Arguments\n");
    
    for (int i = 0; i < cmd->argc; i++) {
        printf("argv[%d] = %s\n", i, cmd->argv[i]);
    }
    
    printf("Input    : %s\n", cmd->input ? cmd->input : "None");
    printf("Output   : %s\n", cmd->output ? cmd->output : "None");
    printf("Append   : %s\n", cmd->append ? "Yes" : "No");
    printf("Background   : %s\n", cmd->background ? "Yes" : "No");
    printf("\n===================\n");
}
