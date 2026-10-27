#include <stdio.h>
#include <conio.h>
#include <windows.h>
#include "evidence.h"
#include "evidence_parser.h"
#include "game_main.h"
#include "color_palette.h"

void handle_ev_input(int count);
void show_evidence(int index);

static Evidence *ev_list;
static int level;

enum KEYS {
    PREFIX_A = 0,
    PREFIX_B = 224,
    ESCAPE = 27,
    UP_ARROW = 72,
    DOWN_ARROW = 80,
    ENTER = 13
};

void evidence_menu_init(int _level) {
    level = _level;
    char ev_path[86];
    snprintf(ev_path, sizeof(ev_path), "data/level_%d/evidence.txt", level);

    FILE *fp = fopen(ev_path, "r");
    if (!fp) {
        printf("Error: Could not open evidence file for level %d\n", level);
        return;
    }

    ev_list = parse_evidence(fp);

    handle_ev_input(evidence_count());
}

void handle_ev_input(int count) {
    int selected = 0;
    int key;

    if (ev_list == NULL || count <= 0) {
        return;
    }

    while (1) {
        system("cls");
        printf(BG_LEATHER GOLD);

        printf("-----------------------------------------------------------------\n");
        printf("|                 E V I D E N C E   P A N E L                   |\n");
        printf("-----------------------------------------------------------------\n\n");
        printf("Select evidence:\n\n");

        for (int i = 0; i < count; i++) {
            if (i == selected) {
                printf(BG_ACID_LIME CRIMSON " >  %s  " "\n", ev_list[i].name);
            } else {
                printf(BG_LEATHER GOLD "    %s " "\n", ev_list[i].name);
            }
            printf(BG_LEATHER GOLD);
            Sleep(50);
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
            show_evidence(selected);
        }
    }
}

void show_evidence(int index) {
    system("cls");

    printf("|             EVIDENCE: %s               |\n\n", ev_list[index].name);

    printf("Description:\n\n");
    printf("%s", ev_list[index].description);

    printf("\n\nPress ESC to go back");
    int key;

    while (1) {
        key = _getch();

        if (key == ESCAPE) {
            system("cls");
            break;
        }
    }
    handle_ev_input(evidence_count());
}