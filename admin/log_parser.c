/**
 * =============================================================================
 * PROJECT: The Hollow Cipher (THC)
 * FILE: admin/log_parser.c
 * PURPOSE: Audit Log Parser and Viewer
 * 
 * DESCRIPTION:
 *   This module is responsible for reading and presenting the administrative
 *   edit logs stored in "data/edit_log.thcdb".
 * 
 *   Whenever an administrator modifies game data (such as evidence, interrogation,
 *   history, or statements) through the built-in level editor, a timestamped
 *   entry is prepended to the log database. This module reads and formats those
 *   entries so admins can inspect modifications and track maintenance activity.
 * =============================================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <conio.h>

#include "log_parser.h"

/**
 * @brief Reads and displays the administrative edit logs to the console.
 *
 * Workflow:
 *   1. Attempts to open "data/edit_log.thcdb" in read mode ("r").
 *   2. If the file is missing or inaccessible, alerts the user and waits
 *      for a return key (ESC, Enter, or 'B'/'b') before exiting.
 *   3. If opened successfully, renders the table header and iterates through
 *      the file line by line using fgets().
 *   4. Strips trailing newline ('\n') characters to ensure clean formatting.
 *   5. Skips empty lines.
 *   6. Displays numbered entries ([1], [2], ...) in reverse-chronological order
 *      (as saved by cmd_edit:write_log).
 *   7. Closes the file handle to prevent resource leaks.
 *   8. Halts until the administrator presses ESC (ASCII 27), Enter (ASCII 13),
 *      or 'B'/'b' to return cleanly to the admin dashboard.
 */
void view_edit_logs(void) {
    /* Attempt to open the log database file for reading */
    FILE* log_file = fopen("data/edit_log.thcdb", "r");
    if (!log_file) {
        printf("No edit logs found.\n");
        printf("\nPress ESC or Enter to go back...");

        /*
         * Wait for user acknowledgement via specific keypresses:
         *   - ASCII 27: ESC
         *   - ASCII 13: Carriage Return / Enter
         *   - 'b' / 'B': Back key
         */
        while (1) {
            int key = _getch();
            if (key == 27 || key == 13 || key == 'b' || key == 'B') {
                break;
            }
        }
        system("cls");
        return;
    }

    /* Print header banner */
    printf("-----------------------------------------------------------------\n");
    printf("|                     E D I T   L O G S                         |\n");
    printf("-----------------------------------------------------------------\n\n");

    char line[1024];
    int count = 0;

    /*
     * Read log entries line-by-line until End-Of-File (EOF).
     * Buffer size of 1024 is sufficient for filepath + timestamp descriptions.
     */
    while (fgets(line, sizeof(line), log_file)) {
        /*
         * Strip trailing newline ('\n') character if present to prevent
         * unintended blank lines during console output.
         */
        int len = strlen(line);
        if (len > 0 && line[len - 1] == '\n') {
            line[len - 1] = '\0';
        }

        /* Skip empty or blank lines */
        if (strlen(line) == 0) {
            continue;
        }

        /* Increment entry counter and display formatted log record */
        count++;
        printf("[%d] %s\n", count, line);
    }

    /* If file exists but had no valid non-empty log entries */
    if (count == 0) {
        printf("No edit logs found.\n");
    }

    printf("\nTotal entries: %d\n", count);

    /* Close the file stream */
    fclose(log_file);

    /*
     * Navigation pause loop: Keep screen visible until user explicitly
     * signals an intent to navigate back.
     */
    printf("\nPress ESC or Enter to go back...");
    while (1) {
        int key = _getch();
        if (key == 27 || key == 13 || key == 'b' || key == 'B') {
            break;
        }
    }

    /* Clear screen before returning to the caller */
    system("cls");
}
