#ifndef CREDENTIALS_MANAGER_H
#define CREDENTIALS_MANAGER_H

bool is_pass_valid(char* pass_string);
void set_new_pass(void);
void write_pass(const char* pass_string);
void pass_changelog(void);

#endif