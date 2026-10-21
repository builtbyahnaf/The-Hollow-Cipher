
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "past_history_parser.h"

static int total_history = 0;
static PastHistory *history_list = NULL;

static void trim_spaces(char *text)
{
    char *start = text;

    while (isspace((unsigned char)*start))
        start++;

    if (start != text)
        memmove(text, start, strlen(start) + 1);

    size_t len = strlen(text);

    while (len > 0 && isspace((unsigned char)text[len - 1]))
        text[--len] = '\0';
}

PastHistory *parse_past_history(FILE *file)
{
    char line[2048];

    if (file == NULL)
        return NULL;

    free_past_history();
    rewind(file);

    while (fgets(line, sizeof(line), file) != NULL)
    {
        line[strcspn(line, "\r\n")] = '\0';
        trim_spaces(line);

        if (line[0] == '[')
        {
            char *end = strchr(line, ']');

            if (end == NULL)
                continue;

            *end = '\0';

            char *name = line + 1;
            trim_spaces(name);

            if (strcmp(name, "Past History") == 0 ||
                strcmp(name, "Investigative Direction") == 0)
            {
                continue;
            }

            PastHistory *temp = realloc(
                history_list,
                (total_history + 1) * sizeof(PastHistory)
            );

            if (temp == NULL)
            {
                free_past_history();
                return NULL;
            }

            history_list = temp;

            history_list[total_history].name = malloc(strlen(name) + 1);
            history_list[total_history].history = calloc(1, sizeof(char));

            if (history_list[total_history].name == NULL ||
                history_list[total_history].history == NULL)
            {
                free(history_list[total_history].name);
                free(history_list[total_history].history);
                free_past_history();
                return NULL;
            }

            strcpy(history_list[total_history].name, name);
            total_history++;
        }
        else if (total_history > 0 && line[0] != '\0')
        {
            PastHistory *current = &history_list[total_history - 1];

            size_t old_len = strlen(current->history);
            size_t line_len = strlen(line);

            char *temp = realloc(
                current->history,
                old_len + line_len + 2
            );

            if (temp == NULL)
            {
                free_past_history();
                return NULL;
            }

            current->history = temp;

            if (old_len > 0)
                current->history[old_len++] = '\n';

            memcpy(current->history + old_len, line, line_len + 1);
        }
    }

    return history_list;
}

int past_history_count(void)
{
    return total_history;
}

void free_past_history(void)
{
    for (int i = 0; i < total_history; i++)
    {
        free(history_list[i].name);
        free(history_list[i].history);
    }

    free(history_list);
    history_list = NULL;
    total_history = 0;
}
