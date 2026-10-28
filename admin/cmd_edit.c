#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <time.h>
#include <windows.h>
#include <conio.h>

#include "cmd_edit.h"
#include "admin_panel.h"
#include "parser_utils.h"

enum LEVELS {
    LEVEL_1 = 1,
    LEVEL_2 = 2,
    LEVEL_3 = 3,
    LEVEL_4 = 4,
    LEVEL_5 = 5
};

enum OPTIONS {
    EVIDENCE = 1,
    INTERROGATION = 2,
    PAST_HISTORY = 3,
    SOLUTION = 4,
    STATEMENT = 5
};

/*
 * =============================================================================
 * PACKING IMPLEMENTATION (Packed Bit-Field Struct)
 * =============================================================================
 * By default, compilers pad structures to 2, 4, or 8 byte boundaries.
 * '#pragma pack(push, 1)' pushes the current packing alignment and sets 1-byte
 * packing, stripping all internal and trailing padding.
 *
 * EditSelection combines bit-fields with packing:
 *   - selected_menu occupies 5 bits (range 0 to 31, sufficient for menu IDs 1-5).
 *   - selected_level occupies 3 bits (range 0 to 7, covering level indices 1-5).
 * Total: 5 + 3 = 8 bits = exactly 1 byte in memory.
 */
#pragma pack(push, 1)
typedef struct {
    unsigned int selected_menu : 5;
    unsigned int selected_level : 3;
} EditSelection;
#pragma pack(pop)

void component_selection_dialogue(int LEVEL) {
    char selection;

    while (1) {
        printf("\n \n");

        printf("               SELECT THE COMPONENT YOU WANT TO EDIT              ");

        printf("\n \n");

        printf("COMPONENT 1: EVIDENCE [E]\n");
        printf("COMPONENT 2: INTERROGATION [I]\n");
        printf("COMPONENT 3: PAST HISTORY [H]\n");
        printf("COMPONENT 4: SOLUTION [A]\n");
        printf("COMPONENT 5: STATEMENT [S]\n\n");
        printf("GO BACK [X]\n");

        printf("ENTER SELECTION: ");
        if (scanf(" %c", &selection) != 1) {
            int c;
            while ((c = getchar()) != '\n' && c != EOF);
            continue;
        }
        int c;
        while ((c = getchar()) != '\n' && c != EOF);

        selection = tolower(selection);

        switch (selection) {
        case 'e':
            CLEAR_SCREEN();
            printf("Evidence file opened in notepad for you to edit:\n");
            open_editor(LEVEL, EVIDENCE);
            break;

        case 'i':
            CLEAR_SCREEN();
            printf("Interrogation file opened in notepad for you to edit:\n");
            open_editor(LEVEL, INTERROGATION);
            break;
        case 'h':
            CLEAR_SCREEN();
            printf("Past History file opened in notepad for you to edit:\n");
            open_editor(LEVEL, PAST_HISTORY);
            break;
        case 'a':
            CLEAR_SCREEN();
            printf("Solution file opened in notepad for you to edit:\n");
            open_editor(LEVEL, SOLUTION);
            break;
        case 's':
            CLEAR_SCREEN();
            printf("Statement file opened in notepad for you to edit:\n");
            open_editor(LEVEL, STATEMENT);
            break;
        case 'x':
            CLEAR_SCREEN();
            return;

        default:
            printf("INVALID SELECTION\n");
            Sleep(1000);
            break;
        }
    }
}

void select_level(void) {
    int selection;

    while (1) {
        system("cls");
        printf("-----------------------------------------------------------------\n");
        printf("|               T H E   H O L L O W    C I P H E R               |\n");
        printf("-----------------------------------------------------------------\n");

        printf("\n \n");

        printf("               SELECT THE LEVEL YOU WANT TO EDIT              ");

        printf("\n \n");

        printf("TO GO BACK, ENTER 0 (ZERO)\n\n");
        printf("LEVEL 1: THE SILENT PATIENT\n");
        printf("LEVEL 2: THE MISSING NOTEBOOK\n");
        printf("LEVEL 3: THE HIDDEN DESTINATION\n");
        printf("LEVEL 4: THE MAN WHO DEVELOPED THE DARK\n");
        printf("LEVEL 5: THE HOLLOW CIPHER\n\n");

        printf("ENTER SELECTION (0-5): ");
        if (scanf("%d", &selection) != 1) {
            int c;
            while ((c = getchar()) != '\n' && c != EOF);
            printf("Error: Invalid level\n");
            Sleep(1000);
            continue;
        }
        int c;
        while ((c = getchar()) != '\n' && c != EOF);

        if (selection < 0 || selection > 5) {
            printf("Error: Invalid level\n");
            Sleep(1000);
            continue;
        }

        if (selection == 0) {
            CLEAR_SCREEN();
            return;
        }

        CLEAR_SCREEN();
        component_selection_dialogue(selection);
    }
}

const char* get_component_filename(int component) {
    switch (component) {
        case EVIDENCE: return "evidence.txt";
        case INTERROGATION: return "interrogation.txt";
        case PAST_HISTORY: return "past_history.txt";
        case SOLUTION: return "solution.txt";
        case STATEMENT: return "statement.txt";
        default: return NULL;
    }
}

void write_log(int LEVEL, int COMPONENT) {
    const char* filename = get_component_filename(COMPONENT);
    if (!filename) {
        return;
    }

    char filepath[256];
    snprintf(filepath, sizeof(filepath), "data/level_%d/%s", LEVEL, filename);

    time_t now = time(NULL);
    struct tm* tm_info = localtime(&now);

    char time_str[128];
    strftime(time_str, sizeof(time_str), "%d, %B, %Y, %A, %I:%M:%S %p", tm_info);

    char new_log[512];
    snprintf(new_log, sizeof(new_log), "Edited %s on (%s)\n", filepath, time_str);

    FILE* log_file = fopen("data/edit_log.thcdb", "r");
    char* existing_logs = NULL;
    size_t existing_size = 0;

    if (log_file) {
        fseek(log_file, 0, SEEK_END);
        existing_size = ftell(log_file);
        rewind(log_file);

        if (existing_size > 0) {
            existing_logs = (char *)malloc(existing_size + 1);
            if (existing_logs) {
                fread(existing_logs, 1, existing_size, log_file);
                existing_logs[existing_size] = '\0';
            }
        }
        fclose(log_file);
    }

    log_file = fopen("data/edit_log.thcdb", "w");
    if (!log_file) {
        SAFE_FREE(existing_logs);
        return;
    }

    fputs(new_log, log_file);
    if (existing_logs) {
        fputs(existing_logs, log_file);
        SAFE_FREE(existing_logs);
    }

    fclose(log_file);
}

void open_editor(int LEVEL, int COMPONENT) {
    if (LEVEL < 1 || LEVEL > 5) {
        printf("Error: Invalid level\n");
        return;
    }

    /* Instantiate packed configuration record */
    EditSelection config;
    config.selected_level = (unsigned int)LEVEL;
    config.selected_menu = (unsigned int)COMPONENT;

    const char* filename = get_component_filename((int)config.selected_menu);
    if (!filename) {
        printf("Error: Invalid component\n");
        return;
    }

    char filepath[256];
    snprintf(filepath, sizeof(filepath), "data/level_%d/%s", (int)config.selected_level, filename);

    FILE* test = fopen(filepath, "r");
    if (!test) {
        printf("Error: File not found: %s\n", filepath);
        return;
    }
    fclose(test);

    char command[512];
    snprintf(command, sizeof(command), "notepad.exe \"%s\"", filepath);

    printf("Opening: %s\n", filepath);
    system(command);

    write_log((int)config.selected_level, (int)config.selected_menu);

    printf("File editing complete. Changes saved.\n");
    printf("\nPress any key to continue...");
    _getch();
    CLEAR_SCREEN();
}
