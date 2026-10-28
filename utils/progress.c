#include <stdio.h>
#include <stdbool.h>
#include <string.h>

#include "progress.h"
#include "parser_utils.h"

/*
 * Game progress state array.
 * Level 1 (index 0) is hardcoded to always be unlocked (true).
 */
static bool level_states[MAX_LEVELS] = {true, false};

bool is_signature_valid(FILE *pfile) {
    char signature[10];
    if (fscanf(pfile, "%9s", signature) != 1) {
        return false;
    }
    return strcmp(signature, "THCDB_V1") == 0;
}

void load_progress(void) {
    /* Level 1 must always be unlocked by default (hardcoded) */
    level_states[0] = true;

    FILE *pfile = fopen("data/progress.thcdb", "r");
    if (!pfile) {
        printf("Error reading file. The progress data is either missing or corrupted.\n");
        return;
    }

    if (!is_signature_valid(pfile)) {
        printf("Invalid signature detected\n");
        fclose(pfile);
        return;
    }

    int level_index, state;
    while (fscanf(pfile, "%d %d", &level_index, &state) == 2) {
        if (level_index == 0) {
            level_states[0] = true;
        } else if (level_index > 0 && level_index < MAX_LEVELS && (state == 0 || state == 1)) {
            level_states[level_index] = (state != 0);
        } else if (level_index >= MAX_LEVELS && (state == 0 || state == 1)) {
            /* Gracefully skip extra levels beyond current MAX_LEVELS */
            continue;
        } else {
            printf("The progress data is corrupted.\n");
            break;
        }
    }

    /* Enforce Level 1 unlocked status even if save file had other values */
    level_states[0] = true;
    fclose(pfile);
}

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

/*
 * Updates level progress and rewrites data/progress.thcdb using PackedProgressRecord.
 * Demonstrates:
 *   PACKING: Serializes using PackedProgressRecord structs with 0 padding bytes.
 */
void set_level_unlocked(int level_index) {
    if (level_index < 0 || level_index >= MAX_LEVELS) {
        printf("Error: Invalid level_index was passed to set_level_unlocked()\n");
        return;
    }

    level_states[level_index] = true;
    level_states[0] = true; /* Level 1 must remain unlocked */

    FILE *up_file = fopen("data/progress.thcdb", "w");
    if (!up_file) {
        printf("Error: Could not open progress file for writing\n");
        return;
    }

    /* Write database header signature */
    fprintf(up_file, "THCDB_V1\n");

    /*
     * Build an array of packed records.
     * With 1-byte packing, this struct has an exact 2-byte footprint.
     */
    PackedProgressRecord records[MAX_LEVELS];
    for (int i = 0; i < MAX_LEVELS; i++) {
        records[i].level_index = (unsigned char)i;
        records[i].is_unlocked = (unsigned char)((i == 0) ? 1 : (level_states[i] ? 1 : 0));
        fprintf(up_file, "%d %d\n", records[i].level_index, records[i].is_unlocked);
    }

    fclose(up_file);
}

/*
 * Resets progress to default state where only Level 1 is unlocked.
 */
void reset_progress(void) {
    for (int i = 0; i < MAX_LEVELS; i++) {
        level_states[i] = false;
    }
    level_states[0] = true;

    FILE *up_file = fopen("data/progress.thcdb", "w");
    if (!up_file) {
        printf("Error: Could not open progress file for writing\n");
        return;
    }

    fprintf(up_file, "THCDB_V1\n");

    PackedProgressRecord records[MAX_LEVELS];
    for (int i = 0; i < MAX_LEVELS; i++) {
        records[i].level_index = (unsigned char)i;
        records[i].is_unlocked = (unsigned char)((i == 0) ? 1 : (level_states[i] ? 1 : 0));
        fprintf(up_file, "%d %d\n", records[i].level_index, records[i].is_unlocked);
    }

    fclose(up_file);
    printf("Progress reset successfully. Only Level 1 is unlocked.\n");
}
