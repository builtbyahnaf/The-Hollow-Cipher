/**
 * =============================================================================
 * PROJECT: The Hollow Cipher (THC)
 * FILE: admin/cmd_edit.c
 * PURPOSE: Interactive In-Game Level Content Editor and Audit Logger
 * 
 * DESCRIPTION:
 *   This module provides administrative functionality to inspect and modify
 *   story components and puzzle data for all 5 game levels directly from the
 *   admin console.
 *
 *   Key Capabilities:
 *     1. Level and Component Selection:
 *        Presents hierarchical CLI menus allowing the administrator to select
 *        a specific level (1-5) and component (Evidence, Interrogation,
 *        Past History, Solution, or Statement).
 *     2. External Editor Launch:
 *        Spawns Microsoft Notepad (notepad.exe) on the target component file,
 *        allowing direct text editing while keeping the game paused.
 *     3. Audit Logging (Reverse-Chronological Prepend):
 *        Every edit event is timestamped and prepended to "data/edit_log.thcdb"
 *        so the latest modification is always at the top of the log.
 *     4. Bit-Field Struct and Packing Demonstration:
 *        Demonstrates memory conservation using packed C bit-field structures
 *        (#pragma pack) to compress level and menu selections into 1 byte.
 * =============================================================================
 */

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

/**
 * @enum LEVELS
 * @brief Identifiers for each game chapter/level.
 */
enum LEVELS {
    LEVEL_1 = 1,
    LEVEL_2 = 2,
    LEVEL_3 = 3,
    LEVEL_4 = 4,
    LEVEL_5 = 5
};

/**
 * @enum OPTIONS
 * @brief Identifiers for each editable game file component within a level.
 */
enum OPTIONS {
    EVIDENCE = 1,       /**< Evidence clues file (evidence.txt) */
    INTERROGATION = 2,  /**< Suspect interrogation dialogues (interrogation.txt) */
    PAST_HISTORY = 3,   /**< Background history/lore (past_history.txt) */
    SOLUTION = 4,       /**< Level passcodes / solution answers (solution.txt) */
    STATEMENT = 5       /**< Level introduction / case statement (statement.txt) */
};

/*
 * =============================================================================
 * PACKING IMPLEMENTATION (Packed Bit-Field Struct)
 * =============================================================================
 * By default, C compilers align structure members to machine-word or standard
 * byte boundaries (e.g., 2, 4, or 8 bytes) by inserting internal and trailing
 * padding bytes.
 *
 * '#pragma pack(push, 1)' saves the compiler's current packing alignment state
 * and enforces 1-byte alignment, eliminating all alignment padding.
 *
 * The 'EditSelection' struct combines bit-fields with 1-byte packing:
 *   - 'selected_menu': Allocated 5 bits (unsigned range 0 to 31),
 *     which comfortably accommodates component enum IDs 1 through 5.
 *   - 'selected_level': Allocated 3 bits (unsigned range 0 to 7),
 *     which covers level indices 1 through 5.
 * Total memory footprint: 5 + 3 = 8 bits = exactly 1 byte.
 * '#pragma pack(pop)' restores the compiler's previous alignment setting.
 * =============================================================================
 */
#pragma pack(push, 1)
typedef struct {
    unsigned int selected_menu : 5;  /**< Menu/Component selection (5 bits: 0-31) */
    unsigned int selected_level : 3; /**< Level selection (3 bits: 0-7) */
} EditSelection;
#pragma pack(pop)

/**
 * @brief Displays the component selection menu for a chosen level.
 *
 * Prompts the administrator to select which narrative/puzzle component
 * of the level they wish to edit:
 *   - [E] Evidence
 *   - [I] Interrogation
 *   - [H] Past History
 *   - [A] Solution
 *   - [S] Statement
 *   - [X] Return to level selection
 *
 * Validates input, flushes stdin, and launches the editor on choice.
 *
 * @param LEVEL The integer level index (1-5) currently being edited.
 */
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

        /* Safely read single character input */
        if (scanf(" %c", &selection) != 1) {
            int c;
            while ((c = getchar()) != '\n' && c != EOF);
            continue;
        }

        /* Flush remaining characters from the input stream */
        int c;
        while ((c = getchar()) != '\n' && c != EOF);

        /* Normalize character to lowercase */
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
            /* Return to select_level() */
            CLEAR_SCREEN();
            return;

        default:
            printf("INVALID SELECTION\n");
            Sleep(1000);
            break;
        }
    }
}

/**
 * @brief Displays the level selection menu for editing game content.
 *
 * Allows the administrator to choose which chapter/level (1 to 5) to edit,
 * or enter 0 to return to the parent admin control panel.
 *
 * Handles integer input validation and cleans input stream on failure.
 */
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

        /* Validate integer entry */
        if (scanf("%d", &selection) != 1) {
            int c;
            while ((c = getchar()) != '\n' && c != EOF);
            printf("Error: Invalid level\n");
            Sleep(1000);
            continue;
        }

        /* Flush newline and trailing characters from stdin */
        int c;
        while ((c = getchar()) != '\n' && c != EOF);

        /* Range check */
        if (selection < 0 || selection > 5) {
            printf("Error: Invalid level\n");
            Sleep(1000);
            continue;
        }

        /* 0 signals user request to return to the admin dashboard */
        if (selection == 0) {
            CLEAR_SCREEN();
            return;
        }

        /* Proceed to component selection for the chosen level */
        CLEAR_SCREEN();
        component_selection_dialogue(selection);
    }
}

/**
 * @brief Maps a component enum ID to its corresponding text filename.
 *
 * @param component The component ID (OPTIONS enum: EVIDENCE, INTERROGATION, etc.).
 * @return const char* Pointer to the static filename string, or NULL if invalid.
 */
const char* get_component_filename(int component) {
    switch (component) {
        case EVIDENCE:      return "evidence.txt";
        case INTERROGATION: return "interrogation.txt";
        case PAST_HISTORY:  return "past_history.txt";
        case SOLUTION:      return "solution.txt";
        case STATEMENT:     return "statement.txt";
        default:            return NULL;
    }
}

/**
 * @brief Records an edit event into the audit log in reverse chronological order.
 *
 * Log Prepending Workflow:
 *   1. Resolves the edited file path based on LEVEL and COMPONENT.
 *   2. Generates an RFC-readable timestamp string using time() and strftime().
 *   3. Formats the new log line: "Edited <filepath> on (<timestamp>)\n".
 *   4. Reads any existing content from "data/edit_log.thcdb" into dynamic memory:
 *        - fseek(SEEK_END) + ftell() determine the current file size.
 *        - malloc() allocates an exact temporary buffer.
 *        - fread() copies the existing log content into memory.
 *   5. Re-opens "data/edit_log.thcdb" in "w" (overwrite/create) mode:
 *        - Writes the new log entry first (prepending).
 *        - Writes the previous log entries immediately following.
 *   6. Safely frees the allocated buffer via SAFE_FREE() and closes the file handle.
 *
 * @param LEVEL The level index modified (1-5).
 * @param COMPONENT The component enum identifier modified.
 */
void write_log(int LEVEL, int COMPONENT) {
    const char* filename = get_component_filename(COMPONENT);
    if (!filename) {
        return;
    }

    /* Build target file path */
    char filepath[256];
    snprintf(filepath, sizeof(filepath), "data/level_%d/%s", LEVEL, filename);

    /* Generate current timestamp */
    time_t now = time(NULL);
    struct tm* tm_info = localtime(&now);

    char time_str[128];
    strftime(time_str, sizeof(time_str), "%d, %B, %Y, %A, %I:%M:%S %p", tm_info);

    /* Format the log entry */
    char new_log[512];
    snprintf(new_log, sizeof(new_log), "Edited %s on (%s)\n", filepath, time_str);

    /*
     * Step 1: Read existing log entries into memory so we can prepend the new entry.
     */
    FILE* log_file = fopen("data/edit_log.thcdb", "r");
    char* existing_logs = NULL;
    size_t existing_size = 0;

    if (log_file) {
        /* Determine file size */
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

    /*
     * Step 2: Open log file in write ("w") mode to write the new entry followed
     * by previously saved entries.
     */
    log_file = fopen("data/edit_log.thcdb", "w");
    if (!log_file) {
        SAFE_FREE(existing_logs);
        return;
    }

    /* Write newest log line first */
    fputs(new_log, log_file);

    /* Append previous log records if any existed */
    if (existing_logs) {
        fputs(existing_logs, log_file);
        SAFE_FREE(existing_logs);
    }

    fclose(log_file);
}

/**
 * @brief Launches an external text editor (Notepad) for the specified level component.
 *
 * Execution Steps:
 *   1. Validates level range (1-5).
 *   2. Stores the selection in the packed bit-field struct 'EditSelection'
 *      to ensure compact state management.
 *   3. Resolves the target component's filename and constructs its file path:
 *      "data/level_<level>/<component>.txt".
 *   4. Verifies that the target file exists on disk before spawning editor.
 *   5. Executes Microsoft Notepad via Windows command:
 *      notepad.exe "<filepath>"
 *      (Note: system() call is synchronous; execution blocks until the user
 *      closes the Notepad window).
 *   6. Records the modification in "data/edit_log.thcdb" via write_log().
 *   7. Pauses with _getch() to display confirmation before returning to menu.
 *
 * @param LEVEL The game level index (1-5).
 * @param COMPONENT The component enum identifier to edit.
 */
void open_editor(int LEVEL, int COMPONENT) {
    if (LEVEL < 1 || LEVEL > 5) {
        printf("Error: Invalid level\n");
        return;
    }

    /*
     * Instantiate and populate the packed bit-field configuration record.
     * Compresses selection into 1 byte (3 bits for level, 5 bits for menu).
     */
    EditSelection config;
    config.selected_level = (unsigned int)LEVEL;
    config.selected_menu = (unsigned int)COMPONENT;

    /* Resolve filename from component enum */
    const char* filename = get_component_filename((int)config.selected_menu);
    if (!filename) {
        printf("Error: Invalid component\n");
        return;
    }

    /* Construct full relative path to target data file */
    char filepath[256];
    snprintf(filepath, sizeof(filepath), "data/level_%d/%s", (int)config.selected_level, filename);

    /* Verify target file exists and is accessible before launching editor */
    FILE* test = fopen(filepath, "r");
    if (!test) {
        printf("Error: File not found: %s\n", filepath);
        return;
    }
    fclose(test);

    /* Assemble system command to launch Notepad with the file */
    char command[512];
    snprintf(command, sizeof(command), "notepad.exe \"%s\"", filepath);

    printf("Opening: %s\n", filepath);

    /*
     * Launch Notepad synchronously.
     * The game process blocks here until the admin closes Notepad.
     */
    system(command);

    /* Record timestamped edit action in audit log */
    write_log((int)config.selected_level, (int)config.selected_menu);

    printf("File editing complete. Changes saved.\n");
    printf("\nPress any key to continue...");
    _getch();
    CLEAR_SCREEN();
}
