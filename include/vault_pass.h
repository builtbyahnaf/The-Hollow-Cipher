#ifndef VAULT_PASS_H
#define VAULT_PASS_H

#include <stdio.h>
#include <stdbool.h>

/*
 * =============================================================================
 * UNION IMPLEMENTATION: Dual-View Passcode Representation
 * =============================================================================
 * Demonstrates a C union overlay. The union provides dual interpretations of the
 * generated 5-digit passcode in memory:
 *   - numeric_code: 32-bit signed integer representation for numerical operations.
 *   - raw_bits: 32-bit unsigned representation for bitwise verification/masking.
 *
 * Both members occupy the exact same 4-byte memory location without runtime conversion.
 */
typedef union {
    int numeric_code;
    unsigned int raw_bits;
} VaultPasscodeOverlay;

int  generate_pass(void);
bool has_unique_digits(int num);
bool check_vault_unlocked(FILE *file);
void set_vault_unlocked(FILE *file);

#endif /* VAULT_PASS_H */
