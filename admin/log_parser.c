#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <conio.h>
#include "log_parser.h"

void view_edit_logs(void) {
    FILE* log_file = fopen("data/edit_log.thcdb", "r");
    if (!log_file) {
        printf("No edit logs found.\n");
        printf("\nPress ESC or Enter to go back...");
        while (1) {
            int key = _getch();
            if (key == 27 || key == 13 || key == 'b' || key == 'B') {
                break;
            }
        }
        system("cls");
        return;
    }

    printf("-----------------------------------------------------------------\n");
    printf("|                     E D I T   L O G S                         |\n");
    printf("-----------------------------------------------------------------\n\n");

    char line[1024];
    int count = 0;

    while (fgets(line, sizeof(line), log_file)) {
        int len = strlen(line);
        if (len > 0 && line[len - 1] == '\n') {
            line[len - 1] = '\0';
        }

        if (strlen(line) == 0) {
            continue;
        }

        count++;
        printf("[%d] %s\n", count, line);
    }

    if (count == 0) {
        printf("No edit logs found.\n");
    }

    printf("\nTotal entries: %d\n", count);
    fclose(log_file);

    printf("\nPress ESC or Enter to go back...");
    while (1) {
        int key = _getch();
        if (key == 27 || key == 13 || key == 'b' || key == 'B') {
            break;
        }
    }
    system("cls");
}
