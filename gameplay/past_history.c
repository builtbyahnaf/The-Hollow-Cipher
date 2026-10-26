#include <stdio.h>
#include <conio.h>
#include <windows.h>
#include "past_history.h"
#include "past_history_parser.h"
#include "game_main.h"
#include "color_palette.h"

void handle_ph_input(int count);
void show_history(int index);

static PastHistory *ph_list;
static int level;

enum KEYS {
    PREFIX_A = 0,
    PREFIX_B = 224,
    ESCAPE = 27,
    UP_ARROW = 72,
    DOWN_ARROW = 80,
    ENTER = 13
};

void past_history_menu_init(int _level) {
    level = _level;
    char ph_path[86];
    snprintf(ph_path, sizeof(ph_path), "data/level_%d/past_history.txt", level);

    FILE *fp = fopen(ph_path, "r");
    if (!fp) {
        printf("Error: Could not open past history file for level %d\n", level);
        return;
    }

    ph_list = parse_past_history(fp);

    handle_ph_input(past_history_count());
}

void handle_ph_input(int count) {
    int selected = 0;
    int key;

    if (ph_list == NULL || count <= 0) {
        return;
    }

    while (1) {
        system("cls");
        printf(BG_LEATHER GOLD);

        printf("-----------------------------------------------------------------\n");
        printf("|                P A S T   H I S T O R Y   P A N E L              |\n");
        printf("-----------------------------------------------------------------\n\n");
        printf("Select a person:\n\n");

        for (int i = 0; i < count; i++) {
            if (i == selected) {
                printf(BG_ACID_LIME CRIMSON " >  %s  " "\n", ph_list[i].name);
            } else {
                printf(BG_LEATHER GOLD "    %s " "\n", ph_list[i].name);
            }
            printf(BG_LEATHER GOLD);
            Sleep(180);
        }

        printf("\n" "UP/DOWN: Navigate | ENTER: Select | ESC: Exit" "\n");

        key = _getch();

        if (key == ESCAPE) {
            system("cls");
            load_level(level, false);
        }

        if (key == PREFIX_A || key == PREFIX_B) {
            key = _getch();

            if (key == UP_ARROW) {
                selected--;
                if (selected < 0) {
                    selected = count - 1;
                }
            }
            else if (key == DOWN_ARROW) {
                selected++;
                if (selected >= count) {
                    selected = 0;
                }
            }
            continue;
        }
        else if (key == ENTER) {
            show_history(selected);
        }
    }
}

void show_history(int index) {
    system("cls");

    printf("|             PAST HISTORY: %s               |\n\n", ph_list[index].name);

    printf("History:\n\n");
    printf("%s", ph_list[index].history);

    printf("\n\nPress ESC to go back");
    int key;

    while (1) {
        key = _getch();

        if (key == ESCAPE) {
            system("cls");
            break;
        }
    }
    handle_ph_input(past_history_count());
}