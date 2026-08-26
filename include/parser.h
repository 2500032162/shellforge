#ifndef PARSER_H
#define PARSER_H

#include "token.h"

typedef struct command {
    char **argv;
    int argc;
    char *input;
    char *output;
    int append;
    int background;
    struct command *next;
} Command;

Command *parse_tokens(const TokenList *tokens);
void free_command(Command *cmd);
void print_command(const Command *cmd);

#endif
