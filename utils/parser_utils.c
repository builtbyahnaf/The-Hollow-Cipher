/**
 * =============================================================================
 * PROJECT: The Hollow Cipher (THC)
 * FILE: utils/parser_utils.c
 * PURPOSE: Shared Parser Utilities, Manual Garbage Collector, and Union Demo
 *
 * DESCRIPTION:
 *   This module is the backbone of the THC parsing subsystem. It provides:
 *
 *   1. MANUAL GARBAGE COLLECTOR (GC)
 *      C has no automatic runtime memory management. Every call to malloc()
 *      or realloc() that escapes a function scope risks a memory leak if the
 *      pointer is lost (e.g., due to an error path or screen transition).
 *
 *      This GC implements a fixed-size pointer registry (gc_registry[]) of up
 *      to MAX_GC_ENTRIES (1024) heap addresses. Parsers register allocations
 *      via gc_track() and deregister before intentional frees via gc_untrack().
 *      A single gc_collect() sweep frees all remaining tracked pointers in bulk.
 *
 *   2. CORE PARSER HELPERS
 *      - copy_string(): Heap-allocates a string copy and auto-registers it with the GC.
 *      - remove_newline(): Strips \r\n endings from fgets() output.
 *      - is_header(): Detects bracketed section markers like [Evidence] or [Interrogation].
 *
 *   3. TAGGED UNION DEMONSTRATION (CaseRecord / CasePayload)
 *      Illustrates a practical tagged union (discriminated union / variant type).
 *      All payload variants (evidence_desc, suspect_statement, etc.) share one
 *      memory block, selected via the CasePayloadType enum discriminant.
 * =============================================================================
 */

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
 * the registry slots. This provides deterministic, bulk heap management without
 * requiring individual caller cleanup paths at every site.
 *
 * Registry structure:
 *   gc_registry[] — Fixed-size array of void* pointers (max MAX_GC_ENTRIES = 1024)
 *   gc_count      — Current number of live tracked allocations
 */
static void *gc_registry[MAX_GC_ENTRIES];
static size_t gc_count = 0;

/**
 * @brief Registers a heap pointer with the manual garbage collector.
 *
 * Adds `ptr` to gc_registry[] if not already tracked and registry is not full.
 * Duplicate registrations are silently ignored (idempotent).
 *
 * @param ptr Pointer to the heap allocation to track. NULL is a no-op.
 * @return The same pointer `ptr` (allows use inline as: `p = gc_track(malloc(...))`).
 */
void *gc_track(void *ptr) {
    if (ptr == NULL) {
        return NULL;
    }

    /* Check if pointer is already registered to avoid duplicate entries */
    for (size_t i = 0; i < gc_count; i++) {
        if (gc_registry[i] == ptr) {
            return ptr; /* Already tracked; no action needed */
        }
    }

    if (gc_count < MAX_GC_ENTRIES) {
        gc_registry[gc_count++] = ptr;
    } else {
        /* Registry overflow warning — consider increasing MAX_GC_ENTRIES */
        printf("Warning: GC registry full (%d slots). Consider increasing MAX_GC_ENTRIES.\n", MAX_GC_ENTRIES);
    }

    return ptr;
}

/**
 * @brief Removes a pointer from the GC registry without freeing it.
 *
 * Called before an explicit free() to prevent double-free when the caller
 * handles deallocation manually (e.g., clear_evidence(), clear_statements()).
 *
 * Uses swap-with-last compaction to keep the registry dense and avoid
 * shifting the entire array on each removal (O(n) scan, O(1) removal).
 *
 * @param ptr Pointer to deregister. NULL or untracked pointers are silently ignored.
 */
void gc_untrack(void *ptr) {
    if (ptr == NULL || gc_count == 0) {
        return;
    }

    for (size_t i = 0; i < gc_count; i++) {
        if (gc_registry[i] == ptr) {
            /*
             * Compact registry: overwrite this slot with the last entry,
             * then null and decrement the tail. O(1) removal.
             */
            gc_registry[i] = gc_registry[gc_count - 1];
            gc_registry[gc_count - 1] = NULL;
            gc_count--;
            return;
        }
    }
}

/**
 * @brief Frees all tracked heap allocations and clears the GC registry.
 *
 * Sweeps the entire gc_registry[], freeing all non-NULL pointers via SAFE_FREE.
 * Resets gc_count to 0 after the sweep.
 *
 * When compiled with THC_DEBUG=1, emits a diagnostic count of reclaimed allocations.
 * This is placed at "Debugging Code Place 1 of 2" per project convention.
 */
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

    /* Reset registry to empty state */
    gc_count = 0;

    /*
     * =========================================================================
     * DEBUGGING CODE (Place 1 of 2): Manual GC Sweep Diagnostics
     * =========================================================================
     * Uses conditional compilation (#if / #endif) to only emit diagnostics when
     * THC_DEBUG is set to 1 at compile time (e.g., gcc -DTHC_DEBUG=1).
     * This avoids any performance overhead or console noise in release builds.
     */
#if defined(THC_DEBUG) && (THC_DEBUG == 1)
    printf("[DEBUG GC] Manual Garbage Collection: reclaimed %zu heap allocations.\n", freed_count);
#endif
}

/**
 * @brief Returns the current number of pointers tracked by the GC.
 *
 * Useful for debugging — a non-zero count after gc_collect() would indicate
 * the GC was called while untracked allocations remained active.
 *
 * @return Number of currently registered pointers.
 */
size_t gc_active_count(void) {
    return gc_count;
}

/**
 * @brief Heap-allocates a copy of a C string and registers it with the GC.
 *
 * Combines malloc() + strcpy() into a single safe helper. The resulting
 * pointer is automatically gc_track()'d so it participates in bulk cleanup
 * via gc_collect().
 *
 * @param source Source null-terminated string to duplicate. NULL returns NULL.
 * @return Heap-allocated copy of source, or NULL on allocation failure.
 */
char *copy_string(const char *source) {
    if (source == NULL) {
        return NULL;
    }

    size_t length = strlen(source);
    char *copy = (char *)malloc(length + 1); /* +1 for null terminator */

    if (copy == NULL) {
        return NULL;
    }

    strcpy(copy, source);

    /* Register the allocation with manual garbage collection */
    gc_track(copy);

    return copy;
}

/**
 * @brief Strips trailing newline ('\n') and carriage return ('\r') from a string.
 *
 * fgets() retains the newline character read from the file. This helper
 * truncates the string at the first occurrence of either line-ending character
 * using strcspn(), which returns the index of the first matching byte.
 *
 * Example:
 *   Input:  "Hello\r\n"
 *   Output: "Hello"
 *
 * @param text Mutable null-terminated string to strip in-place. NULL is a no-op.
 */
void remove_newline(char *text) {
    if (text == NULL) {
        return;
    }
    text[strcspn(text, "\r\n")] = '\0';
}

/**
 * @brief Checks whether a line is a bracketed section header.
 *
 * Recognizes tags of the form [Tag Name] as used throughout the game data files.
 * Leading whitespace is skipped before checking brackets.
 *
 * Examples of valid headers:
 *   "[Evidence]", "[Interrogation]", "[Past History]", "[Answer]", "[Explanation]"
 *
 * @param text Null-terminated string to test. NULL returns false.
 * @return true if the string begins with '[' and ends with ']', false otherwise.
 */
bool is_header(const char *text) {
    if (text == NULL) {
        return false;
    }

    /* Skip leading whitespace before checking bracket characters */
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
 * CaseRecord pairs a title string with a CasePayload tagged union.
 *
 * In C, a union allocates only as much memory as its largest member. All
 * members (evidence_desc, suspect_statement, past_history, raw_text) are
 * char* pointers occupying the same 4/8-byte slot (depending on the platform).
 *
 * The 'type' discriminant (CasePayloadType enum) is stored alongside the union
 * in CaseRecord. The caller must check 'type' before accessing a union member,
 * as accessing the wrong member yields undefined behaviour.
 *
 * This pattern is commonly called a "tagged union" or "discriminated union".
 */

/**
 * @brief Constructs a CaseRecord with a typed union payload.
 *
 * Copies title and content strings onto the heap via copy_string() (GC-tracked).
 * Assigns the content to the appropriate union member based on the type tag.
 *
 * @param title   Null-terminated record title string.
 * @param type    Discriminant identifying which union member to populate.
 * @param content Null-terminated payload content string.
 * @return Populated CaseRecord struct by value.
 */
CaseRecord create_case_record(const char *title, CasePayloadType type, const char *content) {
    CaseRecord record;
    record.title = copy_string(title);
    record.type = type;

    /*
     * Union member assignment based on the type discriminant.
     * All branches call copy_string() which writes to the same memory address
     * in the union — only the semantic label differs.
     */
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

/**
 * @brief Releases all heap memory held by a CaseRecord.
 *
 * Unregisters from the GC and frees both the title and the union payload.
 * Since all union members share the same memory address, freeing raw_text
 * always correctly frees the active payload regardless of which member was set.
 *
 * @param record Pointer to the CaseRecord to free. NULL is a no-op.
 */
void free_case_record(CaseRecord *record) {
    if (record == NULL) {
        return;
    }

    /* Untrack and free the title string */
    if (record->title != NULL) {
        gc_untrack(record->title);
        SAFE_FREE(record->title);
    }

    /*
     * Free union payload via raw_text member.
     * Because all union members share the same memory, this correctly
     * frees whichever member was actually assigned in create_case_record().
     */
    if (record->payload.raw_text != NULL) {
        gc_untrack(record->payload.raw_text);
        SAFE_FREE(record->payload.raw_text);
    }
}
