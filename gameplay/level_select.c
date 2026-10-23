#include <stdio.h>
//#include "progress.h"
#include <stdlib.h>
#include <conio.h>
#include "level_select.h"
#include "main.h"
#include "game_main.h"

void show_level_selection(){
    int selected_level;
    //Todo: add UI_framework.c if possible
    printf("-----------------------------------------------------------------\n");
    printf("|               T H E   H O L L O W    C I P H E R               |\n");
    printf("-----------------------------------------------------------------\n");

    printf("\n \n");

    RETRY:
    printf("GO BACK [0]\n\n");
    printf("LEVEL 1: THE SILENT PATIENT\n");
    printf("LEVEL 2: THE MISSING NOTEBOOK [NOT IMPLEMENTED YET]\n");
    printf("\n");
    

    printf("ENTER SELECTION (0-2): ");
    scanf("%d", &selected_level);

    if (selected_level < 0 || selected_level > 2 /*Replace with max levels*/) {
        printf("Enter between 0-2. Please try again.\n");
        goto RETRY;
    } else if (selected_level == 0) {
        _getch();
        system("cls");
        //going back to previous screen
        game_init();
    } else {
        load_level(selected_level, true);
    }
    

    /** To do: add passing selected level */
}