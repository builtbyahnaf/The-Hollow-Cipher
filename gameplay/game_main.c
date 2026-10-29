/**
 * =============================================================================
 * PROJECT: The Hollow Cipher (THC)
 * FILE: gameplay/game_main.c
 * PURPOSE: Core Gameplay Engine & Level Controller
 * 
 * DESCRIPTION:
 *   This module orchestrates the primary gameplay loop once a level has been
 *   loaded. It serves as the detective's main investigation workspace, managing:
 *     1. Level statement and prologue presentation with suspense animations.
 *     2. In-level command dispatching (Evidences, Interrogations, Histories).
 *     3. Case solution submission, validation, and level progression unlocking.
 *     4. Detailed case resolution narrative playback.
 *     5. User settings (typewriter text speed customization).
 *     6. Game progression statistics inspection.
 * =============================================================================
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <windows.h>
#include <conio.h>
#include <ctype.h>

#include "progress.h"
#include "level_select.h"
#include "main.h"
#include "interrogation.h"
#include "evidence.h"
#include "past_history.h"
#include "evidence_vault.h"
#include "game_main.h"
#include "color_palette.h"
#include "ans_validator.h"
#include "parser_utils.h"

/* Forward declarations for internal subroutines and external symbols */
extern int get_typewriter_delay(void);
extern void show_level_selection(void);
void submit_answer(int level);

/**
 * @brief Loads and initializes a game chapter/level.
 *
 * Execution flow:
 *   1. Displays suspense prelude animation if view_suspense_and_anim is true.
 *   2. Opens and displays the case prologue from "data/level_<level>/statement.txt".
 *   3. Applies subtle typewriter delay for the opening text (first 400 characters)
 *      to establish atmosphere.
 *   4. Hands control over to handle_menu() for the interactive investigation loop.
 *
 * @param level Integer chapter index (1 through 5).
 * @param view_suspense_and_anim If true, plays animated suspense banner before text.
 */
void load_level(int level, bool view_suspense_and_anim) {
    if (view_suspense_and_anim) {
        suspense_view();
    }
    
    /* Clear console screen and display level header */
    system("cls");
    printf("-----------------------------------------------------------------\n");
    printf("|                         L E V E L : %d                         |\n", level);
    printf("-----------------------------------------------------------------\n\n");

    /* Construct relative path to the level's statement narrative */
    char path[86];
    snprintf(path, sizeof(path), "data/level_%d/statement.txt", level);

    FILE *fp = fopen(path, "r");
    if (!fp) {
        printf("Error: Could not open level %d statement file\n", level);
        Sleep(1500);
        return;
    }

    /*
     * Read and display the case statement character-by-character.
     * Paces the first 400 characters with a 1ms sleep to provide
     * a subtle terminal typewriter effect.
     */
    int char_count = 0;
    char ch;
    while ((ch = fgetc(fp)) != EOF) {
        putchar(ch);
        char_count++;
        if (view_suspense_and_anim && char_count <= 400) {
            Sleep(1);
        }
    }
    fclose(fp);

    /* Transfer control to interactive investigation menu */
    handle_menu(level);
    return;
}

/**
 * @brief Renders the atmospheric suspense banner before a case begins.
 *
 * Prints a cryptic prelude message character-by-character using the current
 * typewriter delay setting to build dramatic anticipation.
 */
void suspense_view(void) {
    system("cls");

    char suspense_text[] = "Embrace yourself before unraveling a dark mystery...";

    printf("-----------------------------------------------------------------\n");
    printf("|               T H E   H O L L O W    C I P H E R               |\n");
    printf("-----------------------------------------------------------------\n\n");

    int delay = get_typewriter_delay();

    /* Render text character-by-character with typewriter delay */
    for (int i = 0; suspense_text[i] != '\0'; i++) {
        printf("%c", suspense_text[i]);
        fflush(stdout);
        Sleep(delay);
    }
    Sleep(500);
}

/**
 * @brief In-level interactive navigation menu and action dispatcher.
 *
 * Provides detective options:
 *   - [E] EVIDENCES: Inspect discovered physical evidence (requires unlocking the Evidence Vault).
 *   - [I] INTERROGATIONS: Question suspects and review interrogation transcripts.
 *   - [H] PAST HISTORIES: Review background lore and suspect dossiers.
 *   - [A] ENTER YOUR ANSWER: Formulate and submit final deduction to solve the case.
 *   - [X] EXIT: Return to level selection screen with confirmation prompt.
 *
 * Paced with 200ms sleep delays between options for atmospheric terminal rendering.
 *
 * @param level Integer chapter index (1 through 5).
 */
void handle_menu(int level) {
    char selection;

    while (1) {
        printf("\n\n");
        Sleep(200);
        printf("EVIDENCES [E]\n");
        Sleep(200);
        printf("INTERROGATIONS [I]\n");
        Sleep(200);
        printf("PAST HISTORIES [H]\n");
        Sleep(200);
        printf("ENTER YOUR ANSWER [A]\n");
        Sleep(200);
        printf("EXIT [X]\n\n");
        Sleep(200);

        printf("ENTER SELECTION: ");

        /* Read single character option safely */
        if (scanf(" %c", &selection) != 1) {
            int c;
            while ((c = getchar()) != '\n' && c != EOF);
            continue;
        }

        /* Flush remaining characters from standard input stream */
        int c;
        while ((c = getchar()) != '\n' && c != EOF);

        /* Normalize character to lowercase for case-insensitive matching */
        selection = tolower(selection);

        switch (selection) {
        case 'e':
            /*
             * Evidence Access:
             * If the vault has already been cracked in this session, open evidence list.
             * Otherwise, launch the number deduction puzzle to unlock the vault.
             */
            system("cls");
            if (is_vault_unlocked()) {
                evidence_menu_init(level);
            } else {
                show_vault_puzzle(level);
            }
            break;

        case 'i':
            /* Suspect Interrogations Menu */
            system("cls");
            interrogation_menu_init(level);
            system("cls");
            break;

        case 'h':
            /* Past History / Dossier Menu */
            system("cls");
            past_history_menu_init(level);
            system("cls");
            break;

        case 'a':
            /* Submit Final Deduction / Solution */
            system("cls");
            submit_answer(level);
            system("cls");
            break;

        case 'x':
            /* Exit with user confirmation */
            printf("\nAre you sure you want to exit to level selection? (Y/N): ");
            char confirm;
            if (scanf(" %c", &confirm) == 1) {
                while ((c = getchar()) != '\n' && c != EOF);
                if (confirm == 'y' || confirm == 'Y') {
                    system("cls");
                    /* Free dynamically parsed data structures before returning */
                    gc_collect();
                    show_level_selection();
                    return; /* Returns cleanly back through load_level */
                }
            } else {
                while ((c = getchar()) != '\n' && c != EOF);
            }
            system("cls");
            break;

        default:
            printf("Invalid selection. Please try again.\n");
            Sleep(1000);
            system("cls");
            break;
        }
    }
}

/**
 * @brief Handles player deduction submission, verification, and campaign progression.
 *
 * Workflow:
 *   1. Reads the detective's written answer string via fgets() to support multi-word answers.
 *   2. Trims carriage return and newline line endings using strcspn().
 *   3. Verifies non-empty answer.
 *   4. Opens "data/level_<level>/solution.txt" and delegates verification to is_ans_correct().
 *   5. If CORRECT:
 *        - Plays animated success banner.
 *        - Unlocks the next campaign chapter via set_level_unlocked(level).
 *        - Reads and displays the full narrative case explanation (parse_solution_desc),
 *          skipping the "[Explanation]" header tag.
 *        - Returns player to chapter selection.
 *   6. If INCORRECT:
 *        - Displays failure message and prompts player to keep investigating.
 *        - Returns player to chapter selection.
 *
 * @param level The current level index being solved.
 */
void submit_answer(int level) {
    char response[256];
    printf("-----------------------------------------------------------------\n");
    printf("|                    S U B M I T   A N S W E R                   |\n");
    printf("-----------------------------------------------------------------\n\n");
    printf("Enter your answer:\n\n");
    
    /* Safely read player's response line */
    if (fgets(response, sizeof(response), stdin) == NULL) {
        printf("Error reading input.\n");
        Sleep(1500);
        return;
    }
    
    /* Strip trailing \r and \n characters */
    response[strcspn(response, "\r\n")] = '\0';
    
    /* Reject blank input */
    if (strlen(response) == 0) {
        printf("\nAnswer cannot be empty.\n");
        Sleep(1500);
        return;
    }
    
    /* Construct path to solution file */
    char sol_path[86];
    snprintf(sol_path, sizeof(sol_path), "data/level_%d/solution.txt", level);
    
    FILE *sol_fp = fopen(sol_path, "r");
    if (!sol_fp) {
        printf("\nError: Could not open solution file.\n");
        Sleep(1500);
        return;
    }
    
    printf("\nChecking your answer...\n");
    Sleep(1000);
    
    /* Validate answer using fuzzy/exact matching in ans_validator */
    if (is_ans_correct(response, sol_fp)) {
        fclose(sol_fp);
        sol_fp = NULL;

        int delay = get_typewriter_delay();

        /* Play animated correct banner */
        char correct_text[] = "\n>>> CORRECT! <<<\n";
        printf(GREEN);
        for (int i = 0; correct_text[i] != '\0'; i++) {
            printf("%c", correct_text[i]);
            fflush(stdout);
            Sleep(delay);
        }
        Sleep(300);
        printf(BG_LEATHER GOLD);
        printf("\nWell done, detective. You've solved the case.\n");
        
        /* Unlock progression for the subsequent level */
        if (level < MAX_LEVELS) {
            set_level_unlocked(level);
            printf("\nLevel %d unlocked!\n", level + 1);
        } else {
            printf("\nYou have completed all available levels!\n");
        }
        
        /*
         * Extract and display narrative explanation from solution file
         * using typewriter text pacing.
         */
        sol_fp = fopen(sol_path, "r");
        if (sol_fp) {
            char *explanation = parse_solution_desc(sol_fp);
            fclose(sol_fp);

            if (explanation) {
                printf("\n--- EXPLANATION ---\n\n");

                /* Locate and skip the "[Explanation]" marker */
                char *exp_text = strstr(explanation, "[Explanation]");
                if (exp_text) {
                    exp_text += 13; /* Skip past "[Explanation]" */
                    while (*exp_text == '\r' || *exp_text == '\n') {
                        exp_text++;
                    }
                } else {
                    exp_text = explanation;
                }

                /* Typewriter output for the case resolution narrative */
                for (int i = 0; exp_text[i] != '\0'; i++) {
                    printf("%c", exp_text[i]);
                    fflush(stdout);
                    Sleep(3);
                }
                printf("\n");

                free(explanation);
            }
        }
    } else {
        if (sol_fp) {
            fclose(sol_fp);
        }
        printf(RED "\n>>> INCORRECT <<<\n" GOLD);
        printf("\nThat's not the right answer. Keep investigating...\n");
    }
    
    printf("\nPress any key to continue...");
    _getch();
    show_level_selection();
}

/* Static global setting controlling typewriter delay in milliseconds */
static int typewriter_delay = 30; // Default delay in ms

/**
 * @brief Interactive configuration menu to adjust typewriter animation speed.
 *
 * Allows adjusting the delay per character:
 *   - [F] Faster: Decreases delay by 5ms (minimum 5ms limit).
 *   - [S] Slower: Increases delay by 5ms (maximum 200ms limit).
 *   - [D] Default: Restores factory 30ms setting.
 *   - [B] Back: Returns to caller.
 */
void text_speed_menu(void) {
    while (1) {
        system("cls");
        printf(BG_LEATHER GOLD);
        printf("-----------------------------------------------------------------\n");
        printf("|                      T E X T   S P E E D                       |\n");
        printf("-----------------------------------------------------------------\n\n");
        
        printf("Current speed: %d ms per character\n\n", typewriter_delay);
        printf("FASTER [F] - Decrease delay\n");
        printf("SLOWER [S] - Increase delay\n");
        printf("DEFAULT [D] - Reset to 30ms\n");
        printf("GO BACK [B]\n\n");
        
        printf("ENTER SELECTION: ");
        char selection;
        if (scanf(" %c", &selection) != 1) {
            int c;
            while ((c = getchar()) != '\n' && c != EOF);
            continue;
        }
        int c;
        while ((c = getchar()) != '\n' && c != EOF);
        
        selection = tolower(selection);
        
        switch (selection) {
        case 'f':
            if (typewriter_delay > 5) {
                typewriter_delay -= 5;
                printf("\nSpeed increased! New delay: %dms\n", typewriter_delay);
            } else {
                printf("\nAlready at maximum speed!\n");
            }
            Sleep(1000);
            break;
        case 's':
            if (typewriter_delay < 200) {
                typewriter_delay += 5;
                printf("\nSpeed decreased! New delay: %dms\n", typewriter_delay);
            } else {
                printf("\nAlready at minimum speed!\n");
            }
            Sleep(1000);
            break;
        case 'd':
            typewriter_delay = 30;
            printf("\nReset to default (30ms)\n");
            Sleep(1000);
            break;
        case 'b':
            return;
        default:
            printf("\nInvalid selection.\n");
            Sleep(1000);
            break;
        }
    }
}

/**
 * @brief Retrieves the current typewriter delay configuration.
 *
 * @return Current character delay in milliseconds.
 */
int get_typewriter_delay(void) {
    return typewriter_delay;
}

/**
 * @brief Displays overall campaign statistics and progression metrics.
 *
 * Queries save data via load_progress() and is_level_unlocked(), displaying:
 *   - Total unlocked levels count and percentage completed.
 *   - Status breakdown for Level 1 through 5 (UNLOCKED / LOCKED).
 *   - Current typewriter delay setting.
 *   - Evidence Vault unlock state for the current session.
 */
void statistics_menu(void) {
    system("cls");
    printf(BG_LEATHER GOLD);
    printf("-----------------------------------------------------------------\n");
    printf("|                      S T A T I S T I C S                       |\n");
    printf("-----------------------------------------------------------------\n\n");
    
    /* Load latest progress from progress.thcdb */
    load_progress();
    
    /* Calculate unlocked chapters */
    int unlocked_count = 0;
    for (int i = 0; i < MAX_LEVELS; i++) {
        if (is_level_unlocked(i)) unlocked_count++;
    }
    
    printf("Levels unlocked: %d / %d\n", unlocked_count, MAX_LEVELS);
    printf("Completion: %d%%\n\n", (unlocked_count * 100) / MAX_LEVELS);
    
    /* Enumerate individual level states */
    for (int i = 0; i < MAX_LEVELS; i++) {
        printf("Level %d: %s\n", i + 1, is_level_unlocked(i) ? "UNLOCKED" : "LOCKED");
    }
    
    printf("\nText speed: %d ms/char\n", typewriter_delay);
    
    /* Check session Evidence Vault status */
    printf("Evidence Vault: %s\n", is_vault_unlocked() ? "UNLOCKED" : "LOCKED");
    
    printf("\nPress any key to continue...");
    _getch();
}
