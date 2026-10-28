#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <stdbool.h>

#include "vault_pass.h"
#include "parser_utils.h"

static bool seeded = false;

/*
 * Checks if a 5-digit number consists of strictly unique/distinct digits.
 * Example: 98651 is valid (all 5 digits are unique: 9, 8, 6, 5, 1).
 * Example: 98151 is invalid (digit 1 appears twice).
 */
bool has_unique_digits(int num) {
    if (num < 10000 || num > 99999) {
        return false;
    }
    int digits[5] = {
        (num / 10000) % 10,
        (num / 1000) % 10,
        (num / 100) % 10,
        (num / 10) % 10,
        num % 10
    };

    for (int i = 0; i < 5; i++) {
        for (int j = i + 1; j < 5; j++) {
            if (digits[i] == digits[j]) {
                return false;
            }
        }
    }
    return true;
}

/*
 * Generates a randomized 5-digit passcode where all 5 digits are strictly unique.
 * Demonstrates:
 *   1. UNION: Uses VaultPasscodeOverlay to store both the integer and char representation.
 *   2. PREPROCESSORS & #CONDITIONAL: Diagnostic log under #if THC_DEBUG.
 *   3. DEBUGGING CODE (Place 2 of 2): Logs passcode details when compiled in debug mode.
 */
int generate_pass(void) {
    if (!seeded) {
        srand((unsigned int)time(NULL));
        seeded = true;
    }

    int digits[10] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};

    /*
     * Select a non-zero leading digit (1-9) for position 0 to guarantee
     * a valid 5-digit number >= 10000.
     */
    int first_idx = 1 + (rand() % 9);
    int temp = digits[0];
    digits[0] = digits[first_idx];
    digits[first_idx] = temp;

    /*
     * Fisher-Yates shuffle the remaining indices (1-9) so that positions 1-4
     * draw unique, uniformly distributed digits from the remaining set.
     */
    for (int i = 9; i > 1; i--) {
        int j = 1 + (rand() % i);
        temp = digits[i];
        digits[i] = digits[j];
        digits[j] = temp;
    }

    int result = 0;
    for (int i = 0; i < 5; i++) {
        result = result * 10 + digits[i];
    }

    /*
     * =========================================================================
     * UNION IMPLEMENTATION (Overlay View)
     * =========================================================================
     * Store passcode in union overlay to provide both signed integer and raw bit access.
     */
    VaultPasscodeOverlay pass_overlay;
    pass_overlay.numeric_code = result;

    /*
     * =========================================================================
     * DEBUGGING CODE (Place 2 of 2): Vault Passcode Diagnostics
     * =========================================================================
     * Conditional compilation directive: Active only if THC_DEBUG is set to 1.
     * Prints the generated secret passcode during playtesting or QA debug builds.
     */
#if defined(THC_DEBUG) && (THC_DEBUG == 1)
    printf("[DEBUG VAULT] Generated secret passcode: %05d (raw bits: 0x%08X)\n",
           pass_overlay.numeric_code, pass_overlay.raw_bits);
#endif

    return pass_overlay.numeric_code;
}

bool check_vault_unlocked(FILE *file) {
    int status;

    if (file == NULL) {
        return false;
    }

    /* Rewind file position indicator back to the beginning of the stream */
    rewind(file);

    if (fscanf(file, "%d", &status) != 1) {
        return false;
    }

    return status == 1;
}

void set_vault_unlocked(FILE *file) {
    if (file == NULL) {
        return;
    }

    rewind(file);

    if (fprintf(file, "1\n") < 0) {
        return;
    }

    fflush(file);
}
