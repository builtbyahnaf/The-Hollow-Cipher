
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "interrogation_parser.h"

static int statement_count = 0;
static SuspectStatements *statements = NULL;

static char *copy_text(const char *text)
{
    char *copy = malloc(strlen(text) + 1);

    if (copy != NULL)
    {
        strcpy(copy, text);
    }

    return copy;
}

static void trim_spaces(char *text)
{
    char *start = text;

    while (isspace((unsigned char)*start))
    {
        start++;
    }

    if (start != text)
    {
        memmove(text, start, strlen(start) + 1);
    }

    size_t len = strlen(text);

    while (len > 0 && isspace((unsigned char)text[len - 1]))
    {
        text[--len] = '\0';
    }
}

static int is_suspect_header(const char *line)
{
    while (isspace((unsigned char)*line))
    {
        line++;
    }

    return line[0] == '[' &&
           strchr(line, ']') != NULL &&
           strcmp(line, "[Interrogation]") != 0;
}

SuspectStatements *parse_interrogation(FILE *file_path)
{
    char line[2048];

    if (file_path == NULL)
    {
        return NULL;
    }

    free_statements();
    rewind(file_path);

    while (fgets(line, sizeof(line), file_path) != NULL)
    {
        line[strcspn(line, "\r\n")] = '\0';

        if (is_suspect_header(line))
        {
            char *start = strchr(line, '[') + 1;
            char *end = strchr(start, ']');

            *end = '\0';
            trim_spaces(start);

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

            statements[statement_count].name = copy_text(start);
            statements[statement_count].statement = copy_text("");

            if (statements[statement_count].name == NULL ||
                statements[statement_count].statement == NULL)
            {
                free(statements[statement_count].name);
                free(statements[statement_count].statement);
                free_statements();
                return NULL;
            }

            statement_count++;
        }
        else if (statement_count > 0 && line[0] != '\0')
        {
            SuspectStatements *current =
                &statements[statement_count - 1];

            size_t old_len = strlen(current->statement);
            size_t line_len = strlen(line);

            char *temp = realloc(
                current->statement,
                old_len + line_len + 2
            );

            if (temp == NULL)
            {
                free_statements();
                return NULL;
            }

            current->statement = temp;

            if (old_len > 0)
            {
                current->statement[old_len++] = '\n';
            }

            memcpy(
                current->statement + old_len,
                line,
                line_len + 1
            );
        }
    }

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
