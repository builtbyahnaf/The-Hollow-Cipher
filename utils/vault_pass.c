#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <stdbool.h>

#include "vault_pass.h"
#include "parser_utils.h"

static bool seeded = false;

/*
 * Generates a randomized 5-digit passcode with distinct leading digits.
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
    for (int i = 9; i > 0; i--) {
        int j = rand() % (i + 1);
        int temp = digits[i];
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
     * Overlay the calculated integer code into the union and format its string representation.
     */
    VaultPasscodeOverlay pass_overlay;
    pass_overlay.numeric_code = result;
    snprintf(pass_overlay.digit_chars, sizeof(pass_overlay.digit_chars), "%05d", result);

    /*
     * =========================================================================
     * DEBUGGING CODE (Place 2 of 2): Vault Passcode Diagnostics
     * =========================================================================
     * Conditional compilation directive: Active only if THC_DEBUG is set to 1.
     * Prints the generated secret passcode during playtesting or QA debug builds.
     */
#if defined(THC_DEBUG) && (THC_DEBUG == 1)
    printf("[DEBUG VAULT] Generated secret passcode: %05d (as string: \"%s\")\n",
           pass_overlay.numeric_code, pass_overlay.digit_chars);
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
