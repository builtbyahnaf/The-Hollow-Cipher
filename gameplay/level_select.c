#include <stdio.h>
#include "progress.h"
#include <stdlib.h>
#include <windows.h>
#include <conio.h>
#include "level_select.h"
#include "main.h"
#include "game_main.h"

void show_level_selection(void) {
    int selected_level;

    while (1) {
        load_progress();

        system("cls");
        printf("-----------------------------------------------------------------\n");
        printf("|               T H E   H O L L O W    C I P H E R               |\n");
        printf("-----------------------------------------------------------------\n\n");

        printf("GO BACK [0]\n\n");
        printf("LEVEL 1: THE SILENT PATIENT\n");

        printf("LEVEL 2: THE MISSING NOTEBOOK");
        if (!is_level_unlocked(1)) printf(" [LOCKED]");
        printf("\n");

        printf("LEVEL 3: THE HIDDEN DESTINATION");
        if (!is_level_unlocked(2)) printf(" [LOCKED]");
        printf("\n");

        printf("LEVEL 4: THE MAN WHO DEVELOPED THE DARK");
        if (!is_level_unlocked(3)) printf(" [LOCKED]");
        printf("\n");

        printf("LEVEL 5: THE HOLLOW CIPHER");
        if (!is_level_unlocked(4)) printf(" [LOCKED]");
        printf("\n\n");

        printf("ENTER SELECTION (0-5): ");
        if (scanf("%d", &selected_level) != 1) {
            int c;
            while ((c = getchar()) != '\n' && c != EOF);
            printf("Invalid input. Please enter a number between 0-5.\n");
            Sleep(1000);
            continue;
        }
        int c;
        while ((c = getchar()) != '\n' && c != EOF);

        if (selected_level < 0 || selected_level > 5) {
            printf("Enter between 0-5. Please try again.\n");
            Sleep(1000);
            continue;
        } else if (selected_level == 0) {
            system("cls");
            game_init();
            return; // Returns cleanly to game_init()
        } else if (selected_level != 1 && !is_level_unlocked(selected_level - 1)) {
            printf("This level is locked. Complete previous levels to unlock.\n");
            Sleep(1500);
            continue;
        } else {
            load_level(selected_level, true);
            // When user exits the level, return here and loop back to level select
        }
    }
}
