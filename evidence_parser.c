
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "evidence_parser.h"

static int total_evidence = 0;
static Evidence *evidence_list = NULL;

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

static int is_evidence_title(const char *line)
{
    while (isspace((unsigned char)*line))
    {
        line++;
    }

    if (!isdigit((unsigned char)*line))
    {
        return 0;
    }

    while (isdigit((unsigned char)*line))
    {
        line++;
    }

    return *line == '.';
}

Evidence *parse_evidence(FILE *file)
{
    char line[2048];

    if (file == NULL)
    {
        return NULL;
    }

    free_evidence();
    rewind(file);

    while (fgets(line, sizeof(line), file) != NULL)
    {
        line[strcspn(line, "\r\n")] = '\0';
        trim_spaces(line);

        if (is_evidence_title(line))
        {
            char *title = strchr(line, '.');

            if (title == NULL)
            {
                continue;
            }

            title++;
            trim_spaces(title);

            Evidence *temp = realloc(
                evidence_list,
                (total_evidence + 1) * sizeof(Evidence)
            );

            if (temp == NULL)
            {
                free_evidence();
                return NULL;
            }

            evidence_list = temp;

            evidence_list[total_evidence].name = copy_text(title);
            evidence_list[total_evidence].description = copy_text("");

            if (evidence_list[total_evidence].name == NULL ||
                evidence_list[total_evidence].description == NULL)
            {
                free(evidence_list[total_evidence].name);
                free(evidence_list[total_evidence].description);

                evidence_list[total_evidence].name = NULL;
                evidence_list[total_evidence].description = NULL;

                free_evidence();
                return NULL;
            }

            total_evidence++;
        }
        else if (total_evidence > 0 && line[0] != '\0')
        {
            Evidence *current = &evidence_list[total_evidence - 1];

            size_t old_len = strlen(current->description);
            size_t line_len = strlen(line);

            char *temp = realloc(
                current->description,
                old_len + line_len + 2
            );

            if (temp == NULL)
            {
                free_evidence();
                return NULL;
            }

            current->description = temp;

            if (old_len > 0)
            {
                current->description[old_len++] = '\n';
            }

            memcpy(
                current->description + old_len,
                line,
                line_len + 1
            );
        }
    }

    return evidence_list;
}

int evidence_count(void)
{
    return total_evidence;
}

void free_evidence(void)
{
    for (int i = 0; i < total_evidence; i++)
    {
        free(evidence_list[i].name);
        free(evidence_list[i].description);
    }

    free(evidence_list);
    evidence_list = NULL;
    total_evidence = 0;
}
