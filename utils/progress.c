#include <stdio.h>
#include <stdbool.h>
#include <string.h>

#define MAX_LEVELS 2

static bool level_states[MAX_LEVELS] = {false};

bool is_signature_valid(FILE *pfile) {
    char signature[10];
    if (fscanf(pfile, "%9s", signature) != 1) {
        return false;
    }
    return strcmp(signature, "THCDB_V1") == 0;
}

void load_progress() {
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
        if (level_index >= 0 && level_index < MAX_LEVELS && (state == 0 || state == 1)) {
            if (level_index == 0) {
                level_states[0] = true;
            } else {
                level_states[level_index] = (state) ? true : false;
            }
        } else {
            printf("The progress data is corrupted.\n");
            break;
        }
    }
    fclose(pfile);
}

bool is_level_unlocked(int level_index) {
    if (level_index < 0 || level_index > MAX_LEVELS) {
        printf("Error: Invalid level_index was passed to is_level_unlocked()\n");
        return false;
    }
    return level_states[level_index];
}

void set_level_unlocked(int level_index) {
    if (level_index < 0 || level_index > MAX_LEVELS) {
        printf("Error: Invalid level_index was passed to set_level_unlocked()\n");
        return;
    }
    level_states[level_index] = true;

    FILE *up_file = fopen("data/progress.thcdb", "w");
    if (!up_file) {
        printf("Error: Could not open progress file for writing\n");
        return;
    }
    fprintf(up_file, "THCDB_V1\n");
    for (int i = 0; i < MAX_LEVELS; i++) {
        fprintf(up_file, "%d %d\n", i, level_states[i] ? 1 : 0);
    }
    fclose(up_file);
}