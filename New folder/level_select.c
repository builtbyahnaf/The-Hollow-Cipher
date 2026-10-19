#include <stdio.h>
//#include "progress.h"
#include "level_select.h"

void show_level_selection(){
    int selected_level;
    //Todo: add UI_framework.c if possible
    printf("-----------------------------------------------------------------\n");
    printf("|               T H E   H O L L O W    C I P H E R               |\n");
    printf("-----------------------------------------------------------------\n");

    printf("\n \n");

    RETRY:
    printf("GO BACK [B]\n\n");
    printf("LEVEL 1: THE SILENT PATIENT\n");
    printf("LEVEL 2: THE MISSING NOTEBOOK\n");
    printf("\n");
    

    printf("ENTER SELECTION (1-3): ");
    scanf("%d", &selected_level);

    if (selected_level < 1 || selected_level > 3 /*Replace with max levels*/) {
        printf("Enter between 1-3. Please try again.\n");
        goto RETRY;
    }

    /** To do: add passing selected level */
}