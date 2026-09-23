#ifndef EXECUTOR_H
#define EXECUTOR_H

#include "parser.h"

#define MAX_ARGS 64

int execute_command(Command *cmd);
int execute_pipeline(Command *head);

#endif
