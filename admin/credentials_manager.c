/**
 * =============================================================================
 * PROJECT: The Hollow Cipher (THC)
 * FILE: admin/credentials_manager.c
 * PURPOSE: User Authentication and Credentials Administration
 * 
 * DESCRIPTION:
 *   This module manages user credentials and access permissions for the game's
 *   administrative functions. It handles:
 *     - Parsing and saving credentials to flat-file storage ("data/credentials.thcdb").
 *     - Authentication of username/password pairs and standalone passwords.
 *     - Protected user account policies (root system accounts cannot be deleted).
 *     - Interactive CLI workflows for adding users, removing users, and updating
 *       the administrator password.
 *     - Audit timestamp tracking of the last password/credential modification.
 * 
 * FILE FORMAT (data/credentials.thcdb):
 *   Username: <username> Password: <password>\n
 *   ...
 *   LAST_CHANGED: <day>, <Month>, <year>, <Weekday>, <time>\n
 * =============================================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdbool.h>

#include "credentials_manager.h"

/* Filepath to the credentials database */
#define CREDENTIALS_FILE "data/credentials.thcdb"

/* Maximum length allowed for a password string (including null-terminator) */
#define MAX_PASS_LEN 128

/* Maximum capacity of administrative users supported by the system */
#define MAX_USERS 50

/**
 * @struct User
 * @brief Represents an administrative user record.
 * 
 * Contains:
 *   - username: Null-terminated string representing the user handle.
 *   - password: Null-terminated string representing the user's password.
 *   - protected: Flag indicating whether this account is protected from deletion.
 *     The primary default accounts (first two loaded) are marked protected.
 */
typedef struct {
    char username[64];
    char password[MAX_PASS_LEN];
    bool protected;
} User;

/* In-memory array storing all loaded user accounts */
static User users[MAX_USERS];

/* Total number of active accounts currently loaded in memory */
static int user_count = 0;

/**
 * @brief Loads user credentials from the database file into memory.
 *
 * Reads "data/credentials.thcdb" line-by-line, parsing lines with the pattern:
 *   "Username: <user> Password: <pass>"
 *
 * Parsing details:
 *   - Strips trailing newlines ('\n') from each line.
 *   - Finds the "Username: " prefix (10 characters) to extract the username.
 *   - Finds the " Password: " delimiter (11 characters) to extract the password.
 *   - Null-terminates the username portion in-place before copying.
 *   - Bounded copying via strncpy prevents memory overruns.
 *   - The first two accounts (indices 0 and 1) are marked as 'protected' so they
 *     cannot be removed via the UI, safeguarding the system from admin lockout.
 */
static void load_users(void) {
    FILE* file = fopen(CREDENTIALS_FILE, "r");
    if (!file) {
        /* File does not exist yet or cannot be opened; return quietly */
        return;
    }

    user_count = 0;
    char line[256];

    while (fgets(line, sizeof(line), file) && user_count < MAX_USERS) {
        /* Strip trailing newline character */
        int len = strlen(line);
        if (len > 0 && line[len - 1] == '\n') {
            line[len - 1] = '\0';
        }

        /* Skip blank lines */
        if (strlen(line) == 0) continue;

        /* Locate username token prefix */
        char* user_start = strstr(line, "Username: ");
        if (!user_start) continue;
        user_start += 10; /* Advance past "Username: " */

        /* Locate password token delimiter */
        char* pass_start = strstr(user_start, " Password: ");
        if (!pass_start) continue;

        /*
         * Temporarily terminate user_start at the delimiter
         * so it contains only the username string.
         */
        *pass_start = '\0';
        pass_start += 11; /* Advance past " Password: " */

        /* Safely copy into active user struct */
        strncpy(users[user_count].username, user_start, sizeof(users[user_count].username) - 1);
        users[user_count].username[sizeof(users[user_count].username) - 1] = '\0';

        strncpy(users[user_count].password, pass_start, sizeof(users[user_count].password) - 1);
        users[user_count].password[sizeof(users[user_count].password) - 1] = '\0';

        /*
         * First two accounts are default root/system accounts and cannot be removed
         * by regular user management functions.
         */
        if (user_count < 2) {
            users[user_count].protected = true;
        } else {
            users[user_count].protected = false;
        }

        user_count++;
    }

    fclose(file);
}

/**
 * @brief Persists all active in-memory user records back to the credentials file.
 *
 * Overwrites "data/credentials.thcdb" with:
 *   1. All user accounts in "Username: ... Password: ..." format.
 *   2. An updated "LAST_CHANGED: <timestamp>" entry reflecting the current
 *      local time of modification.
 */
static void save_users(void) {
    FILE* file = fopen(CREDENTIALS_FILE, "w");
    if (!file) {
        printf("Error: Could not open credentials file for writing.\n");
        return;
    }

    /* Retrieve and format current system date and time */
    time_t now = time(NULL);
    struct tm* tm_info = localtime(&now);
    char timestamp[128];
    strftime(timestamp, sizeof(timestamp), "%d, %B, %Y, %A, %I:%M:%S %p", tm_info);

    /* Write each user record */
    for (int i = 0; i < user_count; i++) {
        fprintf(file, "Username: %s Password: %s\n", users[i].username, users[i].password);
    }

    /* Append last modified metadata timestamp */
    fprintf(file, "LAST_CHANGED: %s\n", timestamp);

    fclose(file);
}

/**
 * @brief Initializes the credentials cache by loading accounts from disk.
 *
 * Safe to invoke repeatedly; ensures the in-memory user list matches the disk file.
 */
void init_credentials(void) {
    load_users();
}

/**
 * @brief Validates if an entered password matches any known user password.
 *
 * @param pass_string The plaintext password to test.
 * @return true if a matching password exists in the database, false otherwise.
 */
bool is_pass_valid(char* pass_string) {
    init_credentials();
    for (int i = 0; i < user_count; i++) {
        if (strcmp(users[i].password, pass_string) == 0) {
            return true;
        }
    }
    return false;
}

/**
 * @brief Authenticates a specific username and password credential pair.
 *
 * @param username The account username to authenticate.
 * @param password The plaintext password to authenticate.
 * @return true if credentials are valid and match an account, false otherwise.
 */
bool is_user_valid(char* username, char* password) {
    init_credentials();
    for (int i = 0; i < user_count; i++) {
        if (strcmp(users[i].username, username) == 0 && strcmp(users[i].password, password) == 0) {
            return true;
        }
    }
    return false;
}

/**
 * @brief Displays a formatted list of all registered users to standard output.
 *
 * Appends a "[PROTECTED]" indicator next to root/system accounts that cannot be deleted.
 */
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

/**
 * @brief Registers a new user account and commits it to disk.
 *
 * Performs validation:
 *   - Checks if user capacity (MAX_USERS) has been reached.
 *   - Enforces unique usernames (prevents duplicate accounts).
 *
 * @param username New username handle.
 * @param password Plaintext password.
 * @return true on successful addition, false on failure (capacity or duplicate).
 */
bool add_user(char* username, char* password) {
    init_credentials();

    /* Verify capacity limit */
    if (user_count >= MAX_USERS) {
        printf("Error: Maximum number of users reached.\n");
        return false;
    }

    /* Enforce username uniqueness */
    for (int i = 0; i < user_count; i++) {
        if (strcmp(users[i].username, username) == 0) {
            printf("Error: Username already exists.\n");
            return false;
        }
    }

    /* Add new user entry */
    strncpy(users[user_count].username, username, sizeof(users[user_count].username) - 1);
    users[user_count].username[sizeof(users[user_count].username) - 1] = '\0';

    strncpy(users[user_count].password, password, sizeof(users[user_count].password) - 1);
    users[user_count].password[sizeof(users[user_count].password) - 1] = '\0';

    /* User-created accounts are not protected */
    users[user_count].protected = false;
    user_count++;

    /* Save modified list to disk */
    save_users();
    return true;
}

/**
 * @brief Removes a user account by its 0-based array index and commits to disk.
 *
 * Security:
 *   - Validates that index is within range [0, user_count - 1].
 *   - Prevents deletion of protected accounts (users[index].protected == true).
 *   - Shifts trailing elements left to preserve a contiguous array.
 *
 * @param index 0-based index of the user to delete.
 * @return true if deletion succeeded, false otherwise.
 */
bool remove_user(int index) {
    init_credentials();

    /* Bounds check */
    if (index < 0 || index >= user_count) {
        printf("Error: Invalid user index.\n");
        return false;
    }

    /* Guard protected accounts */
    if (users[index].protected) {
        printf("Error: Cannot remove protected user.\n");
        return false;
    }

    /* Shift subsequent user entries to fill the deleted slot */
    for (int i = index; i < user_count - 1; i++) {
        users[i] = users[i + 1];
    }
    user_count--;

    /* Persist updated list to disk */
    save_users();
    return true;
}

/**
 * @brief Legacy password update helper for the primary root administrator (index 0).
 *
 * @param pass_string The new password string for the root admin.
 */
void write_pass(const char* pass_string) {
    init_credentials();
    if (user_count > 0) {
        strncpy(users[0].password, pass_string, sizeof(users[0].password) - 1);
        users[0].password[sizeof(users[0].password) - 1] = '\0';
        save_users();
    }
}

/**
 * @brief Helper function to securely read a trimmed line of text from standard input.
 *
 * Flushes stdout before reading, utilizes fgets() to prevent buffer overflows,
 * and strips both newline ('\n') and carriage return ('\r') line endings.
 *
 * @param buffer Output destination buffer.
 * @param size Maximum buffer size.
 * @param prompt String displayed to user prior to reading.
 * @return true if a line was read successfully, false on error or EOF.
 */
static bool read_input_line(char* buffer, size_t size, const char* prompt) {
    printf("%s", prompt);
    fflush(stdout);

    if (fgets(buffer, (int)size, stdin) == NULL) {
        printf("Error reading input.\n");
        return false;
    }

    /* Strip trailing newline and carriage return characters */
    size_t len = strlen(buffer);
    while (len > 0 && (buffer[len - 1] == '\n' || buffer[len - 1] == '\r')) {
        buffer[--len] = '\0';
    }
    return true;
}

/**
 * @brief Interactive CLI workflow to change the root administrator's password.
 *
 * Prompts user to:
 *   1. Enter the new password.
 *   2. Re-enter the password for confirmation.
 * Verifies that the input is non-empty and both entries match before saving.
 */
void set_new_pass(void) {
    char pass1[MAX_PASS_LEN] = {0};
    char pass2[MAX_PASS_LEN] = {0};

    if (!read_input_line(pass1, sizeof(pass1), "Enter new password: ")) {
        return;
    }

    if (strlen(pass1) == 0) {
        printf("Error: Password cannot be empty.\n");
        return;
    }

    if (!read_input_line(pass2, sizeof(pass2), "Confirm new password: ")) {
        return;
    }

    if (strcmp(pass1, pass2) != 0) {
        printf("Error: Passwords do not match.\n");
        return;
    }

    /* Update root administrator password */
    write_pass(pass1);
    printf("Password updated successfully.\n");
}

/**
 * @brief Interactive CLI workflow to add a new user.
 *
 * Prompts for:
 *   - Username (must be non-empty and unique).
 *   - Password (must be non-empty).
 *   - Password confirmation (must match password).
 * If valid, creates the account and saves it to disk.
 */
void add_new_user_interactive(void) {
    init_credentials();
    char username[64] = {0};
    char pass1[MAX_PASS_LEN] = {0};
    char pass2[MAX_PASS_LEN] = {0};

    if (!read_input_line(username, sizeof(username), "Enter new username: ")) {
        return;
    }

    if (strlen(username) == 0) {
        printf("Error: Username cannot be empty.\n");
        return;
    }

    if (!read_input_line(pass1, sizeof(pass1), "Enter password: ")) {
        return;
    }

    if (strlen(pass1) == 0) {
        printf("Error: Password cannot be empty.\n");
        return;
    }

    if (!read_input_line(pass2, sizeof(pass2), "Confirm password: ")) {
        return;
    }

    if (strcmp(pass1, pass2) != 0) {
        printf("Error: Passwords do not match.\n");
        return;
    }

    if (add_user(username, pass1)) {
        printf("User added successfully.\n");
    }
}

/**
 * @brief Interactive CLI workflow to remove an existing user account.
 *
 * Displays the current user list, prompts for the 1-based account number,
 * validates user selection, checks protection status, and removes the user.
 */
void remove_user_interactive(void) {
    init_credentials();
    list_users();

    printf("\nEnter user number to remove (1-%d): ", user_count);
    int choice;
    if (scanf("%d", &choice) != 1) {
        printf("Error: Invalid input.\n");
        int c;
        while ((c = getchar()) != '\n' && c != EOF);
        return;
    }

    /* Flush stdin buffer after reading integer */
    int c;
    while ((c = getchar()) != '\n' && c != EOF);

    /* Validate user choice range */
    if (choice < 1 || choice > user_count) {
        printf("Error: Invalid choice.\n");
        return;
    }

    /* Convert 1-based selection to 0-based array index */
    if (remove_user(choice - 1)) {
        printf("User removed successfully.\n");
    }
}

/**
 * @brief Displays the timestamp when credentials were last updated.
 *
 * Reads "data/credentials.thcdb" and locates the line beginning with
 * "LAST_CHANGED:" to extract and display the audit timestamp.
 */
void pass_changelog(void) {
    FILE* file = fopen(CREDENTIALS_FILE, "r");
    if (!file) {
        printf("No credentials file found.\n");
        return;
    }

    char line[256];
    char changelog[256] = {0};

    /* Search for the LAST_CHANGED metadata tag */
    while (fgets(line, sizeof(line), file)) {
        if (strncmp(line, "LAST_CHANGED:", 13) == 0) {
            size_t len = strlen(line);
            if (len > 0 && line[len - 1] == '\n') {
                line[len - 1] = '\0';
            }
            /* Copy the substring following "LAST_CHANGED: " (14 chars) */
            strcpy(changelog, line + 14);
        }
    }
    fclose(file);

    /* Render password log header */
    printf("-----------------------------------------------------------------\n");
    printf("|                  P A S S W O R D   L O G                      |\n");
    printf("-----------------------------------------------------------------\n\n");

    if (strlen(changelog) > 0) {
        printf("Password last changed: %s\n", changelog);
    } else {
        printf("No password change history found.\n");
    }
}
