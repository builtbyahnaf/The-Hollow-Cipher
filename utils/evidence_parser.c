/**
 * =============================================================================
 * PROJECT: The Hollow Cipher (THC)
 * FILE: utils/evidence_parser.c
 * PURPOSE: Evidence File Parser with Dynamic Array and GC Integration
 *
 * DESCRIPTION:
 *   This module reads "data/level_<N>/evidence.txt", parses bracketed section
 *   headers as individual evidence item names, and accumulates their multi-line
 *   description bodies into heap-allocated strings.
 *
 * DATA FILE FORMAT (evidence.txt):
 *   [Evidences]
 *   [<Evidence Item Name>]
 *   <line 1 of description>
 *   <line 2 of description>
 *   [<Next Evidence Item Name>]
 *   <description lines>
 *   ...
 *
 * MEMORY MODEL:
 *   - ev_list is a dynamically resized array of Evidence structs (via realloc).
 *   - Each struct contains heap-allocated name and description strings registered
 *     with the manual GC via copy_string() and gc_track().
 *   - Descriptions grow incrementally via append_description() which reallocates
 *     the buffer and reregisters the potentially relocated pointer.
 *   - clear_evidence() / free_evidence() perform full cleanup via gc_untrack +
 *     SAFE_FREE + gc_collect().
 * =============================================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>

#include "evidence_parser.h"
#include "parser_utils.h"

/* Total number of evidence items parsed in the most recent parse_evidence() call */
static int ev_count = 0;

/* Pointer to the dynamically allocated Evidence struct array */
static Evidence *ev_list = NULL;

/**
 * @brief Frees all dynamically allocated evidence data and resets the module state.
 *
 * Iterates over every parsed Evidence entry, unregistering and freeing both
 * the name and description heap strings. Then frees and NULLs the ev_list
 * array itself and resets ev_count to 0.
 *
 * gc_untrack() is called before SAFE_FREE() so the GC registry stays consistent
 * and avoids double-free during a later gc_collect() sweep.
 */
static void clear_evidence(void) {
    for (int i = 0; i < ev_count; i++) {
        gc_untrack(ev_list[i].name);
        SAFE_FREE(ev_list[i].name);
        gc_untrack(ev_list[i].description);
        SAFE_FREE(ev_list[i].description);
    }

    SAFE_FREE(ev_list);

    ev_list = NULL;
    ev_count = 0;
}

/**
 * @brief Appends a text line to an evidence item's description string.
 *
 * Reallocates the description buffer to accommodate the incoming line.
 * Because realloc() may return a different pointer (if it moves the block
 * to a new memory address), the old pointer is first untracked from the GC
 * registry and the new pointer is re-registered after assignment.
 *
 * Uses memcpy() starting at the old null-terminator position to concatenate
 * the new line while preserving the existing content.
 *
 * @param description  Pointer-to-pointer to the current description buffer.
 * @param line         Null-terminated text line to append.
 * @return 1 on success, 0 on realloc failure (caller handles cleanup).
 */
static int append_description(char **description, const char *line) {
    int old_length = strlen(*description);
    int line_length = strlen(line);

    /* Reallocate to hold old content + new line + null terminator */
    char *temp = realloc(
        *description,
        old_length + line_length + 1
    );

    if (temp == NULL) {
        return 0; /* Allocation failed */
    }

    /*
     * Deregister old pointer from the GC before updating *description,
     * since realloc may have invalidated the old address.
     */
    gc_untrack(*description);
    *description = temp;
    gc_track(*description); /* Re-register the (possibly new) pointer */

    /* Append new line starting at the old null-terminator position */
    memcpy(
        *description + old_length,
        line,
        line_length + 1 /* Copy including new null terminator */
    );

    return 1;
}

/**
 * @brief Parses an evidence file and returns a dynamic array of Evidence structs.
 *
 * Parsing Logic:
 *   1. Calls clear_evidence() to discard any data from a prior parse.
 *   2. Reads lines via fgets() with a 2048-byte buffer (sufficient for long descriptions).
 *   3. Strips line endings with remove_newline().
 *   4. For header lines (detected by is_header()):
 *        - "[Evidences]" is the file-level section tag; it is skipped.
 *        - All other headers are treated as new evidence item names.
 *        - ev_list is grown by one slot via realloc(), the name is stored
 *          via copy_string(), and the description is initialized to "".
 *   5. For non-header lines (after at least one item has been parsed):
 *        - Appends the text + "\n" to the current item's description buffer.
 *   6. Closes the file and returns ev_list (or NULL if no items were found).
 *
 * @param file_path Open FILE* handle to the level's evidence.txt.
 * @return Pointer to the internal ev_list array, or NULL on failure / empty file.
 */
Evidence *parse_evidence(FILE *file_path) {
    char line[2048];

    /* Discard any previously parsed evidence data */
    clear_evidence();

    if (file_path == NULL) {
        return NULL;
    }

    while (fgets(line, sizeof(line), file_path) != NULL) {
        remove_newline(line); /* Strip \r\n to get a clean comparison string */

        if (is_header(line)) {
            char *name = line;

            /* "[Evidences]" is the file-level section marker, not an item name */
            if (strcmp(name, "[Evidences]") == 0) {
                continue;
            }

            /*
             * Grow ev_list by one entry to hold the new evidence item.
             * realloc() preserves all previous entries in memory.
             */
            Evidence *temp = realloc(
                ev_list,
                (ev_count + 1) * sizeof(Evidence)
            );

            if (temp == NULL) {
                clear_evidence();
                fclose(file_path);
                return NULL;
            }

            ev_list = temp;

            /* Initialize new entry: copy name and set empty description */
            ev_list[ev_count].name = copy_string(name);
            ev_list[ev_count].description = copy_string("");

            if (ev_list[ev_count].name == NULL ||
                ev_list[ev_count].description == NULL) {
                clear_evidence();
                fclose(file_path);
                return NULL;
            }

            ev_count++;

        } else if (ev_count > 0) {
            /*
             * Body text line: append to the description of the most recently
             * added evidence item, followed by a newline for formatting.
             */
            if (!append_description(&ev_list[ev_count - 1].description, line) ||
                !append_description(&ev_list[ev_count - 1].description, "\n")) {
                clear_evidence();
                fclose(file_path);
                return NULL;
            }
        }
    }

    fclose(file_path);

    if (ev_count == 0) {
        return NULL; /* File contained no usable evidence entries */
    }

    return ev_list;
}

/**
 * @brief Returns the number of evidence items parsed in the last parse_evidence() call.
 *
 * @return Integer count of evidence items currently held in ev_list.
 */
int evidence_count(void) {
    return ev_count;
}

/**
 * @brief Fully releases all evidence memory and triggers a GC sweep.
 *
 * Calls clear_evidence() to untrack and free individual items,
 * then gc_collect() to sweep any remaining tracked allocations that were
 * not explicitly freed (e.g., orphaned realloc intermediates).
 */
void free_evidence(void) {
    clear_evidence();
    /* Manual GC sweep to clean up any remaining tracked allocations */
    gc_collect();
}
