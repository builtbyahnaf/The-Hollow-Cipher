#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>

#include "parser_utils.h"
#include "interrogation_parser.h"
#include "past_history_parser.h"
#include "evidence_parser.h"

/*
 * =============================================================================
 * MANUAL GARBAGE COLLECTION IMPLEMENTATION
 * =============================================================================
 * The manual GC maintains an internal registry of dynamically allocated heap
 * pointers. When functions like copy_string() allocate heap memory, they register
 * the pointer via gc_track().
 *
 * When the level finishes, unloads, or errors occur, gc_collect() is invoked to
 * iterate over all registered allocations, free them via SAFE_FREE, and nullify
 * the registry slots. This provides deterministic, bulk heap management.
 */
static void *gc_registry[MAX_GC_ENTRIES];
static size_t gc_count = 0;

void *gc_track(void *ptr) {
    if (ptr == NULL) {
        return NULL;
    }

    /* Check if the pointer is already tracked */
    for (size_t i = 0; i < gc_count; i++) {
        if (gc_registry[i] == ptr) {
            return ptr;
        }
    }

    if (gc_count < MAX_GC_ENTRIES) {
        gc_registry[gc_count++] = ptr;
    } else {
        printf("Warning: GC registry full (%d slots). Consider increasing MAX_GC_ENTRIES.\n", MAX_GC_ENTRIES);
    }

    return ptr;
}

void gc_untrack(void *ptr) {
    if (ptr == NULL || gc_count == 0) {
        return;
    }

    for (size_t i = 0; i < gc_count; i++) {
        if (gc_registry[i] == ptr) {
            /* Compact registry by swapping with the last entry */
            gc_registry[i] = gc_registry[gc_count - 1];
            gc_registry[gc_count - 1] = NULL;
            gc_count--;
            return;
        }
    }
}

void gc_collect(void) {
#if defined(THC_DEBUG) && (THC_DEBUG == 1)
    size_t freed_count = 0;
#endif

    for (size_t i = 0; i < gc_count; i++) {
        if (gc_registry[i] != NULL) {
            SAFE_FREE(gc_registry[i]);
#if defined(THC_DEBUG) && (THC_DEBUG == 1)
            freed_count++;
#endif
        }
    }

    gc_count = 0;

    /*
     * =========================================================================
     * DEBUGGING CODE (Place 1 of 2): Manual GC Sweep Diagnostics
     * =========================================================================
     * Uses #conditional compilation to only emit diagnostics when THC_DEBUG is
     * enabled at compile time. Tracks how many dynamic heap objects were freed.
     */
#if defined(THC_DEBUG) && (THC_DEBUG == 1)
    printf("[DEBUG GC] Manual Garbage Collection: reclaimed %zu heap allocations.\n", freed_count);
#endif
}

size_t gc_active_count(void) {
    return gc_count;
}

/*
 * Creates a dynamically allocated copy of a string and registers it with the manual GC.
 */
char *copy_string(const char *source) {
    if (source == NULL) {
        return NULL;
    }

    size_t length = strlen(source);
    char *copy = (char *)malloc(length + 1);

    if (copy == NULL) {
        return NULL;
    }

    strcpy(copy, source);

    /* Register the allocation with manual garbage collection */
    gc_track(copy);

    return copy;
}

/*
 * Removes a line's newline characters without removing other text.
 */
void remove_newline(char *text) {
    if (text == NULL) {
        return;
    }
    text[strcspn(text, "\r\n")] = '\0';
}

/*
 * Checks whether a line is a bracketed header (e.g. [Evidence], [Interrogation]).
 */
bool is_header(const char *text) {
    if (text == NULL) {
        return false;
    }

    while (isspace((unsigned char)*text)) {
        text++;
    }

    int length = strlen(text);

    if (length >= 2 && text[0] == '[' && text[length - 1] == ']') {
        return true;
    }
    return false;
}

/*
 * =============================================================================
 * UNION IMPLEMENTATION HELPER FUNCTIONS
 * =============================================================================
 * Demonstrates the use of the CasePayload tagged union.
 * Depending on the 'type' discriminant, the corresponding union member is initialized,
 * illustrating that all text variants share the same underlying memory slot.
 */
CaseRecord create_case_record(const char *title, CasePayloadType type, const char *content) {
    CaseRecord record;
    record.title = copy_string(title);
    record.type = type;

    /* Union member assignment based on type tag */
    switch (type) {
    case PAYLOAD_TYPE_EVIDENCE:
        record.payload.evidence_desc = copy_string(content);
        break;
    case PAYLOAD_TYPE_STATEMENT:
        record.payload.suspect_statement = copy_string(content);
        break;
    case PAYLOAD_TYPE_HISTORY:
        record.payload.past_history = copy_string(content);
        break;
    case PAYLOAD_TYPE_GENERIC:
    default:
        record.payload.raw_text = copy_string(content);
        break;
    }

    return record;
}

void free_case_record(CaseRecord *record) {
    if (record == NULL) {
        return;
    }

    /* Untrack and free title */
    if (record->title != NULL) {
        gc_untrack(record->title);
        SAFE_FREE(record->title);
    }

    /* Free union payload (all pointers point to the same shared address) */
    if (record->payload.raw_text != NULL) {
        gc_untrack(record->payload.raw_text);
        SAFE_FREE(record->payload.raw_text);
    }
}
