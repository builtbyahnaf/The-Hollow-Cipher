#include <stdio.h>
#include <conio.h>
#include <windows.h>
#include "interrogation.h"
#include "interrogation_parser.h"
#include "game_main.h"

#include "color_palette.h"

int handle_input(SuspectStatements *sus_sts, int count, int level);

// Display the suspect menu and return the selected array index.
// Returns -1 if the user presses Escape.
void interrogation_menu_init(int level) {
    

    // There is nothing to display if no suspects were loaded.
    //hide_scrollbar();

    //enable_ansi_colors();
    char interr_path[86];
    snprintf(interr_path, sizeof(interr_path), "data/level_%d/interrogation.txt", level);

    FILE *fp = fopen(interr_path, "r");
    if (!fp) {
        printf("Error: Could not open interrogation file for level %d\n", level);
        return;
    }

    SuspectStatements *sus_sts = parse_interrogation(fp);

    handle_input(sus_sts, statements_count(), level);

    
}

int handle_input(SuspectStatements *sus_sts, int count, int level){
    int selected = 0;
    int key;

    if (sus_sts == NULL || count <= 0) {
        return -1;
    }

    while (1) {
        // Clear the screen and move the cursor to the top-left.
        printf("\033[2J\033[H");

        // Apply background and foreground colors from main.c
        printf(BG_LEATHER GOLD);

        printf("====================================\n");
        printf("         THE HOLLOW CIPHER\n");
        printf("====================================\n\n");
        printf("Select a suspect:\n\n");

        // Display every suspect in the array.
        for (int i = 0; i < count; i++) {

            if (i == selected) {
                // Highlight selected suspect with gold foreground
                printf(GOLD " > %s " RESET "\n", sus_sts[i].name);
            } else {
                // Normal suspects use ivory foreground
                printf(IVORY "   %s" RESET "\n", sus_sts[i].name);
            }
        }

        printf("\n" IVORY "UP/DOWN: Navigate | ENTER: Select | ESC: Exit" RESET "\n");

        // Wait for one keyboard input.
        key = _getch();

        // Escape exits the menu.
        if (key == 27) {
            system("cls");
            load_level(level);
        }

        // Arrow keys produce a prefix followed by a key code.
        if (key == 0 || key == 224) {
            key = _getch();

            if (key == 72) {
                // Up arrow: move to the previous suspect.
                selected--;

                // Wrap to the last suspect at the top.
                if (selected < 0) {
                    selected = count - 1;
                }
            }
            else if (key == 80) {
                // Down arrow: move to the next suspect.
                selected++;

                // Wrap to the first suspect at the bottom.
                if (selected >= count) {
                    selected = 0;
                }
            }
            continue;
        }
        else if (key == 13) {
            // Enter: return the selected suspect's index.
            return selected;
        }
    }
}
