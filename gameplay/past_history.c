/**
 * =============================================================================
 * PROJECT: The Hollow Cipher (THC)
 * FILE: gameplay/past_history.c
 * PURPOSE: Interactive Suspect Background / Past History Dossier Browser
 * 
 * DESCRIPTION:
 *   This module provides the player with an interactive keyboard-navigable
 *   menu to review the background histories and personal dossiers of persons
 *   of interest in the active case.
 * 
 *   Key Capabilities:
 *     - Loads and parses history entries from "data/level_<level>/past_history.txt".
 *     - Implements real-time cursor navigation using Windows console key codes
 *       (UP / DOWN arrow keys with 2-byte prefix handling).
 *     - Visual selection highlighting using ANSI theme colors.
 *     - Modal view showing the full history/backstory of a selected person.
 *     - Memory lifecycle management: frees dynamically allocated history data
 *       via free_past_history() upon exit.
 * 
 * NOTE:
 *   The architecture of this file is intentionally parallel to evidence.c and
 *   interrogation.c. All three modules share the same keyboard navigation
 *   pattern (KEYS enum, prefix detection, wrap-around cursor), differing
 *   only in the data they parse and the field they display.
 * =============================================================================
 */

#include <stdio.h>
#include <conio.h>
#include <windows.h>

#include "past_history.h"
#include "past_history_parser.h"
#include "game_main.h"
#include "color_palette.h"

/* Forward declarations of internal subroutines */
void handle_ph_input(int count);
void show_history(int index);

/* Pointer to dynamically allocated array of past history records */
static PastHistory *ph_list;

/* Current chapter/level index */
static int level;

/**
 * @enum KEYS
 * @brief ASCII and extended virtual key codes for Windows console interaction.
 *
 * In the Windows console, special keys like arrow keys generate a two-byte
 * sequence: a prefix byte (0x00 or 0xE0 / 224) followed by the actual key code.
 */
enum KEYS {
    PREFIX_A = 0,     /**< First prefix byte for extended keys */
    PREFIX_B = 224,   /**< Second prefix byte for extended keys */
    ESCAPE = 27,      /**< Escape key: exits back to level menu */
    UP_ARROW = 72,    /**< Up arrow key code */
    DOWN_ARROW = 80,  /**< Down arrow key code */
    ENTER = 13        /**< Enter / Carriage Return: view selected history record */
};

/**
 * @brief Initializes the Past History panel for the specified level.
 *
 * Execution Steps:
 *   1. Formats relative path: "data/level_<level>/past_history.txt".
 *   2. Opens file and verifies validity to prevent segmentation faults.
 *   3. Calls parse_past_history() to parse records into heap memory.
 *   4. Launches the interactive cursor loop with handle_ph_input().
 *
 * @param _level Active level number (1 through 5).
 */
void past_history_menu_init(int _level) {
    level = _level;
    char ph_path[86];
    snprintf(ph_path, sizeof(ph_path), "data/level_%d/past_history.txt", level);

    /* Open file with error check to prevent crash on missing data */
    FILE *fp = fopen(ph_path, "r");
    if (!fp) {
        printf("Error: Could not open past history file for level %d\n", level);
        return;
    }

    /* Parse file into dynamically allocated PastHistory struct array */
    ph_list = parse_past_history(fp);

    /* Hand off to interactive cursor navigation loop */
    handle_ph_input(past_history_count());
}

/**
 * @brief Handles real-time keyboard navigation and rendering of the history list.
 *
 * Features:
 *   - Displays each person's name from the history list.
 *   - Selected entry is highlighted in Acid Lime background with Crimson text.
 *   - Non-selected entries use Leather background with Gold text.
 *   - A 50ms Sleep() between renders gives a smooth terminal drawing effect.
 *   - Non-blocking _getch() captures keypresses:
 *       - ESCAPE: Cleans up memory via free_past_history() and reloads the
 *         level menu via load_level() without suspense animation.
 *       - UP_ARROW / DOWN_ARROW: Moves cursor with wrap-around behaviour.
 *       - ENTER: Opens full dossier for highlighted person via show_history().
 *
 * @param count Total number of history records available in this level.
 */
void handle_ph_input(int count) {
    int selected = 0;
    int key;

    /* Guard against empty or null history arrays */
    if (ph_list == NULL || count <= 0) {
        return;
    }

    while (1) {
        /* Clear screen and apply default theme palette */
        system("cls");
        printf(BG_LEATHER GOLD);

        /* Print Past History Panel header */
        printf("-----------------------------------------------------------------\n");
        printf("|                P A S T   H I S T O R Y   P A N E L              |\n");
        printf("-----------------------------------------------------------------\n\n");
        printf("Select a person:\n\n");

        /* Render each history record name with cursor highlight */
        for (int i = 0; i < count; i++) {
            if (i == selected) {
                /* Highlighted current selection */
                printf(BG_ACID_LIME CRIMSON " >  %s  " "\n", ph_list[i].name);
            } else {
                /* Inactive list items */
                printf(BG_LEATHER GOLD "    %s " "\n", ph_list[i].name);
            }
            printf(BG_LEATHER GOLD);
            Sleep(50); /* Small delay for smooth incremental console rendering */
        }

        printf("\n" "UP/DOWN: Navigate | ENTER: Select | ESC: Exit" "\n");

        /* Wait for a single keyboard input */
        key = _getch();

        /* ESC exits back to level menu, freeing history memory first */
        if (key == ESCAPE) {
            system("cls");
            free_past_history(); /* Deallocate dynamically parsed history entries */
            load_level(level, false);
            return;
        }

        /*
         * Extended key handling (Arrow keys):
         * Windows console produces an escape prefix byte (0x00 or 0xE0 / 224)
         * followed by the specific key code. We consume both bytes together.
         */
        if (key == PREFIX_A || key == PREFIX_B) {
            key = _getch();

            if (key == UP_ARROW) {
                selected--;
                /* Wrap around to last entry when moving above the first */
                if (selected < 0) {
                    selected = count - 1;
                }
            }
            else if (key == DOWN_ARROW) {
                selected++;
                /* Wrap around to first entry when moving below the last */
                if (selected >= count) {
                    selected = 0;
                }
            }
            continue;
        }
        else if (key == ENTER) {
            /* Open the full historical background of the highlighted person */
            show_history(selected);
        }
    }
}

/**
 * @brief Displays the full background history of a selected person.
 *
 * Clears the screen, prints the person's name and their complete recorded
 * history/backstory text, then holds in a modal loop until the player
 * presses ESC to return to the history list.
 *
 * @param index Array index of the chosen history record.
 */
void show_history(int index) {
    system("cls");

    printf("|             PAST HISTORY: %s               |\n\n", ph_list[index].name);
    printf("History:\n\n");
    printf("%s", ph_list[index].history);

    printf("\n\nPress ESC to go back");
    int key;

    /* Modal loop: Hold display until player presses ESC */
    while (1) {
        key = _getch();

        if (key == ESCAPE) {
            system("cls");
            return; /* Returns cleanly back to handle_ph_input() */
        }
    }
}
