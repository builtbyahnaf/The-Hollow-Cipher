
#ifndef VAULT_PASS_H
#define VAULT_PASS_H

#include <stdio.h>
#include <stdbool.h>

int generate_pass(void);

bool check_vault_unlocked(FILE *file);

void set_vault_unlocked(FILE *file);

#endif
