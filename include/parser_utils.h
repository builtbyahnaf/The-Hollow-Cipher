#ifndef PARSER_UTILS_H
#define PARSER_UTILS_H

#include <stdio.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>

/*
 * =============================================================================
 * PREPROCESSORS: Macros, Safety Wrappers, and Constants
 * =============================================================================
 * Preprocessors run before compilation. They perform text substitutions,
 * conditional compilation, and define reusable functional macros.
 */

/*
 * Debug configuration preprocessor macro.
 * Set to 1 to activate debugging code across the repository (at the 2 allowed locations).
 * Set to 0 for production/standard gameplay.
 */
#ifndef THC_DEBUG
#define THC_DEBUG 0
#endif

/*
 * SAFE_FREE Macro:
 * Evaluates the pointer; if non-NULL, frees it and sets it to NULL.
 * Uses the 'do { ... } while (0)' idiom so it expands as a single syntactic
 * statement that is safe within if-else structures without trailing semicolons.
 */
#define SAFE_FREE(ptr) \
    do { \
        if ((ptr) != NULL) { \
            free(ptr); \
            (ptr) = NULL; \
        } \
    } while (0)

/*
 * ARRAY_SIZE Macro:
 * Computes the number of elements in a statically-sized array at compile time.
 */
#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof((arr)[0]))

/*
 * =============================================================================
 * #CONDITIONAL COMPILATION: Platform-Specific Console Directives
 * =============================================================================
 * Conditional preprocessor directives (#if, #elif, #else, #endif) allow the
 * compiler to selectively include code blocks depending on platform definitions.
 */

#define CLEAR_SCREEN() system("cls")


/*
 * =============================================================================
 * UNION IMPLEMENTATION: Tagged Union (Variant) for Case File Payloads
 * =============================================================================
 * A union in C is a user-defined data type where all members share the same
 * memory location. The size of the union is the size of its largest member.
 * This pattern is memory-efficient because only one payload type is stored
 * at any given moment for a parsed narrative entity.
 */
typedef enum {
    PAYLOAD_TYPE_EVIDENCE = 0,
    PAYLOAD_TYPE_STATEMENT = 1,
    PAYLOAD_TYPE_HISTORY = 2,
    PAYLOAD_TYPE_GENERIC = 3
} CasePayloadType;

typedef union {
    char *evidence_desc;
    char *suspect_statement;
    char *past_history;
    char *raw_text;
} CasePayload;

/*
 * Tagged union container: pairs the CasePayload union with its type tag.
 */
typedef struct {
    char *title;
    CasePayloadType type;
    CasePayload payload;
} CaseRecord;

/*
 * =============================================================================
 * MANUAL GARBAGE COLLECTION: Central Heap Allocation Tracker
 * =============================================================================
 * C lacks automatic runtime garbage collection. To prevent memory leaks during
 * dynamic parsing (e.g., when reading multiline files into heap-allocated strings),
 * this manual GC module records dynamically allocated pointers in an internal registry.
 *
 * Calling gc_collect() sweeps the entire registry, frees all active pointers using
 * SAFE_FREE(), and resets tracking, ensuring zero memory leaks on screen exits or level resets.
 */
#define MAX_GC_ENTRIES 1024

void  *gc_track(void *ptr);
void   gc_untrack(void *ptr);
void   gc_collect(void);
size_t gc_active_count(void);

/*
 * Core parser utility function prototypes
 */
char *copy_string(const char *source);
void  remove_newline(char *text);
bool  is_header(const char *text);

/*
 * Helper functions demonstrating CaseRecord union construction and destruction
 */
CaseRecord create_case_record(const char *title, CasePayloadType type, const char *content);
void free_case_record(CaseRecord *record);

#endif /* PARSER_UTILS_H */
