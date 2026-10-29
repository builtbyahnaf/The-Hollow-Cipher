/**
 * =============================================================================
 * PROJECT: The Hollow Cipher (THC)
 * FILE: gameplay/level_select.c
 * PURPOSE: Chapter and Level Selection Menu Interface
 * 
 * DESCRIPTION:
 *   This module renders the chapter selection screen where players choose
 *   which case file / mystery level to investigate.
 * 
 *   Key Capabilities:
 *     - Dynamically checks level completion state from save storage
 *       ("data/progress.thcdb") using the progress module.
 *     - Displays dynamic "[LOCKED]" indicators next to chapters that have not
 *       yet been unlocked.
 *     - Prevents access to locked levels, ensuring players solve cases in
 *       sequential order.
 *     - Level 1 is always unlocked by default.
 *     - Handles user navigation, input sanitization, and graceful return
 *       to the main game initialization menu.
 * =============================================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
#include <conio.h>

#include "progress.h"
#include "level_select.h"
#include "main.h"
#include "game_main.h"

/**
 * @brief Displays the interactive level selection menu and handles player choice.
 *
 * Execution Loop:
 *   1. Reloads player save data via load_progress() on every iteration so
 *      newly unlocked levels appear unlocked immediately after solving a case.
 *   2. Displays the ASCII banner and the 5 game chapters:
 *        - Level 1: The Silent Patient (always accessible)
 *        - Level 2: The Missing Notebook (unlocked after Level 1)
 *        - Level 3: The Hidden Destination (unlocked after Level 2)
 *        - Level 4: The Dark Discovery (unlocked after Level 3)
 *        - Level 5: The Hollow Cipher (unlocked after Level 4)
 *   3. Prompts user for a numeric selection (0 to 5):
 *        - 0: Returns to main game menu (game_init).
 *        - 1-5: Validates unlock status and launches the level via load_level().
 *   4. Flushes the standard input buffer to discard leftover newline characters
 *      or invalid input tokens.
 */
void show_level_selection(void) {
    int selected_level;

    while (1) {
        /* Refresh progress state from progress.thcdb */
        load_progress();

        /* Clear console screen and print menu header */
        system("cls");
        printf("-----------------------------------------------------------------\n");
        printf("|               T H E   H O L L O W    C I P H E R               |\n");
        printf("-----------------------------------------------------------------\n\n");

        printf("GO BACK [0]\n\n");

        /* Level 1 is the starting chapter; always unlocked */
        printf("LEVEL 1: THE SILENT PATIENT\n");

        /* Display chapters 2 through 5 with dynamic [LOCKED] status */
        printf("LEVEL 2: THE MISSING NOTEBOOK");
        if (!is_level_unlocked(1)) printf(" [LOCKED]");
        printf("\n");

        printf("LEVEL 3: THE HIDDEN DESTINATION");
        if (!is_level_unlocked(2)) printf(" [LOCKED]");
        printf("\n");

        printf("LEVEL 4: THE DARK DISCOVERY");
        if (!is_level_unlocked(3)) printf(" [LOCKED]");
        printf("\n");

        printf("LEVEL 5: THE HOLLOW CIPHER");
        if (!is_level_unlocked(4)) printf(" [LOCKED]");
        printf("\n\n");

        printf("ENTER SELECTION (0-5): ");

        /*
         * Validate numeric input. If user enters non-numeric input (e.g. letters),
         * scanf returns != 1. Flush stdin up to newline/EOF and prompt again.
         */
        if (scanf("%d", &selected_level) != 1) {
            int c;
            while ((c = getchar()) != '\n' && c != EOF);
            printf("Invalid input. Please enter a number between 0-5.\n");
            Sleep(1000);
            continue;
        }

        /* Flush remaining newline character left by scanf in stdin */
        int c;
        while ((c = getchar()) != '\n' && c != EOF);

        /* Validate range [0, 5] */
        if (selected_level < 0 || selected_level > 5) {
            printf("Enter between 0-5. Please try again.\n");
            Sleep(1000);
            continue;
        } else if (selected_level == 0) {
            /* 0 indicates user wishes to exit to top-level menu */
            system("cls");
            game_init();
            return; /* Returns cleanly to game_init() */
        } else if (selected_level != 1 && !is_level_unlocked(selected_level - 1)) {
            /* Block access if previous chapter has not been solved */
            printf("This level is locked. Complete previous levels to unlock.\n");
            Sleep(1500);
            continue;
        } else {
            /*
             * Launch the selected level with suspense introduction animation.
             * When the user finishes or exits the level, control returns here,
             * looping back to the level selection menu.
             */
            load_level(selected_level, true);
        }
    }
}
