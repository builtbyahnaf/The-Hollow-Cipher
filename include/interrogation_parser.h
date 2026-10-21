#ifndef INTERROGATION_PARSER_H
#define INTERROGATION_PARSER_H

#include <stdio.h>

typedef struct
{
    char *name;
    char *statement;
} SuspectStatements;

SuspectStatements *parse_interrogation(FILE *file_path);

int statements_count(void);

void free_statements(void);

#endif