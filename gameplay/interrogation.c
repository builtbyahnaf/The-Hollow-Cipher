#include <stdio.h>
#include <conio.h>
#include <windows.h>
#include "interrogation.h"
#include "interrogation_parser.h"
#include "past_history.h"
#include "evidence_vault.h"
#include "game_main.h"
#include "color_palette.h"

void handle_input(int count);
void show_statement(int index);

static SuspectStatements *sus_sts;
static int level;

enum KEYS {
    PREFIX_A = 0,
    PREFIX_B = 224,
    ESCAPE = 27,
    UP_ARROW = 72,
    DOWN_ARROW = 80,
    ENTER = 13
};

// Display the suspect menu and return the selected array index.
void interrogation_menu_init(int _level) {
    level = _level;
    char interr_path[86];
    snprintf(interr_path, sizeof(interr_path), "data/level_%d/interrogation.txt", level);

    //Check validity to prevent crash
    FILE *fp = fopen(interr_path, "r");
    if (!fp) {
        printf("Error: Could not open interrogation file for level %d\n", level);
        return;
    }

    sus_sts = parse_interrogation(fp);

    handle_input(statements_count());
}

void handle_input(int count){
    int selected = 0;
    int key;

    if (sus_sts == NULL || count <= 0) {
        return;
    }

    while (1) {
        // Clear the screen and move the cursor to the top-left.
        system("cls");

        // Apply background and foreground colors from main.c
        printf(BG_LEATHER GOLD);

        printf("-----------------------------------------------------------------\n");
        printf("|             I N T E R R O G A T I O N   P A N E L               |\n");
        printf("-----------------------------------------------------------------\n\n");
        printf("Select a suspect:\n\n");

        // Display every suspect in the array.
        for (int i = 0; i < count; i++) {

            if (i == selected) {
                // Highlight selected suspect with gold foreground
                printf(BG_ACID_LIME CRIMSON " >  %s  " "\n", sus_sts[i].name);
            } else {
                // Normal suspects use ivory foreground
                printf(BG_LEATHER GOLD "    %s " "\n", sus_sts[i].name);
            }
            printf(BG_LEATHER GOLD);

            Sleep(50);
        }

        printf("\n"  "UP/DOWN: Navigate | ENTER: Select | ESC: Exit" "\n");
        

        // Wait for one keyboard input.
        key = _getch();

        // Escape exits the menu.
        if (key == ESCAPE) {
            system("cls");
            load_level(level, false);
        }

        // Arrow keys produce a prefix followed by a key code.
        if (key == PREFIX_A || key == PREFIX_B) {
            key = _getch();

            if (key == UP_ARROW) {
                // Up arrow: move to the previous suspect.
                selected--;

                // Wrap to the last suspect at the top.
                if (selected < 0) {
                    selected = count - 1;
                }
            }
            else if (key == DOWN_ARROW) {
                // Down arrow: move to the next suspect.
                selected++;

                // Wrap to the first suspect at the bottom.
                if (selected >= count) {
                    selected = 0;
                }
            }
            continue;
        }
        else if (key == ENTER) {
            // Enter: View the suspect's statement
            show_statement(selected);
        }
    }
}

void show_statement(int index){
    system("cls");

    
    printf("|             INTERROGATION: %s               |\n\n", sus_sts[index].name);
    
    printf("Statement:\n\n");
    printf("%s", sus_sts[index].statement);

    printf("\n\nPress ESC to go back");
    int key;

    while (1)
    {
        key = _getch();

        // Escape exits the menu.
        if (key == ESCAPE) {
            system("cls");
            break;
        }
    }
    handle_input(statements_count());
}
