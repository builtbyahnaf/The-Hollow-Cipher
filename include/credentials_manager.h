#ifndef CREDENTIALS_MANAGER_H
#define CREDENTIALS_MANAGER_H

bool is_pass_valid(char* pass_string);
bool is_user_valid(char* username, char* password);
void init_credentials(void);
void list_users(void);
bool add_user(char* username, char* password);
bool remove_user(int index);
void set_new_pass(void);
void add_new_user_interactive(void);
void remove_user_interactive(void);
void write_pass(const char* pass_string);
void pass_changelog(void);

#endif