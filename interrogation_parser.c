#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "interrogation_parser.h"
static int statement_count = 0;

static SuspectStatements *statements = NULL;
SuspectStatements *parse_interrogation(FILE *file_path)
{
    char line[2048];
    char *name;
    char *statement;

    if (file_path == NULL)
    {
        return NULL;
    }

    free_statements();

    while (fgets(line, sizeof(line), file_path) != NULL)
    {
        line[strcspn(line, "\r\n")] = '\0';

        name = strtok(line, "|");
        statement = strtok(NULL, "|");

        if (name == NULL || statement == NULL)
        {
            continue;
        }
    }
    SuspectStatements *temp = realloc(
    statements,
    (statement_count + 1) * sizeof(SuspectStatements)
);

if (temp == NULL)
{
    free_statements();
    return NULL;
}

statements = temp;

statements[statement_count].name = malloc(strlen(name) + 1);
statements[statement_count].statement = malloc(strlen(statement) + 1);

if (statements[statement_count].name == NULL ||
    statements[statement_count].statement == NULL)
{
    free_statements();
    return NULL;
}

strcpy(statements[statement_count].name, name);
strcpy(statements[statement_count].statement, statement);

statement_count++;
return statements;

}
int statements_count(void)
{
    return statement_count;
}
void free_statements(void)
{
    for (int i = 0; i < statement_count; i++)
    {
        free(statements[i].name);
        free(statements[i].statement);
    }

    free(statements);
    statements = NULL;
    statement_count = 0;
}

