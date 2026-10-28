#ifndef VAULT_PASS_H
#define VAULT_PASS_H

#include <stdio.h>
#include <stdbool.h>

/*
 * =============================================================================
 * UNION IMPLEMENTATION: Dual-View Passcode Overlay
 * =============================================================================
 * An overlay union maps different representations over the exact same block of
 * bytes. Here, VaultPasscodeOverlay allows treating the generated 5-digit passcode
 * either as an integer (for arithmetic/comparison operations) or as an array of
 * characters (for string display and character-by-character analysis).
 *
 * Both members occupy the same memory location, giving dual interpretations
 * without the overhead of runtime conversions.
 */
typedef union {
    int numeric_code;
    char digit_chars[6];
} VaultPasscodeOverlay;

int  generate_pass(void);
bool check_vault_unlocked(FILE *file);
void set_vault_unlocked(FILE *file);

#endif /* VAULT_PASS_H */
