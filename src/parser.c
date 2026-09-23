#define _POSIX_C_SOURCE 200809L

#include "../include/parser.h"
#include "../include/token.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Helper: create a new empty command node */
static Command *create_command(void)
{
    Command *cmd = malloc(sizeof(Command));
    if (!cmd) return NULL;
    cmd->argv = NULL;
    cmd->argc = 0;
    cmd->input = NULL;
    cmd->output = NULL;
    cmd->append = 0;
    cmd->background = 0;
    cmd->next = NULL;
    return cmd;
}

/* Helper: add a word to a command's argv */
static void add_word(Command *cmd, const char *word)
{
    cmd->argv = realloc(cmd->argv, (cmd->argc + 2) * sizeof(char *));
    cmd->argv[cmd->argc] = strdup(word);
    cmd->argc++;
    cmd->argv[cmd->argc] = NULL;
}

Command *parse_tokens(const TokenList *tokens)
{
    if (!tokens || !tokens->head) return NULL;

    Command *head = NULL;
    Command *current = NULL;
    token_t *curr = tokens->head;

    while (curr && curr->type != TOKEN_END) {
        if (!current) {
            current = create_command();
            if (!head) head = current;
        }

        if (curr->type == TOKEN_WORD) {
            add_word(current, curr->value);
        }
        else if (curr->type == TOKEN_PIPE) {
            current->next = create_command();
            current = current->next;
        }
        else if (curr->type == TOKEN_REDIRECT_IN) {
            curr = curr->next;
            if (curr && curr->type == TOKEN_WORD)
                current->input = strdup(curr->value);
        }
        else if (curr->type == TOKEN_REDIRECT_OUT) {
            curr = curr->next;
            if (curr && curr->type == TOKEN_WORD) {
                current->output = strdup(curr->value);
                current->append = 0;
            }
        }
        else if (curr->type == TOKEN_REDIRECT_APPEND) {
            curr = curr->next;
            if (curr && curr->type == TOKEN_WORD) {
                current->output = strdup(curr->value);
                current->append = 1;
            }
        }
        else if (curr->type == TOKEN_AND) {
            current->background = 1;
        }
        else if (curr->type == TOKEN_BACKGROUND) {
            current->background = 1;
        }
        else if (curr->type == TOKEN_SEMICOLON) {
            current->next = create_command();
            current = current->next;
        }

        curr = curr->next;
    }

    return head;
}

int pipeline_count(Command *head)
{
    int n = 0;
    while (head) { n++; head = head->next; }
    return n;
}

void free_command(Command *cmd)
{
    if (!cmd) return;
    if (cmd->argv) {
        for (int i = 0; i < cmd->argc; i++)
            free(cmd->argv[i]);
        free(cmd->argv);
    }
    free(cmd->input);
    free(cmd->output);
    free(cmd);
}

void free_pipeline(Command *head)
{
    while (head) {
        Command *next = head->next;
        free_command(head);
        head = next;
    }
}

void print_command(const Command *cmd)
{
    if (!cmd) {
        printf("Command is NULL\n");
        return;
    }

    printf("\n========= PIPELINE ========\n\n");
    int i = 1;
    while (cmd) {
        printf("Command %d\n\n", i);
        printf("Arguments\n");
        for (int j = 0; j < cmd->argc; j++)
            printf("argv[%d] = %s\n", j, cmd->argv[j]);
        printf("Input    : %s\n", cmd->input ? cmd->input : "None");
        printf("Output   : %s\n", cmd->output ? cmd->output : "None");
        printf("Append   : %s\n", cmd->append ? "Yes" : "No");
        printf("Background   : %s\n", cmd->background ? "Yes" : "No");
        printf("\n");
        cmd = cmd->next;
        i++;
    }
    printf("===================\n");
}
