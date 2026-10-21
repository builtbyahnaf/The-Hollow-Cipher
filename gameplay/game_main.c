#include <stdio.h>
//#include "progress.h"
#include <stdlib.h>
#include "level_select.h"
#include "main.h"
#include <windows.h>
#include "game_main.h"

void load_level(int level) {
    suspense_view();
    system("cls");
    printf("-----------------------------------------------------------------\n");
    printf("|                         L E V E L : %d                         |\n", level);
    printf("-----------------------------------------------------------------\n");

    printf("\n \n");

    char path[86];
    snprintf(path, sizeof(path), "data/level_%d/statement.txt", level);

    FILE *fp = fopen(path, "r");
    if (!fp) {
        printf("Error: Could not open level %d statement file\n", level);
        return;
    }

    char ch;
    while ((ch = fgetc(fp)) != EOF) {
        putchar(ch);
    }

    fclose(fp);
    handle_menu();
}
void suspense_view(){
    system("cls");

    char suspense_text[] = "Embrace yourself before unraveling a dark mystery...";

    printf("-----------------------------------------------------------------\n");
    printf("|               T H E   H O L L O W    C I P H E R               |\n");
    printf("-----------------------------------------------------------------\n");

    printf("\n \n");

    //The typewriter animation
    int delay = 30;  // Typing speed in milliseconds

    // The animation lasts for title_text length x delay ms = 35 x 43 = 1590 ms which is equivalent to 1.6 sec

    for (int i = 0; suspense_text[i] != '\0'; i++) {
        printf("%c", suspense_text[i]);
        fflush(stdout);
        Sleep(delay);
    }
    Sleep(500);
    return;
}
void handle_menu(){
    char selection;
    printf("\n \n");

    Sleep(100);

    int unl = 0;

    RETRY:

    for (int j = 0; j < 5; j++) {
        switch (j) {
        case 0:
            /* code */
            printf("EVIDENCES [E]\n");
            fflush(stdout);
            Sleep(250);
            break;
        case 1:
            printf("INTERROGATIONS [I]\n");
            fflush(stdout);
            Sleep(250);
            break;
        case 2:
            printf("PAST HISTORIES [H]\n");
            fflush(stdout);
            Sleep(250);
            break;
        case 3:
            printf("ENTER YOUR ANSWER [A]\n");
            fflush(stdout);
            Sleep(250);
            break;
        case 4: 
            printf("EXIT [X]\n\n");
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
    case 'e':
        system("cls");
        unl++;
        break;

    case 'i':
        unl++;
        break;

    case 'h':
        unl++;
        break;

    case 'a':
        unl++;
        break;

    case 'x':
        unl++;
        break;

    default:
    printf("Invalid parameters!\n");
        break;
    }
}