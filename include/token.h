#ifndef TOKEN_H
#define TOKEN_H

typedef enum {
    TOKEN_WORD,
    TOKEN_PIPE,
    TOKEN_REDIRECT_IN,
    TOKEN_REDIRECT_OUT,
    TOKEN_REDIRECT_APPEND,
    TOKEN_AND,
    TOKEN_OR,
    TOKEN_SEMICOLON,
    TOKEN_END
} token_type_t;

typedef struct token {
    token_type_t type;
    char *value;
    struct token *next;
} token_t;

typedef struct {
    token_t *head;
    token_t *tail;
    int count;
} TokenList;

TokenList *tokenize(const char *input);
void print_tokens(const TokenList *tokens);
void free_token_list(TokenList *tokens);

#endif
