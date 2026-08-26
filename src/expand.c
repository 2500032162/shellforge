#define _POSIX_C_SOURCE 200809L

#include "../include/expand.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

char *expand_variables(const char *input) {
    if (!input) return NULL;
    
    // For now, just return a copy of the input
    // Variable expansion will be implemented in future milestones
    return strdup(input);
}
