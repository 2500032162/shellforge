#ifndef BUILTIN_H
#define BUILTIN_H

#include "parser.h"

/* Function prototypes */
int is_builtin(const Command *cmd);
int execute_builtin(Command *cmd);

#endif
