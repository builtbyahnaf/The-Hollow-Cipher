
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "interrogation_parser.h"

// Number of suspects currently stored in the array.
static int statement_count = 0;

// Dynamically allocated array containing all suspects.
static SuspectStatements *sus_sts = NULL;


// Creates a dynamically allocated copy of a string.
static char *copy_string(const char *source) {
    char *copy = malloc(strlen(source) + 1);

    if (copy == NULL) {
        return NULL;
    }

    strcpy(copy, source);
    return copy;
}


// Removes whitespace from the beginning and end of a string.
// Returns a pointer to the cleaned string.
static char *trim_whitespace(char *text) {
    // Skip whitespace at the beginning.
    while (isspace((unsigned char)*text)) {
        text++;
    }

    // An empty string needs no further processing.
    if (*text == '\0') {
        return text;
    }

    // Find the last non-whitespace character.
    char *end = text + strlen(text) - 1;

    while (end > text && isspace((unsigned char)*end)) {
        end--;
    }

    // Terminate the string immediately after that character.
    *(end + 1) = '\0';

    return text;
}


// Removes a line's newline characters without removing other text.
static void remove_newline(char *text) {
    text[strcspn(text, "\r\n")] = '\0';
}


// Checks whether a line is a bracketed header.
// Examples: [Interrogation], [ Jawer Chakrovarty ]
static int is_header(const char *text) {
    while (isspace((unsigned char)*text)) {
        text++;
    }

    size_t length = strlen(text);

    // A header must begin with '[' and end with ']'.
    return length >= 2 &&
           text[0] == '[' &&
           text[length - 1] == ']';
}


// Frees all currently stored suspects and their strings.
static void clear_statements(void) {
    for (int i = 0; i < statement_count; i++) {
        free(sus_sts[i].name);
        free(sus_sts[i].statement);
    }

    free(sus_sts);

    sus_sts = NULL;
    statement_count = 0;
}


// Appends a line to a suspect's existing statement.
// The statement grows dynamically as more lines are read.
static int append_statement(char **statement, const char *line)
{
    size_t old_length = strlen(*statement);
    size_t line_length = strlen(line);

    // Include space for the new text and the terminating '\0'.
    char *temp = realloc(
        *statement,
        old_length + line_length + 1
    );

    if (temp == NULL) {
        return 0;
    }

    *statement = temp;

    // Copy the new text after the existing statement.
    memcpy(
        *statement + old_length,
        line,
        line_length + 1
    );

    return 1;
}


// Reads an interrogation file and builds the suspect array.
SuspectStatements *parse_interrogation(FILE *file_path)
{
    char line[2048];

    // Remove results from any previous parsing operation.
    clear_statements();

    if (file_path == NULL) {
        return NULL;
    }

    // Read the file one line at a time.
    while (fgets(line, sizeof(line), file_path) != NULL) {

        remove_newline(line);

        // Check whether this line is a section or suspect header.
        if (is_header(line)) {

            // Remove the surrounding brackets.
            line[strlen(line) - 1] = '\0';
            char *name = trim_whitespace(line + 1);

            // [Interrogation] identifies the section, not a suspect.
            if (strcmp(name, "Interrogation") == 0) {
                continue;
            }

            // Allocate space for one additional suspect.
            SuspectStatements *temp = realloc(
                sus_sts,
                (statement_count + 1) * sizeof(SuspectStatements)
            );

            if (temp == NULL) {
                clear_statements();
                fclose(file_path);
                return NULL;
            }

            sus_sts = temp;

            // Store a clean name without brackets or extra spaces.
            sus_sts[statement_count].name = copy_string(name);

            // Start with an empty statement; subsequent lines fill it.
            sus_sts[statement_count].statement = copy_string("");

            // Stop if either allocation failed.
            if (sus_sts[statement_count].name == NULL ||
                sus_sts[statement_count].statement == NULL) {

                free(sus_sts[statement_count].name);
                free(sus_sts[statement_count].statement);

                clear_statements();
                fclose(file_path);
                return NULL;
            }

            // This suspect is now a valid array entry.
            statement_count++;

        } else if (statement_count > 0) {

            // Ordinary lines belong to the most recently added suspect.
            // Add the line and restore its newline separator.
            if (!append_statement(
                    &sus_sts[statement_count - 1].statement,
                    line) ||
                !append_statement(
                    &sus_sts[statement_count - 1].statement,
                    "\n")) {

                clear_statements();
                fclose(file_path);
                return NULL;
            }
        }

        // Lines before the first suspect header are ignored.
    }

    // The parser has finished reading the file.
    fclose(file_path);

    // Return NULL if no suspects were found.
    if (statement_count == 0) {
        return NULL;
    }

    return sus_sts;
}


// Returns the total number of parsed suspects.
int statements_count(void) {
    return statement_count;
}


// Frees all memory allocated by the parser.
void free_statements(void) {
    clear_statements();
}


/* Old logic codes (doesn't work)


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
*/