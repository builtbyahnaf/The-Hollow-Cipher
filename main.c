#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <windows.h>
#include "level_select.h"
#include "admin_panel.h"
#include "color_palette.h"
#include "main.h"

int main(int argc, char *argv[]) {
    if (argc >= 3 && strcmp(argv[1], "admin_mode") == 0 && strcmp(argv[2], "admin123") == 0) {
        show_panel();
        return 0;
    }

    
    //Setting colors

    printf(BG_LEATHER GOLD);
    system("cls");

    game_init();
    
    return 0;
}
void game_init(){
    char selection;
    char title_text[] = "T H E   H O L L O W    C I P H E R";

    printf("-----------------------------------------------------------------\n");
    printf("|               ");

    //The typewriter animation
    int delay = 35;  // Typing speed in milliseconds

    // The animation lasts for title_text length x delay ms = 35 x 40 = 1400 ms which is equivalent to 1.4 sec

    for (int i = 0; title_text[i] != '\0'; i++) {
        printf("%c", title_text[i]);
        fflush(stdout);
        Sleep(delay);
    }

    printf("               |\n");

    printf("-----------------------------------------------------------------\n");

    printf("\n \n");

    Sleep(100);

    RETRY:

    for (int j = 0; j < 4; j++) {
        switch (j) {
        case 0:
            /* code */
            printf("PLAY [P]\n");
            fflush(stdout);
            Sleep(250);
            break;
        case 1:
            printf("OPTIONS [O]\n");
            fflush(stdout);
            Sleep(250);
            break;
        case 2:
            printf("ABOUT AND CREDITS [A]\n");
            fflush(stdout);
            Sleep(250);
            break;
        case 3:
            printf("EXIT [E]\n\n");
            fflush(stdout);
            Sleep(250);
            break;
        default:
            break;
        }
        
    }

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
        printf("\nAre you sure you want to exit? (Y/N): ");
        char confirm;
        scanf(" %c", &confirm);
        if (confirm == 'y' || confirm == 'Y') {
            system("cls");
            char farewell_text[] = "Thank you for playing THE HOLLOW CIPHER.\n";

        for (int i = 0; farewell_text[i] != '\0'; i++) {
            printf("%c", farewell_text[i]);
            fflush(stdout);
            Sleep(delay);
        }
            
            Sleep(1000);
            exit(0);
        }
        break;

    default:
    printf("Invalid parameters!\n");
        break;
    }


}