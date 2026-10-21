
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>

#include "ans_validator.h"

bool is_ans_correct(char *response, FILE *solution_path)
{
    if (response == NULL || solution_path == NULL)
    {
        return false;
    }

    char ans[256];
    char line[256];
    bool found_answer = false;

    rewind(solution_path);

    while (fgets(line, sizeof(line), solution_path) != NULL)
    {
        line[strcspn(line, "\r\n")] = '\0';

        if (strcmp(line, "[Answer]") == 0)
        {
            if (fgets(ans, sizeof(ans), solution_path) == NULL)
            {
                return false;
            }

            ans[strcspn(ans, "\r\n")] = '\0';
            found_answer = true;
            break;
        }
    }

    if (!found_answer)
    {
        return false;
    }

    /* Remove leading spaces from the correct answer. */
    char *correct = ans;

    while (isspace((unsigned char)*correct))
    {
        correct++;
    }

    /* Remove trailing spaces from the correct answer. */
    size_t len = strlen(correct);

    while (len > 0 && isspace((unsigned char)correct[len - 1]))
    {
        correct[--len] = '\0';
    }

    /* Remove leading spaces from the user's response. */
    while (isspace((unsigned char)*response))
    {
        response++;
    }

    /* Remove trailing spaces from the user's response. */
    size_t response_len = strlen(response);

    while (response_len > 0 &&
           isspace((unsigned char)response[response_len - 1]))
    {
        response_len--;
    }

    if (response_len != strlen(correct))
    {
        return false;
    }

    for (size_t i = 0; i < response_len; i++)
    {
        if (tolower((unsigned char)response[i]) !=
            tolower((unsigned char)correct[i]))
        {
            return false;
        }
    }

    return true;
}

char *parse_solution_desc(FILE *sol)
{
    if (sol == NULL)
    {
        return NULL;
    }

    rewind(sol);

    int capacity = 500;
    int length = 0;
    int ch;

    char *buffer = malloc((size_t)capacity);

    if (buffer == NULL)
    {
        return NULL;
    }

    while ((ch = fgetc(sol)) != EOF)
    {
        if (length + 1 >= capacity)
        {
            if (capacity > 1000000 / 2)
            {
                free(buffer);
                return NULL;
            }

            int new_capacity = capacity * 2;

            char *temp = realloc(buffer, (size_t)new_capacity);

            if (temp == NULL)
            {
                free(buffer);
                return NULL;
            }

            buffer = temp;
            capacity = new_capacity;
        }

        buffer[length++] = (char)ch;
    }

    if (length == 0)
    {
        free(buffer);
        return NULL;
    }

    buffer[length] = '\0';

    return buffer;
}
