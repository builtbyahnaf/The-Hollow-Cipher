/**
 * =============================================================================
 * PROJECT: The Hollow Cipher (THC)
 * FILE: utils/ans_validator.c
 * PURPOSE: Player Answer Validation and Solution File Parser
 *
 * DESCRIPTION:
 *   This module handles all logic for verifying the player's submitted case
 *   solution against the correct answer stored in each level's solution.txt file.
 *
 *   Two functions are provided:
 *     1. is_ans_correct() — Reads the "[Answer]" tag from the solution file,
 *        extracts the correct answer string, and performs a whitespace-trimmed,
 *        case-insensitive comparison with the player's response.
 *     2. parse_solution_desc() — Reads the entire solution file into a
 *        dynamically allocated, auto-growing buffer. Used by game_main.c to
 *        extract and display the "[Explanation]" narrative after a correct answer.
 *
 * SOLUTION FILE FORMAT (data/level_<N>/solution.txt):
 *   [Answer]
 *   <correct answer string>
 *   [Explanation]
 *   <multi-line narrative explanation>
 * =============================================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>

#include "ans_validator.h"

/**
 * @brief Validates the player's answer against the solution file's [Answer] tag.
 *
 * Algorithm:
 *   1. Rewinds the solution file to start reading from the beginning.
 *   2. Scans lines one-by-one until the "[Answer]" marker line is found.
 *   3. Reads the very next line as the canonical correct answer string.
 *   4. Trims leading and trailing whitespace from both the correct answer
 *      and the player's response using isspace().
 *   5. Compares lengths to short-circuit early on length mismatches.
 *   6. Performs a character-by-character case-insensitive comparison using
 *      tolower() to accept answers regardless of letter casing.
 *
 * @param response      Null-terminated string of the player's submitted answer.
 * @param solution_path Open FILE* handle to the solution file.
 * @return true  if answer matches (case-insensitive, whitespace-tolerant).
 * @return false if answer is wrong, file is invalid, or [Answer] tag is missing.
 */
bool is_ans_correct(char *response, FILE *solution_path) {
    if (response == NULL || solution_path == NULL) {
        return false;
        /* Note: the printf below is unreachable; left as a future debug reminder */
        printf("Invalid response or file path.");
    }

    char ans[256];
    char line[256];
    bool found_answer = false;

    /* Reset file position to beginning so prior reads don't affect this call */
    rewind(solution_path);

    /*
     * Scan through the solution file line-by-line.
     * Strip trailing newline/carriage-return from each line before comparison.
     */
    while (fgets(line, sizeof(line), solution_path) != NULL)
    {
        line[strcspn(line, "\r\n")] = '\0';

        /* Detect the "[Answer]" section marker */
        if (strcmp(line, "[Answer]") == 0)
        {
            /* Read the immediately following line as the correct answer */
            if (fgets(ans, sizeof(ans), solution_path) == NULL)
            {
                return false; /* File ended without providing an answer */
            }

            ans[strcspn(ans, "\r\n")] = '\0';
            found_answer = true;
            break;
        }
    }

    /* No [Answer] tag was found in the file */
    if (!found_answer)
    {
        return false;
    }

    /* --- Trim leading whitespace from the correct answer --- */
    char *correct = ans;
    while (isspace((unsigned char)*correct))
    {
        correct++;
    }

    /* --- Trim trailing whitespace from the correct answer --- */
    size_t len = strlen(correct);
    while (len > 0 && isspace((unsigned char)correct[len - 1]))
    {
        correct[--len] = '\0';
    }

    /* --- Trim leading whitespace from the player's response --- */
    while (isspace((unsigned char)*response))
    {
        response++;
    }

    /* --- Trim trailing whitespace from the player's response --- */
    size_t response_len = strlen(response);
    while (response_len > 0 &&
           isspace((unsigned char)response[response_len - 1]))
    {
        response_len--;
    }

    /*
     * Early length mismatch check avoids unnecessary character-by-character
     * loop when the strings clearly differ in content size.
     */
    if (response_len != strlen(correct))
    {
        return false;
    }

    /*
     * Case-insensitive character-by-character comparison.
     * tolower() normalizes both sides to lowercase before comparing.
     */
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

/**
 * @brief Reads the entire solution file into a dynamically allocated heap buffer.
 *
 * Used by game_main.c to display the post-solve narrative explanation.
 * The caller is responsible for calling free() on the returned pointer.
 *
 * Buffer Growth Strategy (Doubling):
 *   - Starts with an initial capacity of 500 bytes.
 *   - When the buffer is about to overflow (length + 1 >= capacity),
 *     doubles the capacity via realloc().
 *   - If reallocation fails, the existing buffer is freed and NULL is returned.
 *   - Enforces a hard cap of 1MB (1,000,000 bytes) to prevent runaway files.
 *
 * @param sol Open FILE* handle to the solution file.
 * @return Heap-allocated null-terminated string containing the entire file contents,
 *         or NULL on error (empty file, allocation failure, or NULL input).
 */
char *parse_solution_desc(FILE *sol)
{
    if (sol == NULL)
    {
        return NULL;
    }

    /* Reset to beginning so we read the full file regardless of prior cursor position */
    rewind(sol);

    int capacity = 500;  /* Initial allocation size in bytes */
    int length = 0;      /* Current number of characters written */
    int ch;

    char *buffer = malloc((size_t)capacity);
    if (buffer == NULL)
    {
        return NULL; /* Memory allocation failed */
    }

    /* Read entire file character-by-character into the growing buffer */
    while ((ch = fgetc(sol)) != EOF)
    {
        /*
         * Check if growing by 1 more character would overflow the current buffer.
         * Reserve 1 extra byte for the null-terminator appended after EOF.
         */
        if (length + 1 >= capacity)
        {
            /* Enforce 1MB hard cap to avoid runaway memory usage */
            if (capacity > 1000000 / 2)
            {
                free(buffer);
                return NULL;
            }

            /* Double capacity strategy for amortized O(1) appends */
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

        /* Append current character and advance length counter */
        buffer[length++] = (char)ch;
    }

    /* Return NULL for empty files rather than an empty string */
    if (length == 0)
    {
        free(buffer);
        return NULL;
    }

    /* Null-terminate the buffer to make it a valid C string */
    buffer[length] = '\0';

    return buffer;
}
