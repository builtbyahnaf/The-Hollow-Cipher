/**
 * =============================================================================
 * PROJECT: The Hollow Cipher (THC)
 * FILE: utils/past_history_parser.c
 * PURPOSE: Past History / Suspect Dossier File Parser
 *
 * DESCRIPTION:
 *   This module reads "data/level_<N>/past_history.txt", parses bracketed
 *   person-name headers as individual dossier entries, and accumulates their
 *   multi-line history text into heap-allocated strings.
 *
 * DATA FILE FORMAT (past_history.txt):
 *   [Past History]
 *   [<Person Name>]
 *   <history line 1>
 *   <history line 2>
 *   [Investigative Direction]   <- optional advisory section; skipped
 *   ...
 *
 * MEMORY MODEL:
 *   - ph_list is a dynamically resized array of PastHistory structs.
 *   - Each struct holds heap-allocated name and history strings registered
 *     with the manual GC via copy_string() and gc_track().
 *   - History entries grow incrementally via append_history(), which reallocates
 *     the buffer and re-registers the possibly relocated pointer with the GC.
 *   - clear_history() / free_past_history() perform full teardown.
 *
 * NOTE:
 *   This file follows the identical parsing pattern as evidence_parser.c and
 *   interrogation_parser.c. The architecture is intentionally parallel across
 *   all three parsers for consistency and maintainability.
 * =============================================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>

#include "past_history_parser.h"
#include "parser_utils.h"

/* Total number of history entries parsed in the most recent parse call */
static int history_count = 0;

/* Pointer to the dynamically allocated PastHistory struct array */
static PastHistory *ph_list = NULL;

/**
 * @brief Frees all dynamically allocated history data and resets module state.
 *
 * Iterates over each PastHistory entry, unregistering and freeing the
 * name and history heap strings. Then frees and NULLs the ph_list array.
 *
 * gc_untrack() is called before SAFE_FREE() to keep the GC registry consistent
 * and prevent double-free during any subsequent gc_collect() sweep.
 */
static void clear_history(void) {
    for (int i = 0; i < history_count; i++) {
        gc_untrack(ph_list[i].name);
        SAFE_FREE(ph_list[i].name);
        gc_untrack(ph_list[i].history);
        SAFE_FREE(ph_list[i].history);
    }

    SAFE_FREE(ph_list);

    ph_list = NULL;
    history_count = 0;
}

/**
 * @brief Appends a line of text to a history entry's buffer.
 *
 * Reallocates the history buffer to accommodate the incoming line.
 * Because realloc() may relocate the block, the old pointer is untracked
 * from the GC before assignment and the new pointer is re-registered after.
 *
 * Uses memcpy() starting at the old null-terminator to concatenate while
 * preserving existing content.
 *
 * @param history  Pointer-to-pointer to the current history text buffer.
 * @param line     Null-terminated text line to append.
 * @return 1 on success, 0 on realloc failure (caller handles cleanup).
 */
static int append_history(char **history, const char *line) {
    int old_length = strlen(*history);
    int line_length = strlen(line);

    /* Reallocate to hold existing content + new line + null terminator */
    char *temp = realloc(
        *history,
        old_length + line_length + 1
    );

    if (temp == NULL) {
        return 0; /* Allocation failed */
    }

    /*
     * Deregister old pointer before updating *history, since realloc
     * may have returned a different address (old pointer is now invalid).
     */
    gc_untrack(*history);
    *history = temp;
    gc_track(*history); /* Re-register the new (or same) pointer */

    /* Concatenate new line starting from where the null terminator was */
    memcpy(
        *history + old_length,
        line,
        line_length + 1 /* Include new null terminator */
    );

    return 1;
}

/**
 * @brief Parses a past_history file and returns a dynamic array of PastHistory structs.
 *
 * Parsing Logic:
 *   1. Calls clear_history() to discard any data from a prior parse.
 *   2. Reads lines via fgets() with a 2048-byte buffer.
 *   3. Strips line endings with remove_newline().
 *   4. For header lines (detected by is_header()):
 *        - "[Past History]" is the file-level section tag; skipped.
 *        - "[Investigative Direction]" is an advisory metadata tag; skipped.
 *        - All other headers are treated as person names (dossier entries).
 *        - ph_list is grown by one slot via realloc(), name is stored via
 *          copy_string(), and history is initialized to an empty string "".
 *   5. For non-header lines (after at least one entry has been parsed):
 *        - Appends the text + "\n" to the most recent entry's history buffer.
 *   6. Closes the file and returns ph_list, or NULL if no entries found.
 *
 * @param file_path Open FILE* handle to the level's past_history.txt.
 * @return Pointer to the internal ph_list array, or NULL on failure / empty file.
 */
PastHistory *parse_past_history(FILE *file_path) {
    char line[2048];

    /* Discard data from any prior parse operation */
    clear_history();

    if (file_path == NULL) {
        return NULL;
    }

    while (fgets(line, sizeof(line), file_path) != NULL) {
        remove_newline(line); /* Strip \r\n to get a clean comparison string */

        if (is_header(line)) {
            char *name = line;

            /*
             * Skip file-level section tags that are not person names.
             * "[Investigative Direction]" is an optional advisory section.
             */
            if (strcmp(name, "[Past History]") == 0 ||
                strcmp(name, "[Investigative Direction]") == 0) {
                continue;
            }

            /* Grow ph_list by one slot for the new history entry */
            PastHistory *temp = realloc(
                ph_list,
                (history_count + 1) * sizeof(PastHistory)
            );

            if (temp == NULL) {
                clear_history();
                fclose(file_path);
                return NULL;
            }

            ph_list = temp;

            /* Initialize new entry: copy name and start with an empty history */
            ph_list[history_count].name = copy_string(name);
            ph_list[history_count].history = copy_string("");

            if (ph_list[history_count].name == NULL ||
                ph_list[history_count].history == NULL) {
                clear_history();
                fclose(file_path);
                return NULL;
            }

            history_count++;

        } else if (history_count > 0) {
            /*
             * Body text line: append to the history of the most recently
             * added dossier entry, followed by a newline for console formatting.
             */
            if (!append_history(&ph_list[history_count - 1].history, line) ||
                !append_history(&ph_list[history_count - 1].history, "\n")) {
                clear_history();
                fclose(file_path);
                return NULL;
            }
        }
    }

    fclose(file_path);

    if (history_count == 0) {
        return NULL; /* File contained no usable history entries */
    }

    return ph_list;
}

/**
 * @brief Returns the number of history entries parsed in the last parse_past_history() call.
 *
 * @return Integer count of PastHistory records currently held in ph_list.
 */
int past_history_count(void) {
    return history_count;
}

/**
 * @brief Fully releases all past history memory and triggers a GC sweep.
 *
 * Calls clear_history() to untrack and free individual records,
 * then gc_collect() to sweep any remaining tracked allocations.
 */
void free_past_history(void) {
    clear_history();
    gc_collect();
}
