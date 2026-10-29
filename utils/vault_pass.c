/**
 * =============================================================================
 * PROJECT: The Hollow Cipher (THC)
 * FILE: utils/vault_pass.c
 * PURPOSE: Evidence Vault Passcode Generator and Validator
 *
 * DESCRIPTION:
 *   This module handles all randomized passcode generation and digit-uniqueness
 *   validation for the Evidence Vault mini-game in evidence_vault.c.
 *
 *   KEY IMPLEMENTATIONS DEMONSTRATED:
 *
 *   1. UNION (VaultPasscodeOverlay):
 *      After generating the integer passcode, it is stored into a
 *      VaultPasscodeOverlay union which provides two views of the same
 *      4-byte memory location:
 *        - numeric_code (int):         Used for all arithmetic operations.
 *        - raw_bits (unsigned int):    Used for bitwise inspection in debug mode.
 *      No conversion or copying is needed — both members point to the same address.
 *
 *   2. PREPROCESSOR / CONDITIONAL COMPILATION (#if THC_DEBUG):
 *      "Debugging Code Place 2 of 2": When compiled with -DTHC_DEBUG=1,
 *      the generated secret passcode is printed to the console for QA/testing.
 *      In production builds (THC_DEBUG=0 or undefined), this block is completely
 *      excluded by the preprocessor — zero runtime overhead.
 *
 *   3. FISHER-YATES SHUFFLE ALGORITHM:
 *      Generates a uniform random permutation of the 10 digits (0-9), then
 *      reads the first 5 digits as the passcode. This guarantees uniqueness
 *      mathematically without rejection-sampling loops.
 * =============================================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <stdbool.h>

#include "vault_pass.h"
#include "parser_utils.h"

/* Tracks whether srand() has been called; ensures seeding happens only once */
static bool seeded = false;

/**
 * @brief Checks if a 5-digit integer has all strictly unique (non-repeating) digits.
 *
 * Extracts each of the 5 digits using positional division and modulo arithmetic,
 * then uses a nested loop (O(n²) for n=5) to compare every pair.
 *
 * Examples:
 *   has_unique_digits(98651) → true  (digits: 9, 8, 6, 5, 1 — all distinct)
 *   has_unique_digits(98151) → false (digit 1 appears at positions 2 and 4)
 *   has_unique_digits(99999) → false (digit 9 repeats throughout)
 *
 * @param num The integer to check. Must be in the 5-digit range [10000, 99999].
 * @return true if all 5 digits are unique, false if out of range or any repeat.
 */
bool has_unique_digits(int num) {
    /* Reject anything outside the valid 5-digit range */
    if (num < 10000 || num > 99999) {
        return false;
    }

    /* Extract all 5 digits using positional arithmetic */
    int digits[5] = {
        (num / 10000) % 10,  /* Ten-thousands digit */
        (num / 1000) % 10,   /* Thousands digit */
        (num / 100) % 10,    /* Hundreds digit */
        (num / 10) % 10,     /* Tens digit */
        num % 10             /* Units digit */
    };

    /* Compare every pair of digits; any match indicates a duplicate */
    for (int i = 0; i < 5; i++) {
        for (int j = i + 1; j < 5; j++) {
            if (digits[i] == digits[j]) {
                return false;
            }
        }
    }
    return true;
}

/**
 * @brief Generates a random 5-digit passcode with strictly unique digits.
 *
 * Algorithm (Fisher-Yates Partial Shuffle):
 *   1. Initialize a pool of all 10 digits: {0, 1, 2, ..., 9}.
 *   2. Guarantee a non-zero leading digit by randomly selecting from {1..9}
 *      and swapping it to position 0. This ensures the result is always >= 10000.
 *   3. Apply a partial Fisher-Yates shuffle to positions 1-9:
 *        - For i from 9 down to 2: swap digits[i] with a random digits[j]
 *          where j is chosen from [1, i].
 *      This produces a uniformly random permutation of the remaining digits.
 *   4. Read the first 5 entries of the shuffled pool as the passcode digits.
 *   5. Combine into a single integer using positional assembly (base-10 encoding).
 *
 * Union Demo (VaultPasscodeOverlay):
 *   The integer result is stored into pass_overlay.numeric_code.
 *   The same memory is simultaneously readable as pass_overlay.raw_bits
 *   (unsigned int) for bitwise diagnostics in debug mode — without any
 *   copy or cast. This demonstrates that union members share one address.
 *
 * Conditional Compilation (Debugging Code Place 2 of 2):
 *   When THC_DEBUG=1, the passcode is printed with its raw bit representation.
 *   This is completely omitted from production builds by the preprocessor.
 *
 * @return A randomly generated 5-digit integer with all unique digits.
 */
int generate_pass(void) {
    /* Seed the PRNG once per program session using current epoch time */
    if (!seeded) {
        srand((unsigned int)time(NULL));
        seeded = true;
    }

    /* Pool of all available digits 0-9 */
    int digits[10] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};

    /*
     * Step 1: Select a non-zero leading digit (1-9) for position 0.
     * Picking from index [1..9] guarantees the leading digit is never 0,
     * which would produce a 4-digit number (e.g., 01234 = 1234).
     */
    int first_idx = 1 + (rand() % 9);
    int temp = digits[0];
    digits[0] = digits[first_idx];
    digits[first_idx] = temp;

    /*
     * Step 2: Fisher-Yates shuffle on positions 1-9 to randomize the
     * remaining digits. By choosing j in [1, i] we never disturb position 0.
     *
     * Fisher-Yates guarantees a uniform random permutation:
     *   for i = 9 downto 2:
     *     j = random in [1, i]
     *     swap digits[i] and digits[j]
     */
    for (int i = 9; i > 1; i--) {
        int j = 1 + (rand() % i);
        temp = digits[i];
        digits[i] = digits[j];
        digits[j] = temp;
    }

    /* Step 3: Assemble the first 5 shuffled digits into a single integer */
    int result = 0;
    for (int i = 0; i < 5; i++) {
        result = result * 10 + digits[i];
    }

    /*
     * =========================================================================
     * UNION IMPLEMENTATION (VaultPasscodeOverlay)
     * =========================================================================
     * Store the passcode in the union overlay. Both members occupy the exact
     * same 4-byte memory address — no copy is performed.
     *   - pass_overlay.numeric_code: the integer passcode for game logic.
     *   - pass_overlay.raw_bits:     unsigned view of the same bytes for debug.
     */
    VaultPasscodeOverlay pass_overlay;
    pass_overlay.numeric_code = result;

    /*
     * =========================================================================
     * DEBUGGING CODE (Place 2 of 2): Vault Passcode Diagnostics
     * =========================================================================
     * This block is active only when compiled with -DTHC_DEBUG=1.
     * In release builds (THC_DEBUG=0 or not defined), the preprocessor removes
     * this entire block before compilation — zero performance cost.
     *
     * Displays both the decimal passcode and its hex bit representation,
     * allowing QA testers to verify the puzzle answer without solving it manually.
     */
#if defined(THC_DEBUG) && (THC_DEBUG == 1)
    printf("[DEBUG VAULT] Generated secret passcode: %05d (raw bits: 0x%08X)\n",
           pass_overlay.numeric_code, pass_overlay.raw_bits);
#endif

    return pass_overlay.numeric_code;
}

/**
 * @brief Reads vault unlock status from an open file handle.
 *
 * Reads a single integer from the file; returns true if the value is 1
 * (unlocked), false for any other value or on read failure.
 *
 * @param file Open FILE* handle pointing to a vault status file.
 * @return true if the stored status value equals 1, false otherwise.
 */
bool check_vault_unlocked(FILE *file) {
    int status;

    if (file == NULL) {
        return false;
    }

    /* Rewind file position indicator back to beginning of stream */
    rewind(file);

    if (fscanf(file, "%d", &status) != 1) {
        return false; /* Read failed or file is empty */
    }

    return status == 1;
}

/**
 * @brief Writes vault unlock status (1 = unlocked) to an open file handle.
 *
 * Rewinds to the beginning of the file and overwrites content with "1\n".
 * Flushes the stream to guarantee the write is committed to disk.
 *
 * @param file Open FILE* handle to the vault status file (must be writable).
 */
void set_vault_unlocked(FILE *file) {
    if (file == NULL) {
        return;
    }

    rewind(file);

    if (fprintf(file, "1\n") < 0) {
        return; /* Write failed; leave file unchanged */
    }

    fflush(file); /* Ensure data reaches disk */
}
