#include "../include/lexer.h"
#include "../include/token.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

// Lexer uses the tokenizer to break input into tokens
// This is a wrapper around tokenize for future expansion

TokenList *lexer_tokenize(const char *input) {
    // For now, just call the tokenizer
    // Future: add more sophisticated lexing logic
    return tokenize(input);
}
