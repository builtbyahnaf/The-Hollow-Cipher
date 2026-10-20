#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include "level_select.h"

int main(int argc, char *argv[]){

    char selection;
    //Todo: add UI_framework.c later if possible
    
    printf("-----------------------------------------------------------------\n");
    printf("|               T H E   H O L L O W    C I P H E R               |\n");
    printf("-----------------------------------------------------------------\n");

    printf("\n \n");

    RETRY:
    printf("PLAY [P]\n");
    printf("OPTIONS [O]\n");
    printf("ABOUT AND CREDITS [A]\n");
    printf("EXIT [E]\n\n");

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
    case 'p':
        /* code */
        system("cls");
        show_level_selection();
        break;

    case 'o':
        /* code */
        break;

    case 'a':
        /* code */
        break;
    
    case 'e':
        /* code */
        break;
    
    default:
        break;
    }
    return 0;
}