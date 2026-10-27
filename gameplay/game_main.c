#include <stdio.h>
//#include "progress.h"
#include <stdlib.h>
#include <windows.h>
#include <stdlib.h>
#include <conio.h>
#include "level_select.h"
#include "main.h"
#include "interrogation.h"
#include "evidence.h"
#include "past_history.h"
#include "evidence_vault.h"
#include "game_main.h"

void load_level(int level, bool view_suspense_and_anim) {
    if (view_suspense_and_anim) {
        suspense_view();
    }
    
    
    
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

    int char_count = 0;
    char ch;
    while ((ch = fgetc(fp)) != EOF) {
        putchar(ch);
        char_count++;
        if (view_suspense_and_anim && char_count <= 400) {
            Sleep(1);
        }
        
    }

    fclose(fp);
    handle_menu(level);
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
void handle_menu(int level){
    char selection;
    printf("\n \n");

    Sleep(100);

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
    scanf(" %c", &selection);

    if (selection < 'A' || selection > 'z') {
        printf("Invalid selection. Please try again.\n");
        goto RETRY;
    } else if (selection >= 'A' && selection <= 'Z') {
        selection = tolower(selection);
    }

    switch (selection) {
    case 'e':
        system("cls");
        
        show_vault_puzzle(level);
        handle_puzzle(get_puzzle_password());
        break;

    case 'i':
        system("cls");
        
        interrogation_menu_init(level);
        break;

    case 'h':
        system("cls");
        past_history_menu_init(level);
        break;

    case 'a':
        break;

    case 'x':
        _getch();
        system("cls");
        show_level_selection();
        break;

    default:
    printf("Invalid parameters!\n");
        break;
    }
}