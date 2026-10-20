#ifndef ANS_VALIDATOR_H
#define ANS_VALIDATOR_H

#include <stdio.h>
#include <stdbool.h>

bool is_ans_correct(char *response, FILE *solution_path);

char *parse_solution_desc(FILE *sol);

#endif

