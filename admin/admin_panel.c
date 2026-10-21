#include <stdio.h>
#include "admin_panel.h"
#include <ctype.h>
#include <stdlib.h>
#include <cmd_edit.h>

void show_panel(){
    char selection;
    //Todo: add UI_framework.c later if possible
    
    
    printf("-----------------------------------------------------------------\n");
    printf("|               T H E   H O L L O W    C I P H E R               |\n");
    printf("-----------------------------------------------------------------\n");

    printf("\n \n");

    printf("                           ADMIN PANEL                          ");

    printf("\n \n");
    
    RETRY:
    printf("EDIT LEVELS [E]\n");
    printf("VIEW EDIT LOGS [L]\n");
    printf("CHANGE ADMIN PASSWORD [P]\n");
    printf("EXIT [X]\n\n");

    printf("ENTER SELECTION: ");
    scanf("%c", &selection);

    if (selection < 'A' || selection > 'z') {
        printf("Invalid selection. Please try again.\n");
        goto RETRY;
    } else if (selection >= 'A' && selection <= 'Z') {
        selection = tolower(selection);
    } else{

    }

    switch (selection)
    {
    case 'e':
        /* code */
        system("cls");
        select_level();
        break;

    case 'l':
        /* code */
        break;

    case 'p':
        /* code */
        break;
    
    case 'x':
        /* code */
        break;
    
    default:
        break;
    }
}