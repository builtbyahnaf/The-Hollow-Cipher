#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "interrogation_parser.h"
#include "parser_utils.h"

// Number of suspects currently stored in the array.
static int statement_count = 0;

// Dynamically allocated array containing all suspects.
static SuspectStatements *sus_sts = NULL;

// Frees all currently stored suspects and their strings using SAFE_FREE and manual GC untracking.
static void clear_statements(void) {
    for (int i = 0; i < statement_count; i++) {
        gc_untrack(sus_sts[i].name);
        SAFE_FREE(sus_sts[i].name);
        gc_untrack(sus_sts[i].statement);
        SAFE_FREE(sus_sts[i].statement);
    }

    SAFE_FREE(sus_sts);

    sus_sts = NULL;
    statement_count = 0;
}

// Appends a line to a suspect's existing statement.
// The statement grows dynamically as more lines are read.
static int append_statement(char **statement, const char *line) {
    int old_length = strlen(*statement);
    int line_length = strlen(line);

    char *temp = realloc(
        *statement,
        old_length + line_length + 1
    );

    if (temp == NULL) {
        return 0;
    }

    gc_untrack(*statement);
    *statement = temp;
    gc_track(*statement);

    memcpy(
        *statement + old_length,
        line,
        line_length + 1
    );

    return 1;
}

// Reads an interrogation file and builds the suspect array.
SuspectStatements *parse_interrogation(FILE *file_path) {
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
            char *name = line;

            // [Interrogation] identifies the section, not a suspect.
            if (strcmp(name, "[Interrogation]") == 0) {
                continue;
            }

            // Expand the array to hold one more suspect.
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

            // Store suspect name and start with an empty statement.
            sus_sts[statement_count].name = copy_string(name);
            sus_sts[statement_count].statement = copy_string("");

            if (sus_sts[statement_count].name == NULL ||
                sus_sts[statement_count].statement == NULL) {

                clear_statements();
                fclose(file_path);
                return NULL;
            }

            statement_count++;
        } else if (statement_count > 0) {
            // Append line to current suspect's statement.
            if (!append_statement(&sus_sts[statement_count - 1].statement, line) ||
                !append_statement(&sus_sts[statement_count - 1].statement, "\n")) {

                clear_statements();
                fclose(file_path);
                return NULL;
            }
        }
    }

    fclose(file_path);

    if (statement_count == 0) {
        return NULL;
    }

    return sus_sts;
}

// Returns the total number of parsed suspects.
int statements_count(void) {
    return statement_count;
}

// Frees all memory allocated by the parser and triggers manual GC cleanup.
void free_statements(void) {
    clear_statements();
    gc_collect();
}
