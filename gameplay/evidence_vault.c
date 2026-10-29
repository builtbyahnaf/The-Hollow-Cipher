/**
 * =============================================================================
 * PROJECT: The Hollow Cipher (THC)
 * FILE: gameplay/evidence_vault.c
 * PURPOSE: Evidence Vault Mini-Game (Numeric Mastermind / Bulls & Cows Puzzle)
 * 
 * DESCRIPTION:
 *   To access physical case evidence, detectives must first unlock the Evidence
 *   Vault. This file implements an interactive 5-digit numeric deduction puzzle
 *   reminiscent of Wordle and Mastermind (Bulls and Cows).
 * 
 * PUZZLE RULES & LOGIC:
 *   1. The vault generates a secret 5-digit passcode with strictly unique
 *      digits (no repeating numbers, e.g., 98651 is valid; 98151 is invalid).
 *   2. The player submits 5-digit guesses, which must also contain 5 unique digits.
 *   3. Two-pass color feedback is provided for each digit:
 *        - GREEN (Mint Green): Digit is present and in the exact position.
 *        - YELLOW (Acid Lime): Digit is present but in the wrong position.
 *        - RED (Crimson): Digit is completely absent from the passcode.
 *   4. Cracking the entire code (all 5 digits green) unlocks the vault for the
 *      session and opens the Evidence Panel.
 * =============================================================================
 */

#include <stdio.h>
#include <windows.h>
#include <stdlib.h>
#include <stdbool.h>

#include "evidence.h"
#include "game_main.h"
#include "vault_pass.h"
#include "color_palette.h"

/* Tracks whether the puzzle was successfully solved during this session */
static bool is_puzzle_passed = false;

/* The current 5-digit secret password */
static int puzzle_password = 0;

/* Active chapter/level index */
static int level;

/* Vault door state: true once cracked, allowing direct access to evidence */
static bool vault_unlocked = false;

/* Forward declaration for core puzzle solver loop */
void handle_puzzle(int pass);

/**
 * @brief Sets the puzzle passed state.
 *
 * @param p Boolean indicating if the puzzle is considered passed.
 */
void set_puzzle_passed(bool p) {
    is_puzzle_passed = p;
}

/**
 * @brief Checks if the evidence vault has been cracked during this session.
 *
 * @return true if the vault is unlocked, false otherwise.
 */
bool is_vault_unlocked(void) {
    return vault_unlocked;
}

/**
 * @brief Retrieves the active secret puzzle passcode.
 *
 * @return Integer representing the 5-digit passcode.
 */
int get_puzzle_password(void) {
    return puzzle_password;
}

/**
 * @brief Initializes and renders the Evidence Vault puzzle introduction and rules.
 *
 * Execution Steps:
 *   1. Resets solver state (is_puzzle_passed = false, puzzle_password = 0).
 *   2. Generates a random 5-digit passcode with guaranteed unique digits
 *      using generate_pass() and has_unique_digits().
 *   3. Renders the Evidence Vault banner and rules with color-coded
 *      demonstrations using atmospheric typewriter pacing.
 *   4. Directly delegates to handle_puzzle() to begin the guess entry loop.
 *
 * @param _level The active game chapter index (1-5).
 */
void show_vault_puzzle(int _level) {
    /* Reset puzzle state for a fresh attempt */
    is_puzzle_passed = false;
    puzzle_password = 0;
    
    /* Clear console screen and store level */
    system("cls");
    level = _level;

    /* Apply thematic console background and foreground styling */
    printf(BG_LEATHER GOLD);

    /* Generate a random 5-digit passcode with strictly unique digits */
    puzzle_password = generate_pass();
    while (!has_unique_digits(puzzle_password)) {
        puzzle_password = generate_pass();
    }

    /* Display header banner */
    printf("-----------------------------------------------------------------\n");
    printf("|                  E V I D E N C E   V A U L T                   |\n");
    printf("-----------------------------------------------------------------\n\n");

    int delay = 10;
    char vault_text_1[] = "The evidences for this case are secured inside this vault. You have to guess the correct password by trial and error.\n"
                          "All 5 digits in the password are unique (no repeating digits).\n\n"
                          "The rules are as follows:\n\n";
    char vault_text_2[] = "The digit is present in the password and it is in the right position.\n";
    char vault_text_3[] = "The digit is present in the password but it is in the wrong position.\n";
    char vault_text_4[] = "The digit is absent in the password.\n";

    /* Render introduction paragraph */
    for (int i = 0; vault_text_1[i] != '\0'; i++) {
        printf("%c", vault_text_1[i]);
        fflush(stdout);
        Sleep(delay);
    }

    /* Render green rule description */
    printf(MINT_GREEN);
    printf("GREEN = " GOLD);
    for (int i = 0; vault_text_2[i] != '\0'; i++) {
        printf("%c", vault_text_2[i]);
        fflush(stdout);
        Sleep(delay);
    }

    /* Render yellow rule description */
    printf(ACID_LIME);
    printf("YELLOW = " GOLD);
    for (int i = 0; vault_text_3[i] != '\0'; i++) {
        printf("%c", vault_text_3[i]);
        fflush(stdout);
        Sleep(delay);
    }

    /* Render red rule description */
    printf(CRIMSON);
    printf("RED = " GOLD);
    for (int i = 0; vault_text_4[i] != '\0'; i++) {
        printf("%c", vault_text_4[i]);
        fflush(stdout);
        Sleep(delay);
    }
    
    /*
     * Intro done — start the puzzle loop directly here so the vault header
     * stays on screen while the player enters guesses.
     */
    handle_puzzle(puzzle_password);
}

/**
 * @brief Interactive input loop and Mastermind evaluation algorithm.
 *
 * Algorithm Details:
 *   1. Extracts the 5 individual digits of the passcode into array pd[5].
 *   2. Prompts player for a 5-digit number or 0 to abort back to level menu.
 *   3. Enforces strict input validation:
 *        - Checks for non-numeric input (flushes stdin).
 *        - Checks for valid 5-digit range [10000, 99999].
 *        - Validates unique digits constraint via has_unique_digits().
 *   4. Extracts the 5 digits of the player's guess into array gd[5].
 *   5. Performs Two-Pass Mastermind Evaluation:
 *        - Pass 1 (Exact Matches / Green): If gd[i] == pd[i], result[i] = 2,
 *          and mark pass_used[i] = 1.
 *        - Pass 2 (Position Mismatches / Yellow): For remaining unmatched digits,
 *          checks if gd[i] matches any unused pd[j]. If so, result[i] = 1,
 *          and mark pass_used[j] = 1.
 *        - Unmatched digits remain 0 (Absent / Red).
 *   6. Prints the 5 digits color-coded according to evaluation results.
 *   7. If all 5 digits are green (result 2): unlocks the vault, sets session
 *      flags, and opens the evidence panel via evidence_menu_init().
 *
 * @param pass The 5-digit integer passcode to be cracked.
 */
void handle_puzzle(int pass) {
    if (is_puzzle_passed) {
        return;
    }

    /*
     * Password validation / fallback generation:
     * Guarantees the secret passcode has 5 strictly unique digits.
     * Example: 98651 is valid (all distinct digits).
     * Example: 98151 is invalid (duplicate digit '1').
     */
    if (!has_unique_digits(pass)) {
        pass = generate_pass();
        while (!has_unique_digits(pass)) {
            pass = generate_pass();
        }
        puzzle_password = pass;
    }

    int guess = 0;

    /* Extract passcode digits from most significant to least significant */
    int pd[5] = {
        (pass / 10000) % 10,
        (pass / 1000) % 10,
        (pass / 100) % 10,
        (pass / 10) % 10,
        pass % 10
    };

RETRY:

    printf("\n\nEnter your guess (5 digits, or 0 to go back): ");

    /*
     * Robust input validation: Prevents crashes and infinite loops
     * when the player enters non-numeric characters.
     */
    if (scanf("%d", &guess) != 1) {
        int c;
        while ((c = getchar()) != '\n' && c != EOF);
        printf("Please enter a valid 5-digit number.\n");
        goto RETRY;
    }

    /* Flush stdin buffer up to newline */
    int c;
    while ((c = getchar()) != '\n' && c != EOF);

    /* 0 permits the player to retreat back to the level menu */
    if (guess == 0) {
        system("cls");
        load_level(level, false);
        return;
    }

    /* Bound check for 5-digit numbers */
    if (guess < 10000 || guess > 99999) {
        printf("Please enter a 5-digit number.\n");
        goto RETRY;
    }

    /*
     * Password guess validation:
     * Enforces that every entered password guess has 5 unique digits.
     * Example: 98651 is valid (all digits distinct), 98151 is invalid (duplicate '1').
     */
    if (!has_unique_digits(guess)) {
        printf("Invalid password: All 5 digits must be unique (e.g. 98651 is valid, 98151 is invalid).\n");
        goto RETRY;
    }

    /* Extract guess digits into array */
    int gd[5] = {
        (guess / 10000) % 10,
        (guess / 1000) % 10,
        (guess / 100) % 10,
        (guess / 10) % 10,
        guess % 10
    };

    /*
     * result array:
     *   0 = Red (Absent from password)
     *   1 = Yellow (Present, but wrong position)
     *   2 = Green (Exact match: right digit in right position)
     */
    int result[5] = {0};
    int pass_used[5] = {0}; /* Tracks digits in password already matched */

    /* First pass: Detect exact matches (Green) */
    for (int i = 0; i < 5; i++) {
        if (gd[i] == pd[i]) {
            result[i] = 2;
            pass_used[i] = 1;
        }
    }

    /* Second pass: Detect position mismatches (Yellow) for remaining digits */
    for (int i = 0; i < 5; i++) {
        if (result[i] == 0) {
            for (int j = 0; j < 5; j++) {
                if (!pass_used[j] && gd[i] == pd[j]) {
                    result[i] = 1;
                    pass_used[j] = 1;
                    break;
                }
            }
        }
    }

    /* Render evaluated guess with ANSI terminal color codes */
    for (int i = 0; i < 5; i++) {
        if (result[i] == 2) {
            printf(MINT_GREEN "%d ", gd[i]);
        } else if (result[i] == 1) {
            printf(ACID_LIME "%d ", gd[i]);
        } else {
            printf(CRIMSON "%d ", gd[i]);
        }
        printf(GOLD);
    }

    /* Check win condition: All 5 digits matched in correct positions */
    if (result[0] == 2 && result[1] == 2 && result[2] == 2 && result[3] == 2 && result[4] == 2) {
        printf(MINT_GREEN "Congratulations! Unlocking Evidence Vault now..." GOLD);
        vault_unlocked = true;
        set_puzzle_passed(true);
        Sleep(2000);
        evidence_menu_init(level);
        return;
    }

    if (!is_puzzle_passed) printf("\nTry again.");

    goto RETRY;
}
