#ifndef EXECUTOR_H
#define EXECUTOR_H

#include "parser.h"

#define MAX_ARGS 64

/* Execute a single command (builtin or external) */
int execute_command(Command *cmd);

/* Execute a full pipeline (single or multi-command) */
int execute_pipeline(Command *head);

#endif
