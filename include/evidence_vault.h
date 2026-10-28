#ifndef EVIDENCE_VAULT_H
#define EVIDENCE_VAULT_H

#include <stdbool.h>

void show_vault_puzzle(int level);
void handle_puzzle(int pass);
void set_puzzle_passed(bool p);
int get_puzzle_password(void);
bool is_vault_unlocked(void);
bool has_unique_digits(int num);

#endif