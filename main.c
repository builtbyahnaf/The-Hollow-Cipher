#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include "level_select.h"
#include "admin_panel.h"

int main(int argc, char *argv[]) {
    if (argc >= 3 && strcmp(argv[1], "admin_mode") == 0 && strcmp(argv[2], "admin123") == 0) {
        show_panel();
        return 0;
    } else {
        printf("Invalid parameters!\n");
    }

    char selection;

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
    }

    switch (selection) {
    case 'p':
        system("cls");
        show_level_selection();
        break;

    case 'o':
        break;

    case 'a':
        break;

    case 'e':
        break;

    default:
    printf("Invalid parameters!\n");
        break;
    }
    
    return 0;
}