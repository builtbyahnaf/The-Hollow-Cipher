
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>

#include "ans_validator.h"


bool is_ans_correct(char *response, FILE *solution_path)
{
    bool ans_valid = false;

    if (response == NULL || solution_path == NULL)
    {
        return false;
    }

    char ans[35];
    char user_ans[35];

    rewind(solution_path);

    if (fgets(ans, sizeof(ans), solution_path) == NULL)
    {
        return false;
    }

    ans[strcspn(ans, "\r\n")] = '\0';

    size_t i = 0;

    while (response[i] != '\0' && i < sizeof(user_ans) - 1)
    {
        user_ans[i] = (char)tolower(
            (unsigned char)response[i]
        );
        i++;
    }

    user_ans[i] = '\0';

    for (i = 0; ans[i] != '\0'; i++)
    {
        ans[i] = (char)tolower((unsigned char)ans[i]);
    }
if (strcmp(user_ans, ans) == 0)
{
    ans_valid = true;
}

    else
    {
        ans_valid = false;
    }

    return ans_valid;
}

char *parse_solution_desc(FILE *sol)
{
    if (sol == NULL)
    {
        return NULL;
    }

    int capacity = 500;
    int length = 0;
    int ch;

    char *buffer = malloc(capacity * sizeof(char));

    if (buffer == NULL)
    {
        return NULL;
    }

    while ((ch = fgetc(sol)) != EOF)
    {
        if (length + 1 >= capacity)
        {
            int new_capacity = capacity * 2;

            char *temp = realloc(
                buffer,
                new_capacity * sizeof(char)
            );

            if (temp == NULL)
            {
                free(buffer);
                return NULL;
            }

            buffer = temp;
            capacity = new_capacity;
        }

        buffer[length] = (char)ch;
        length++;
    }

    if (length == 0)
    {
        free(buffer);
        return NULL;
    }

    buffer[length] = '\0';

    return buffer;
}

