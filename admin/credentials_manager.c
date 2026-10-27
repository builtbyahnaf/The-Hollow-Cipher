#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdbool.h>
#include "credentials_manager.h"

#define CREDENTIALS_FILE "data/credentials.thcdb"
#define MAX_PASS_LEN 128
#define MAX_USERS 50

typedef struct {
    char username[64];
    char password[MAX_PASS_LEN];
    bool protected;
} User;

static User users[MAX_USERS];
static int user_count = 0;

static void load_users(void) {
    FILE* file = fopen(CREDENTIALS_FILE, "r");
    if (!file) {
        return;
    }

    user_count = 0;
    char line[256];
    while (fgets(line, sizeof(line), file) && user_count < MAX_USERS) {
        size_t len = strlen(line);
        if (len > 0 && line[len - 1] == '\n') {
            line[len - 1] = '\0';
        }
        if (strlen(line) == 0) continue;

        char* user_start = strstr(line, "Username: ");
        if (!user_start) continue;
        user_start += 10;

        char* pass_start = strstr(user_start, " Password: ");
        if (!pass_start) continue;

        *pass_start = '\0';
        pass_start += 11;

        strncpy(users[user_count].username, user_start, sizeof(users[user_count].username) - 1);
        strncpy(users[user_count].password, pass_start, sizeof(users[user_count].password) - 1);

        if (user_count < 2) {
            users[user_count].protected = true;
        } else {
            users[user_count].protected = false;
        }

        user_count++;
    }
    fclose(file);
}

static void save_users(void) {
    FILE* file = fopen(CREDENTIALS_FILE, "w");
    if (!file) {
        printf("Error: Could not open credentials file for writing.\n");
        return;
    }

    time_t now = time(NULL);
    struct tm* tm_info = localtime(&now);
    char timestamp[128];
    strftime(timestamp, sizeof(timestamp), "%d, %B, %Y, %A, %I:%M:%S %p", tm_info);

    for (int i = 0; i < user_count; i++) {
        fprintf(file, "Username: %s Password: %s\n", users[i].username, users[i].password);
    }
    fprintf(file, "LAST_CHANGED: %s\n", timestamp);

    fclose(file);
}

void init_credentials(void) {
    load_users();
}

bool is_pass_valid(char* pass_string) {
    init_credentials();
    for (int i = 0; i < user_count; i++) {
        if (strcmp(users[i].password, pass_string) == 0) {
            return true;
        }
    }
    return false;
}

bool is_user_valid(char* username, char* password) {
    init_credentials();
    for (int i = 0; i < user_count; i++) {
        if (strcmp(users[i].username, username) == 0 && strcmp(users[i].password, password) == 0) {
            return true;
        }
    }
    return false;
}

void list_users(void) {
    init_credentials();
    printf("-----------------------------------------------------------------\n");
    printf("|                    U S E R   L I S T                          |\n");
    printf("-----------------------------------------------------------------\n\n");

    for (int i = 0; i < user_count; i++) {
        printf("%d. %s%s\n", i + 1, users[i].username, users[i].protected ? " [PROTECTED]" : "");
    }
    printf("\nTotal users: %d\n", user_count);
}

bool add_user(char* username, char* password) {
    init_credentials();

    if (user_count >= MAX_USERS) {
        printf("Error: Maximum number of users reached.\n");
        return false;
    }

    for (int i = 0; i < user_count; i++) {
        if (strcmp(users[i].username, username) == 0) {
            printf("Error: Username already exists.\n");
            return false;
        }
    }

    strncpy(users[user_count].username, username, sizeof(users[user_count].username) - 1);
    strncpy(users[user_count].password, password, sizeof(users[user_count].password) - 1);
    users[user_count].protected = false;
    user_count++;

    save_users();
    return true;
}

bool remove_user(int index) {
    init_credentials();

    if (index < 0 || index >= user_count) {
        printf("Error: Invalid user index.\n");
        return false;
    }

    if (users[index].protected) {
        printf("Error: Cannot remove protected user.\n");
        return false;
    }

    for (int i = index; i < user_count - 1; i++) {
        users[i] = users[i + 1];
    }
    user_count--;

    save_users();
    return true;
}

void write_pass(const char* pass_string) {
    // Legacy function - updates first user's password
    init_credentials();
    if (user_count > 0) {
        strncpy(users[0].password, pass_string, sizeof(users[0].password) - 1);
        save_users();
    }
}

void set_new_pass(void) {
    char pass1[MAX_PASS_LEN] = {0};
    char pass2[MAX_PASS_LEN] = {0};

    printf("Enter new password: ");
    if (fgets(pass1, sizeof(pass1), stdin) == NULL) {
        printf("Error reading input.\n");
        return;
    }
    size_t len1 = strlen(pass1);
    if (len1 > 0 && pass1[len1 - 1] == '\n') {
        pass1[len1 - 1] = '\0';
    }

    printf("Confirm new password: ");
    if (fgets(pass2, sizeof(pass2), stdin) == NULL) {
        printf("Error reading input.\n");
        return;
    }
    size_t len2 = strlen(pass2);
    if (len2 > 0 && pass2[len2 - 1] == '\n') {
        pass2[len2 - 1] = '\0';
    }

    if (strcmp(pass1, pass2) != 0) {
        printf("Error: Passwords do not match.\n");
        return;
    }

    if (strlen(pass1) == 0) {
        printf("Error: Password cannot be empty.\n");
        return;
    }

    write_pass(pass1);
    printf("Password updated successfully.\n");
}

void add_new_user_interactive(void) {
    init_credentials();
    char username[64] = {0};
    char pass1[MAX_PASS_LEN] = {0};
    char pass2[MAX_PASS_LEN] = {0};

    printf("Enter new username: ");
    if (fgets(username, sizeof(username), stdin) == NULL) {
        printf("Error reading input.\n");
        return;
    }
    size_t len = strlen(username);
    if (len > 0 && username[len - 1] == '\n') {
        username[len - 1] = '\0';
    }

    if (strlen(username) == 0) {
        printf("Error: Username cannot be empty.\n");
        return;
    }

    printf("Enter password: ");
    if (fgets(pass1, sizeof(pass1), stdin) == NULL) {
        printf("Error reading input.\n");
        return;
    }
    len = strlen(pass1);
    if (len > 0 && pass1[len - 1] == '\n') {
        pass1[len - 1] = '\0';
    }

    printf("Confirm password: ");
    if (fgets(pass2, sizeof(pass2), stdin) == NULL) {
        printf("Error reading input.\n");
        return;
    }
    len = strlen(pass2);
    if (len > 0 && pass2[len - 1] == '\n') {
        pass2[len - 1] = '\0';
    }

    if (strcmp(pass1, pass2) != 0) {
        printf("Error: Passwords do not match.\n");
        return;
    }

    if (strlen(pass1) == 0) {
        printf("Error: Password cannot be empty.\n");
        return;
    }

    if (add_user(username, pass1)) {
        printf("User added successfully.\n");
    }
}

void remove_user_interactive(void) {
    init_credentials();
    list_users();
    printf("\nEnter user number to remove (1-%d): ", user_count);
    int choice;
    if (scanf("%d", &choice) != 1) {
        printf("Error: Invalid input.\n");
        while (getchar() != '\n');
        return;
    }
    while (getchar() != '\n');

    if (choice < 1 || choice > user_count) {
        printf("Error: Invalid choice.\n");
        return;
    }

    if (remove_user(choice - 1)) {
        printf("User removed successfully.\n");
    }
}

void pass_changelog(void) {
    FILE* file = fopen(CREDENTIALS_FILE, "r");
    if (!file) {
        printf("No credentials file found.\n");
        return;
    }

    char line[256];
    char changelog[256] = {0};

    while (fgets(line, sizeof(line), file)) {
        if (strncmp(line, "LAST_CHANGED:", 13) == 0) {
            size_t len = strlen(line);
            if (len > 0 && line[len - 1] == '\n') {
                line[len - 1] = '\0';
            }
            strcpy(changelog, line + 14);
        }
    }
    fclose(file);

    printf("-----------------------------------------------------------------\n");
    printf("|                  P A S S W O R D   L O G                      |\n");
    printf("-----------------------------------------------------------------\n\n");

    if (strlen(changelog) > 0) {
        printf("Password last changed: %s\n", changelog);
    } else {
        printf("No password change history found.\n");
    }
}