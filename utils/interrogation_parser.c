/**
 * =============================================================================
 * PROJECT: The Hollow Cipher (THC)
 * FILE: utils/interrogation_parser.c
 * PURPOSE: Interrogation / Suspect Statement File Parser
 *
 * DESCRIPTION:
 *   This module reads "data/level_<N>/interrogation.txt", parses bracketed
 *   suspect name headers, and accumulates their multi-line recorded statement
 *   bodies into heap-allocated strings.
 *
 * DATA FILE FORMAT (interrogation.txt):
 *   [Interrogation]
 *   [<Suspect Name>]
 *   <statement line 1>
 *   <statement line 2>
 *   [<Next Suspect Name>]
 *   <statement lines>
 *   ...
 *
 * MEMORY MODEL:
 *   - sus_sts is a dynamically resized array of SuspectStatements structs.
 *   - Each struct holds heap-allocated name and statement strings registered
 *     with the manual GC.
 *   - Statements grow incrementally via append_statement(), which reallocates
 *     and re-registers the potentially relocated pointer with the GC.
 *   - clear_statements() / free_statements() perform full teardown.
 *
 * NOTE:
 *   This file follows the identical parsing pattern as evidence_parser.c and
 *   past_history_parser.c. The architecture is intentionally symmetric for
 *   consistency and maintainability.
 * =============================================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "interrogation_parser.h"
#include "parser_utils.h"

/* Number of suspects currently stored in the array */
static int statement_count = 0;

/* Dynamically allocated array containing all parsed suspect records */
static SuspectStatements *sus_sts = NULL;

/**
 * @brief Frees all dynamically allocated suspect statement data and resets module state.
 *
 * Iterates over each SuspectStatements entry, unregistering and freeing the
 * name and statement heap strings. Then frees and NULLs the sus_sts array itself.
 *
 * gc_untrack() is called before SAFE_FREE() to keep the GC registry consistent
 * and avoid double-free during any subsequent gc_collect() call.
 */
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

/**
 * @brief Appends a line of text to a suspect's statement buffer.
 *
 * Reallocates the statement buffer to accommodate the incoming line.
 * Because realloc() may move the block to a new address, the old pointer
 * is untracked from the GC before the assignment and the new pointer is
 * re-registered immediately after.
 *
 * Uses memcpy() starting at the old null-terminator to concatenate the
 * new line while preserving previous content.
 *
 * @param statement  Pointer-to-pointer to the current statement buffer.
 * @param line       Null-terminated text line to append.
 * @return 1 on success, 0 on realloc failure (caller handles cleanup).
 */
static int append_statement(char **statement, const char *line) {
    int old_length = strlen(*statement);
    int line_length = strlen(line);

    /* Reallocate to accommodate existing content + new line + null terminator */
    char *temp = realloc(
        *statement,
        old_length + line_length + 1
    );

    if (temp == NULL) {
        return 0; /* Allocation failed */
    }

    /*
     * Deregister old pointer before updating *statement, since realloc
     * may have returned a different address (old pointer is now invalid).
     */
    gc_untrack(*statement);
    *statement = temp;
    gc_track(*statement); /* Re-register the new (or same) pointer */

    /* Concatenate new line starting from where the null terminator was */
    memcpy(
        *statement + old_length,
        line,
        line_length + 1 /* Include new null terminator */
    );

    return 1;
}

/**
 * @brief Parses an interrogation file and returns a dynamic array of SuspectStatements.
 *
 * Parsing Logic:
 *   1. Calls clear_statements() to discard any data from a prior parse.
 *   2. Reads lines via fgets() with a 2048-byte buffer.
 *   3. Strips line endings with remove_newline().
 *   4. For header lines (detected by is_header()):
 *        - "[Interrogation]" is the file-level section tag; it is skipped.
 *        - All other headers are treated as new suspect names.
 *        - sus_sts is grown by one slot via realloc().
 *        - The suspect name is stored via copy_string() and statement is
 *          initialized to an empty string "".
 *   5. For non-header lines (after at least one suspect has been parsed):
 *        - Appends the text + "\n" to the most recent suspect's statement.
 *   6. Closes the file and returns sus_sts, or NULL if empty.
 *
 * @param file_path Open FILE* handle to the level's interrogation.txt.
 * @return Pointer to the internal sus_sts array, or NULL on failure / empty file.
 */
SuspectStatements *parse_interrogation(FILE *file_path) {
    char line[2048];

    /* Remove results from any previous parsing operation */
    clear_statements();

    if (file_path == NULL) {
        return NULL;
    }

    /* Read the file one line at a time */
    while (fgets(line, sizeof(line), file_path) != NULL) {

        remove_newline(line); /* Strip \r\n to get a clean comparison string */

        /* Check whether this line is a section or suspect header */
        if (is_header(line)) {
            char *name = line;

            /* "[Interrogation]" identifies the file section, not a suspect */
            if (strcmp(name, "[Interrogation]") == 0) {
                continue;
            }

            /* Expand the array to hold one more suspect record */
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

            /* Store suspect name and initialize with an empty statement */
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
            /*
             * Body text line: append to the statement of the most recently
             * added suspect, followed by a newline for console formatting.
             */
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
        return NULL; /* File contained no usable suspect entries */
    }

    return sus_sts;
}

/**
 * @brief Returns the total number of suspects parsed in the last parse_interrogation() call.
 *
 * @return Integer count of SuspectStatements records currently held in sus_sts.
 */
int statements_count(void) {
    return statement_count;
}

/**
 * @brief Fully releases all suspect statement memory and triggers a GC sweep.
 *
 * Calls clear_statements() to untrack and free individual suspect records,
 * then gc_collect() to sweep any remaining tracked allocations.
 */
void free_statements(void) {
    clear_statements();
    gc_collect();
}
