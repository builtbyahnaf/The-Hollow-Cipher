/**
 * =============================================================================
 * PROJECT: The Hollow Cipher (THC)
 * FILE: gameplay/evidence.c
 * PURPOSE: Interactive Evidence Investigation Browser
 * 
 * DESCRIPTION:
 *   This module provides the player with an interactive, keyboard-navigable
 *   menu to inspect physical evidence clues uncovered in the active case.
 * 
 *   Key Capabilities:
 *     - Loads and parses clues from "data/level_<level>/evidence.txt".
 *     - Implements real-time cursor navigation using Windows console key codes
 *       (UP / DOWN arrow keys with 2-byte prefix handling).
 *     - Visual selection highlighting using ANSI background and foreground colors.
 *     - Detail view for individual evidence descriptions.
 *     - Memory lifecycle management: frees dynamically allocated clue structures
 *       upon exiting the panel via free_evidence().
 * =============================================================================
 */

#include <stdio.h>
#include <conio.h>
#include <windows.h>

#include "evidence.h"
#include "evidence_parser.h"
#include "game_main.h"
#include "color_palette.h"

/* Forward declarations of internal subroutines */
void handle_ev_input(int count);
void show_evidence(int index);

/* Pointer to dynamically allocated array of parsed evidence items */
static Evidence *ev_list;

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
    ENTER = 13        /**< Enter / Carriage Return: inspect selected item */
};

/**
 * @brief Initializes the evidence panel for the specified level.
 *
 * Execution Steps:
 *   1. Formats relative path: "data/level_<level>/evidence.txt".
 *   2. Opens file and verifies validity to prevent segmentation faults.
 *   3. Calls parse_evidence() from the parser module to generate evidence structures.
 *   4. Launches the interactive menu loop with handle_ev_input().
 *
 * @param _level Active level number (1 through 5).
 */
void evidence_menu_init(int _level) {
    level = _level;
    char ev_path[86];
    snprintf(ev_path, sizeof(ev_path), "data/level_%d/evidence.txt", level);

    /* Open file with error check */
    FILE *fp = fopen(ev_path, "r");
    if (!fp) {
        printf("Error: Could not open evidence file for level %d\n", level);
        return;
    }

    /* Parse file into dynamically allocated array */
    ev_list = parse_evidence(fp);

    /* Hand off to interactive cursor loop */
    handle_ev_input(evidence_count());
}

/**
 * @brief Handles real-time keyboard navigation and rendering of the evidence list.
 *
 * Features:
 *   - Renders evidence items line-by-line.
 *   - Selected item is highlighted in high-contrast Acid Lime background with Crimson text.
 *   - Non-selected items use standard Leather background with Gold text.
 *   - Non-blocking _getch() captures keypresses:
 *       - ESCAPE: Cleans up memory via free_evidence() and returns to level menu.
 *       - UP_ARROW / DOWN_ARROW: Moves cursor with wrap-around boundaries.
 *       - ENTER: Displays detailed evidence dossier via show_evidence().
 *
 * @param count Total number of evidence clues available in this level.
 */
void handle_ev_input(int count) {
    int selected = 0;
    int key;

    /* Guard against empty or null evidence arrays */
    if (ev_list == NULL || count <= 0) {
        return;
    }

    while (1) {
        /* Clear screen and apply default palette */
        system("cls");
        printf(BG_LEATHER GOLD);

        /* Print Evidence Panel header */
        printf("-----------------------------------------------------------------\n");
        printf("|                 E V I D E N C E   P A N E L                   |\n");
        printf("-----------------------------------------------------------------\n\n");
        printf("Select evidence:\n\n");

        /* Display each evidence item with cursor highlight */
        for (int i = 0; i < count; i++) {
            if (i == selected) {
                /* Highlighted current selection */
                printf(BG_ACID_LIME CRIMSON " >  %s  " "\n", ev_list[i].name);
            } else {
                /* Inactive list items */
                printf(BG_LEATHER GOLD "    %s " "\n", ev_list[i].name);
            }
            printf(BG_LEATHER GOLD);
            Sleep(50);
        }

        printf("\n" "UP/DOWN: Navigate | ENTER: Select | ESC: Exit" "\n");

        /* Wait for a single keyboard input */
        key = _getch();

        /* ESC exits back to level menu */
        if (key == ESCAPE) {
            system("cls");
            free_evidence(); /* Deallocate dynamically parsed items */
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
                /* Wrap around to bottom if moving past top item */
                if (selected < 0) {
                    selected = count - 1;
                }
            }
            else if (key == DOWN_ARROW) {
                selected++;
                /* Wrap around to top if moving past bottom item */
                if (selected >= count) {
                    selected = 0;
                }
            }
            continue;
        }
        else if (key == ENTER) {
            /* Open full description of the highlighted evidence item */
            show_evidence(selected);
        }
    }
}

/**
 * @brief Displays the detailed description of a selected evidence item.
 *
 * Clears the screen, prints the evidence name and full investigative notes,
 * and waits until the user presses ESC to return to the evidence list.
 *
 * @param index Array index of the chosen evidence item.
 */
void show_evidence(int index) {
    system("cls");

    printf("|             EVIDENCE: %s               |\n\n", ev_list[index].name);
    printf("Description:\n\n");
    printf("%s", ev_list[index].description);

    printf("\n\nPress ESC to go back");
    int key;

    /* Modal loop: Hold display until player presses ESC */
    while (1) {
        key = _getch();

        if (key == ESCAPE) {
            system("cls");
            return; /* Returns cleanly back to handle_ev_input() */
        }
    }
}
