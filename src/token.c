#define _POSIX_C_SOURCE 200809L

#include "../include/token.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

static token_t *create_token(token_type_t type, const char *value) {
    token_t *tok = malloc(sizeof(token_t));
    if (!tok) return NULL;
    tok->type = type;
    tok->value = value ? strdup(value) : NULL;
    tok->next = NULL;
    return tok;
}

static void append_token(TokenList *list, token_t *tok) {
    if (!list->head) {
        list->head = tok;
        list->tail = tok;
    } else {
        list->tail->next = tok;
        list->tail = tok;
    }
    list->count++;
}

TokenList *tokenize(const char *input) {
    TokenList *list = malloc(sizeof(TokenList));
    if (!list) return NULL;
    list->head = NULL;
    list->tail = NULL;
    list->count = 0;

    const char *p = input;
    while (*p) {
        // Skip whitespace
        while (*p && isspace(*p)) p++;
        if (!*p) break;

        // Handle operators
        if (*p == '|') {
            append_token(list, create_token(TOKEN_PIPE, "|"));
            p++;
            continue;
        }
        if (*p == '<') {
            append_token(list, create_token(TOKEN_REDIRECT_IN, "<"));
            p++;
            continue;
        }
        if (*p == '>') {
            if (*(p+1) == '>') {
                append_token(list, create_token(TOKEN_REDIRECT_APPEND, ">>"));
                p += 2;
            } else {
                append_token(list, create_token(TOKEN_REDIRECT_OUT, ">"));
                p++;
            }
            continue;
        }
        if (*p == '&') {
            if (*(p+1) == '&') {
                append_token(list, create_token(TOKEN_AND, "&&"));
                p += 2;
            } else {
                p++;
            }
            continue;
        }
        if (*p == ';') {
            append_token(list, create_token(TOKEN_SEMICOLON, ";"));
            p++;
            continue;
        }

        // Handle words
        if (isalnum(*p) || *p == '_' || *p == '-' || *p == '.' || *p == '/') {
            const char *start = p;
            while (*p && (isalnum(*p) || *p == '_' || *p == '-' || *p == '.' || *p == '/' || *p == '=' || *p == '~')) p++;
            char *word = malloc(p - start + 1);
            strncpy(word, start, p - start);
            word[p - start] = '\0';
            append_token(list, create_token(TOKEN_WORD, word));
            free(word);
            continue;
        }

        // Skip unknown characters
        p++;
    }

    // Add END token
    append_token(list, create_token(TOKEN_END, "END"));

    return list;
}

void print_tokens(const TokenList *tokens) {
    if (!tokens) return;
    
    printf("\n------------- TOKENS -------------\n");
    token_t *curr = tokens->head;
    int i = 0;
    while (curr) {
        const char *type_str;
        switch (curr->type) {
            case TOKEN_WORD: type_str = "WORD"; break;
            case TOKEN_PIPE: type_str = "PIPE"; break;
            case TOKEN_REDIRECT_IN: type_str = "REDIRECT_IN"; break;
            case TOKEN_REDIRECT_OUT: type_str = "REDIRECT_OUT"; break;
            case TOKEN_REDIRECT_APPEND: type_str = "REDIRECT_APPEND"; break;
            case TOKEN_AND: type_str = "AND"; break;
            case TOKEN_OR: type_str = "OR"; break;
            case TOKEN_SEMICOLON: type_str = "SEMICOLON"; break;
            case TOKEN_END: type_str = "END"; break;
            default: type_str = "UNKNOWN";
        }
        printf("%d : %s    %s\n", i, type_str, curr->value);
        curr = curr->next;
        i++;
    }
    printf("------------------------------------\n");
}

void free_token_list(TokenList *tokens) {
    if (!tokens) return;
    token_t *curr = tokens->head;
    while (curr) {
        token_t *next = curr->next;
        free(curr->value);
        free(curr);
        curr = next;
    }
    free(tokens);
}
