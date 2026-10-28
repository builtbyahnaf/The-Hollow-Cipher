#ifndef PROGRESS_H
#define PROGRESS_H

#define MAX_LEVELS 2

/*
 * =============================================================================
 * PACKING IMPLEMENTATION: Struct Packing for Flat-File Database Records
 * =============================================================================
 * In standard C, the compiler introduces alignment padding between structure
 * members to align variables with CPU word boundaries (e.g. 4 or 8 bytes).
 *
 * '#pragma pack(push, 1)' pushes the current alignment setting onto the internal
 * compiler stack and sets current alignment to 1-byte boundaries, stripping all
 * padding bytes.
 * '#pragma pack(pop)' pops the setting and restores the original compiler alignment.
 *
 * PackedProgressRecord provides a guaranteed byte-level memory layout:
 *   - level_index: 1 byte
 *   - is_unlocked: 1 byte
 *   - Total memory footprint: exactly 2 bytes with zero padding.
 */
#pragma pack(push, 1)
typedef struct {
    unsigned char level_index;
    unsigned char is_unlocked;
} PackedProgressRecord;
#pragma pack(pop)

void load_progress(void);
int  is_level_unlocked(int level_index);
void set_level_unlocked(int level_index);
void reset_progress(void);

#endif /* PROGRESS_H */
