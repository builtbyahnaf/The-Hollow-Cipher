/**
 * =============================================================================
 * PROJECT: The Hollow Cipher (THC)
 * FILE: utils/progress.c
 * PURPOSE: Player Campaign Progression Persistence Layer
 *
 * DESCRIPTION:
 *   This module manages loading, saving, and resetting the player's campaign
 *   save data, which tracks which levels have been unlocked.
 *
 *   Save data is stored in "data/progress.thcdb" as a plain-text database
 *   with a version signature header:
 *
 *   FILE FORMAT (progress.thcdb):
 *     THCDB_V1
 *     <level_index> <is_unlocked>
 *     <level_index> <is_unlocked>
 *     ...
 *
 *   KEY DESIGN DECISIONS:
 *   - Level 1 (index 0) is always unlocked, even if corrupted save data says otherwise.
 *     This is enforced both on load and after every write.
 *   - "THCDB_V1" is a versioned signature to detect stale or malformed files.
 *   - set_level_unlocked() and reset_progress() demonstrate the use of
 *     PackedProgressRecord — a #pragma pack(push,1) struct with no padding.
 *     Each record is exactly 2 bytes on any compiler/platform.
 * =============================================================================
 */

#include <stdio.h>
#include <stdbool.h>
#include <string.h>

#include "progress.h"
#include "parser_utils.h"

/*
 * In-memory level unlock state array.
 * Index maps directly to level number - 1:
 *   level_states[0] = Level 1 (always true)
 *   level_states[1] = Level 2
 *   ...
 *   level_states[4] = Level 5
 *
 * Level 1 (index 0) is hardcoded to always be unlocked.
 */
static bool level_states[MAX_LEVELS] = {true, false, false, false, false};

/**
 * @brief Validates the database signature at the current file position.
 *
 * Reads the first whitespace-delimited token from the file and compares it
 * against the expected version string "THCDB_V1". A mismatch indicates the
 * file is either corrupted, from an incompatible version, or not a valid
 * progress database.
 *
 * @param pfile Open FILE* handle positioned at the start of the database.
 * @return true if signature matches "THCDB_V1", false otherwise.
 */
bool is_signature_valid(FILE *pfile) {
    char signature[10];
    if (fscanf(pfile, "%9s", signature) != 1) {
        return false; /* File is empty or read failed */
    }
    return strcmp(signature, "THCDB_V1") == 0;
}

/**
 * @brief Loads campaign progress from "data/progress.thcdb" into memory.
 *
 * Execution Steps:
 *   1. Resets level_states[0] = true (Level 1 always unlocked before loading).
 *   2. Opens progress.thcdb; prints an error and returns if file is missing.
 *   3. Validates the "THCDB_V1" signature via is_signature_valid().
 *   4. Reads pairs of (level_index, state) integers line-by-line:
 *        - index 0:         Forced to true (Level 1 always unlocked).
 *        - index 1-4, valid state (0 or 1): Sets level_states[index].
 *        - index >= MAX_LEVELS: Skipped gracefully for forward compatibility.
 *        - Any other value: Treated as data corruption; breaks the loop.
 *   5. Re-enforces level_states[0] = true after loading to override any
 *      corrupted save data that might have set it to false.
 */
void load_progress(void) {
    /* Level 1 must always be unlocked before reading save data */
    level_states[0] = true;

    FILE *pfile = fopen("data/progress.thcdb", "r");
    if (!pfile) {
        printf("Error reading file. The progress data is either missing or corrupted.\n");
        return;
    }

    /* Verify the file is a valid THCDB version 1 database */
    if (!is_signature_valid(pfile)) {
        printf("Invalid signature detected\n");
        fclose(pfile);
        return;
    }

    int level_index, state;
    while (fscanf(pfile, "%d %d", &level_index, &state) == 2) {
        if (level_index == 0) {
            /* Level 1 is always unlocked regardless of what the file says */
            level_states[0] = true;
        } else if (level_index > 0 && level_index < MAX_LEVELS && (state == 0 || state == 1)) {
            /* Valid record: update in-memory state */
            level_states[level_index] = (state != 0);
        } else if (level_index >= MAX_LEVELS && (state == 0 || state == 1)) {
            /* Gracefully skip extra levels beyond current MAX_LEVELS */
            continue;
        } else {
            /* Unexpected value — file may be corrupted or malformed */
            printf("The progress data is corrupted.\n");
            break;
        }
    }

    /* Final enforcement: Level 1 stays unlocked even after loading */
    level_states[0] = true;
    fclose(pfile);
}

/**
 * @brief Checks if a specific level is currently unlocked.
 *
 * Level 1 (index 0) is always reported as unlocked regardless of save state.
 * Other indices are checked against the in-memory level_states[] array.
 *
 * @param level_index 0-based level index (0 = Level 1, 4 = Level 5).
 * @return 1 if the level is unlocked, 0 if locked or index is out of range.
 */
int is_level_unlocked(int level_index) {
    /* Level 1 (index 0) is hardcoded to always be unlocked */
    if (level_index == 0) {
        return 1;
    }

    if (level_index < 0 || level_index >= MAX_LEVELS) {
        printf("Error: Invalid level_index was passed to is_level_unlocked()\n");
        return 0;
    }

    return level_states[level_index] ? 1 : 0;
}

/**
 * @brief Marks a level as unlocked and persists all progress to disk.
 *
 * Called after the player successfully solves a case, to unlock the next chapter.
 *
 * Implementation Details — PackedProgressRecord:
 *   Uses a #pragma pack(push, 1) struct to serialize each level's state:
 *     - level_index: unsigned char (1 byte)
 *     - is_unlocked: unsigned char (1 byte)
 *   Total: exactly 2 bytes per record with zero compiler padding.
 *
 * The records are written as human-readable integers ("%d %d\n") to the text
 * database rather than as raw binary, keeping the file inspectable in any editor.
 *
 * @param level_index 0-based index of the level to unlock.
 */
void set_level_unlocked(int level_index) {
    if (level_index < 0 || level_index >= MAX_LEVELS) {
        printf("Error: Invalid level_index was passed to set_level_unlocked()\n");
        return;
    }

    level_states[level_index] = true;
    level_states[0] = true; /* Level 1 must always remain unlocked */

    FILE *up_file = fopen("data/progress.thcdb", "w");
    if (!up_file) {
        printf("Error: Could not open progress file for writing\n");
        return;
    }

    /* Write versioned database signature */
    fprintf(up_file, "THCDB_V1\n");

    /*
     * Build and write one PackedProgressRecord per level.
     * With 1-byte packing (#pragma pack(push, 1)), this struct has an
     * exact 2-byte memory footprint — no alignment padding bytes.
     */
    PackedProgressRecord records[MAX_LEVELS];
    for (int i = 0; i < MAX_LEVELS; i++) {
        records[i].level_index = (unsigned char)i;
        records[i].is_unlocked = (unsigned char)((i == 0) ? 1 : (level_states[i] ? 1 : 0));
        fprintf(up_file, "%d %d\n", records[i].level_index, records[i].is_unlocked);
    }

    fclose(up_file);
}

/**
 * @brief Resets all player progress so that only Level 1 is unlocked.
 *
 * Sets all level_states[] to false, then forces level_states[0] = true.
 * Overwrites "data/progress.thcdb" with fresh default data.
 *
 * Used for in-game "New Game" or debug reset functionality.
 */
void reset_progress(void) {
    /* Reset all levels to locked state */
    for (int i = 0; i < MAX_LEVELS; i++) {
        level_states[i] = false;
    }
    level_states[0] = true; /* Level 1 is always the starting point */

    FILE *up_file = fopen("data/progress.thcdb", "w");
    if (!up_file) {
        printf("Error: Could not open progress file for writing\n");
        return;
    }

    /* Write versioned database signature */
    fprintf(up_file, "THCDB_V1\n");

    /*
     * Serialize the reset state using PackedProgressRecord structs.
     * Level 0 will have is_unlocked = 1; all others will be 0.
     */
    PackedProgressRecord records[MAX_LEVELS];
    for (int i = 0; i < MAX_LEVELS; i++) {
        records[i].level_index = (unsigned char)i;
        records[i].is_unlocked = (unsigned char)((i == 0) ? 1 : (level_states[i] ? 1 : 0));
        fprintf(up_file, "%d %d\n", records[i].level_index, records[i].is_unlocked);
    }

    fclose(up_file);
    printf("Progress reset successfully. Only Level 1 is unlocked.\n");
}
