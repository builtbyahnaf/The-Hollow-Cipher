#include <stdio.h>
#include "progress.h"
#include <stdlib.h>
#include <windows.h>
#include <conio.h>
#include "level_select.h"
#include "main.h"
#include "game_main.h"

void show_level_selection(){
    int selected_level;

    load_progress();

    printf("-----------------------------------------------------------------\n");
    printf("|               T H E   H O L L O W    C I P H E R               |\n");
    printf("-----------------------------------------------------------------\n");

    printf("\n \n");

    RETRY:
    printf("GO BACK [0]\n\n");
    printf("LEVEL 1: THE SILENT PATIENT");
    if (!is_level_unlocked(0)) printf(" [LOCKED]");
    printf("\n");

    printf("LEVEL 2: THE MISSING NOTEBOOK");
    if (!is_level_unlocked(1)) printf(" [LOCKED]");
    printf("\n\n");
    

    printf("ENTER SELECTION (0-2): ");
    scanf("%d", &selected_level);

    if (selected_level < 0 || selected_level > 2) {
        printf("Enter between 0-2. Please try again.\n");
        goto RETRY;
    } else if (selected_level == 0) {
        _getch();
        system("cls");
        game_init();
    } else if (!is_level_unlocked(selected_level - 1)) {
        printf("This level is locked. Complete previous levels to unlock.\n");
        Sleep(1500);
        system("cls");
        goto RETRY;
    } else {
        load_level(selected_level, true);
    }
    

    /** To do: add passing selected level */
}