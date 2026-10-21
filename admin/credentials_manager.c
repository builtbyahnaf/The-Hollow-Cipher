#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdbool.h>
#include "credentials_manager.h"

#define CREDENTIALS_FILE "data/credentials.thcdb"
#define MAX_PASS_LEN 128

bool is_pass_valid(char* pass_string) {
    FILE* file = fopen(CREDENTIALS_FILE, "r");
    if (!file) {
        return false;
    }

    char stored_pass[MAX_PASS_LEN] = {0};
    if (fgets(stored_pass, sizeof(stored_pass), file)) {
        size_t len = strlen(stored_pass);
        if (len > 0 && stored_pass[len - 1] == '\n') {
            stored_pass[len - 1] = '\0';
        }
    }
    fclose(file);

    return strcmp(stored_pass, pass_string) == 0;
}

void write_pass(const char* pass_string) {
    FILE* file = fopen(CREDENTIALS_FILE, "w");
    if (!file) {
        printf("Error: Could not open credentials file for writing.\n");
        return;
    }

    time_t now = time(NULL);
    struct tm* tm_info = localtime(&now);

    char timestamp[128];
    strftime(timestamp, sizeof(timestamp), "%d, %B, %Y, %A, %I:%M:%S %p", tm_info);

    fprintf(file, "%s\n", pass_string);
    fprintf(file, "LAST_CHANGED: %s\n", timestamp);

    fclose(file);
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

void pass_changelog(void) {
    FILE* file = fopen(CREDENTIALS_FILE, "r");
    if (!file) {
        printf("No credentials file found.\n");
        return;
    }

    char line[256];
    char pass[MAX_PASS_LEN] = {0};
    char changelog[256] = {0};

    if (fgets(line, sizeof(line), file)) {
        size_t len = strlen(line);
        if (len > 0 && line[len - 1] == '\n') {
            line[len - 1] = '\0';
        }
        strcpy(pass, line);
    }

    if (fgets(line, sizeof(line), file)) {
        size_t len = strlen(line);
        if (len > 0 && line[len - 1] == '\n') {
            line[len - 1] = '\0';
        }
        strcpy(changelog, line);
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