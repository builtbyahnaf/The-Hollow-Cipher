/**
 * =============================================================================
 * PROJECT: The Hollow Cipher (THC)
 * FILE: gameplay/interrogation.c
 * PURPOSE: Interactive Suspect Interrogation Dialogue Browser
 * 
 * DESCRIPTION:
 *   This module provides the player with an interactive menu to interrogate
 *   suspects and review their testimonies for the active investigation.
 * 
 *   Key Capabilities:
 *     - Loads and parses suspect statements from "data/level_<level>/interrogation.txt".
 *     - Implements real-time cursor navigation using Windows console key codes
 *       (UP / DOWN arrow keys with 2-byte prefix handling).
 *     - Visual selection highlighting using ANSI theme colors.
 *     - Modal view displaying the full suspect dialogue and alibi.
 *     - Memory lifecycle management: frees dynamically allocated statements
 *       via free_statements() upon exit.
 * =============================================================================
 */

#include <stdio.h>
#include <conio.h>
#include <windows.h>

#include "interrogation.h"
#include "interrogation_parser.h"
#include "past_history.h"
#include "evidence_vault.h"
#include "game_main.h"
#include "color_palette.h"

/* Forward declarations of internal subroutines */
void handle_input(int count);
void show_statement(int index);

/* Pointer to dynamically allocated array of suspect statements */
static SuspectStatements *sus_sts;

/* Current chapter/level index */
static int level;

/**
 * @enum KEYS
 * @brief ASCII and extended virtual key codes for Windows console interaction.
 *
 * Special keys like arrow keys generate a two-byte sequence: a prefix
 * byte (0x00 or 0xE0 / 224) followed by the specific directional key code.
 */
enum KEYS {
    PREFIX_A = 0,     /**< First prefix byte for extended keys */
    PREFIX_B = 224,   /**< Second prefix byte for extended keys */
    ESCAPE = 27,      /**< Escape key: exits back to level menu */
    UP_ARROW = 72,    /**< Up arrow key code */
    DOWN_ARROW = 80,  /**< Down arrow key code */
    ENTER = 13        /**< Enter / Carriage Return: inspect selected testimony */
};

/**
 * @brief Initializes the interrogation menu for the specified level.
 *
 * Execution Steps:
 *   1. Formats relative path: "data/level_<level>/interrogation.txt".
 *   2. Opens file and verifies validity to prevent crash/segmentation fault.
 *   3. Calls parse_interrogation() to parse records into memory.
 *   4. Launches the interactive cursor loop with handle_input().
 *
 * @param _level Active level number (1 through 5).
 */
void interrogation_menu_init(int _level) {
    level = _level;
    char interr_path[86];
    snprintf(interr_path, sizeof(interr_path), "data/level_%d/interrogation.txt", level);

    /* Open file with validity check to prevent crash */
    FILE *fp = fopen(interr_path, "r");
    if (!fp) {
        printf("Error: Could not open interrogation file for level %d\n", level);
        return;
    }

    /* Parse file into dynamically allocated suspect structures */
    sus_sts = parse_interrogation(fp);

    /* Hand off to interactive cursor navigation */
    handle_input(statements_count());
}

/**
 * @brief Handles real-time keyboard navigation and rendering of the suspect list.
 *
 * Features:
 *   - Displays suspect names line-by-line.
 *   - Highlighted suspect is rendered in contrasting Acid Lime background with Crimson text.
 *   - Non-selected suspects use Leather background with Gold text.
 *   - Non-blocking _getch() captures keypresses:
 *       - ESCAPE: Cleans up memory via free_statements() and returns to level menu.
 *       - UP_ARROW / DOWN_ARROW: Moves cursor with wrap-around boundaries.
 *       - ENTER: Displays detailed statement dossier via show_statement().
 *
 * @param count Total number of suspects available for questioning in this level.
 */
void handle_input(int count) {
    int selected = 0;
    int key;

    /* Guard against empty or null statement arrays */
    if (sus_sts == NULL || count <= 0) {
        return;
    }

    while (1) {
        /* Clear screen and apply default palette */
        system("cls");
        printf(BG_LEATHER GOLD);

        /* Print Interrogation Panel header */
        printf("-----------------------------------------------------------------\n");
        printf("|             I N T E R R O G A T I O N   P A N E L               |\n");
        printf("-----------------------------------------------------------------\n\n");
        printf("Select a suspect:\n\n");

        /* Display each suspect name with cursor highlight */
        for (int i = 0; i < count; i++) {
            if (i == selected) {
                /* Highlighted current selection */
                printf(BG_ACID_LIME CRIMSON " >  %s  " "\n", sus_sts[i].name);
            } else {
                /* Inactive list items */
                printf(BG_LEATHER GOLD "    %s " "\n", sus_sts[i].name);
            }
            printf(BG_LEATHER GOLD);
            Sleep(50);
        }

        printf("\n" "UP/DOWN: Navigate | ENTER: Select | ESC: Exit" "\n");

        /* Wait for a single keyboard input */
        key = _getch();

        /* Escape exits back to level menu */
        if (key == ESCAPE) {
            system("cls");
            free_statements(); /* Deallocate dynamically parsed statements */
            load_level(level, false);
            return;
        }

        /*
         * Extended key handling (Arrow keys):
         * Windows console produces an escape prefix byte followed by the key code.
         */
        if (key == PREFIX_A || key == PREFIX_B) {
            key = _getch();

            if (key == UP_ARROW) {
                selected--;
                /* Wrap around to bottom if moving past top suspect */
                if (selected < 0) {
                    selected = count - 1;
                }
            }
            else if (key == DOWN_ARROW) {
                selected++;
                /* Wrap around to top if moving past bottom suspect */
                if (selected >= count) {
                    selected = 0;
                }
            }
            continue;
        }
        else if (key == ENTER) {
            /* Open full interrogation statement of highlighted suspect */
            show_statement(selected);
        }
    }
}

/**
 * @brief Displays the full interrogation statement of the selected suspect.
 *
 * Clears the screen, prints the suspect's name and their complete recorded
 * statement, and holds until the player presses ESC to return to the suspect list.
 *
 * @param index Array index of the chosen suspect.
 */
void show_statement(int index) {
    system("cls");

    printf("|             INTERROGATION: %s               |\n\n", sus_sts[index].name);
    printf("Statement:\n\n");
    printf("%s", sus_sts[index].statement);

    printf("\n\nPress ESC to go back");
    int key;

    /* Modal loop: Hold display until player presses ESC */
    while (1) {
        key = _getch();

        /* Escape exits back to suspect list */
        if (key == ESCAPE) {
            system("cls");
            return; /* Returns cleanly back to handle_input() */
        }
    }
}
