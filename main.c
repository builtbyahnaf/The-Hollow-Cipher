#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <windows.h>
#include <conio.h>
#include "level_select.h"
#include "admin_panel.h"
#include "color_palette.h"
#include "main.h"
#include "credentials_manager.h"
#include "progress.h"
#include "parser_utils.h"

extern void load_progress(void);
extern int is_level_unlocked(int level_index);
extern void set_level_unlocked(int level_index);
extern void reset_progress(void);

void options_menu(void);
void reset_progress_menu(void);
int get_typewriter_delay(void);
void theme_menu(void);

int current_theme = 0;

int main(int argc, char *argv[]) {
    if (argc >= 3) {
        init_credentials();
        if (is_user_valid(argv[1], argv[2])) {
            show_panel();
            return 0;
        } else {
            printf("Invalid credentials.\n");
            return 1;
        }
    }

    // Setting colors
    printf(BG_LEATHER GOLD);
    system("cls");

    game_init();
    
    gc_collect();
    return 0;
}

void game_init(void) {
    char selection;
    char title_text[] = "T H E   H O L L O W    C I P H E R";

    printf("-----------------------------------------------------------------\n");
    printf("|               ");

    int delay = get_typewriter_delay();

    for (int i = 0; title_text[i] != '\0'; i++) {
        printf("%c", title_text[i]);
        fflush(stdout);
        Sleep(delay);
    }

    printf("               |\n");
    printf("-----------------------------------------------------------------\n\n");
    Sleep(100);

    while (1) {
        Sleep(200);
        printf("PLAY [P]\n");
        Sleep(200);
        printf("OPTIONS [O]\n");
        Sleep(200);
        printf("ABOUT AND CREDITS [A]\n");
        Sleep(200);
        printf("EXIT [E]\n\n");
        Sleep(200);

        printf("ENTER SELECTION: ");
        if (scanf(" %c", &selection) != 1) {
            int c;
            while ((c = getchar()) != '\n' && c != EOF);
            continue;
        }
        int c;
        while ((c = getchar()) != '\n' && c != EOF);

        selection = tolower(selection);

        switch (selection) {
        case 'p':
            system("cls");
            show_level_selection();
            system("cls");
            break;

        case 'o':
            system("cls");
            options_menu();
            system("cls");
            break;

        case 'a':
            system("cls");
            char version_info[] = "THE HOLLOW CIPHER version 1.0.0-stable\n\n";
            char about_section[] = "The Hollow Cipher is an interactive detective and mystery-solving game developed using the C programming language for KUET CSE 1-1 Project. The game places the player in the role of an investigator who must study the available evidence, question suspects, and connect conflicting pieces of information to uncover the truth.\n"
                                    "The project is built around a narrative-driven investigation. Instead of relying solely on action or random gameplay, it encourages logical reasoning, attention to detail, and critical analysis. The game flow is story-driven, where each level acts as a step towards the final dramatic conclusion.\n\n";
            char credits_section[] = "================== C R E D I T S ====================\n\n"
                                    "Muhammad Tahmid Ahnaf : Project architecture and planning, overall game experience and quality, administrative actions, vault locking and unlocking mechanisms, animations, themes, consecutive story building, integration of gameplay systems, debugging amd overall performance optimization.\n"
                                    "Rubiya Rahman Mela : Assisted in file parsing logic, vault password generation and solution validation.";
            for (int i = 0; version_info[i] != '\0'; i++) {
                printf("%c", version_info[i]);
                fflush(stdout);
                Sleep(5);
            }
            printf("%s", about_section);
            for (int i = 0; credits_section[i] != '\0'; i++) {
                printf("%c", credits_section[i]);
                fflush(stdout);
                Sleep(1);
            }
            printf("\n\nPress any key to go back...");
            _getch();
            system("cls");
            break;

        case 'e':
            printf("\nAre you sure you want to exit? (Y/N): ");
            char confirm;
            if (scanf(" %c", &confirm) == 1) {
                while ((c = getchar()) != '\n' && c != EOF);
                if (confirm == 'y' || confirm == 'Y') {
                    system("cls");
                    char farewell_text[] = "Thank you for playing THE HOLLOW CIPHER.\n";
                    for (int i = 0; farewell_text[i] != '\0'; i++) {
                        printf("%c", farewell_text[i]);
                        fflush(stdout);
                        Sleep(delay);
                    }
                    Sleep(500);
                    gc_collect();
                    exit(0);
                    return;
                }
            } else {
                while ((c = getchar()) != '\n' && c != EOF);
            }
            system("cls");
            break;

        default:
            printf("Invalid parameters! Please try again.\n");
            Sleep(1000);
            system("cls");
            break;
        }
    }
}

void options_menu(void) {
    char selection;
    load_progress();

    while (1) {
        system("cls");
        printf(BG_LEATHER GOLD);
        printf("-----------------------------------------------------------------\n");
        printf("|                       O P T I O N S                            |\n");
        printf("-----------------------------------------------------------------\n\n");

        printf("RESET PROGRESS [R]\n");
        //printf("THEME [T]\n");
        printf("GO BACK [B]\n\n");

        printf("ENTER SELECTION: ");
        if (scanf(" %c", &selection) != 1) {
            int c;
            while ((c = getchar()) != '\n' && c != EOF);
            continue;
        }
        int c;
        while ((c = getchar()) != '\n' && c != EOF);

        selection = tolower(selection);

        switch (selection) {
        case 'r':
            system("cls");
            printf("\nAre you sure you want to reset all progress? (Y/N): ");
            char confirm;
            if (scanf(" %c", &confirm) == 1) {
                while ((c = getchar()) != '\n' && c != EOF);
                if (confirm == 'y' || confirm == 'Y') {
                    reset_progress();
                    Sleep(1000);
                    system("cls");
                    game_init();
                }
            } else {
                while ((c = getchar()) != '\n' && c != EOF);
            }
            break;
        /*
        case 't':
            theme_menu();
            break;
            */

        case 'b':
            system("cls");
            return;

        default:
            printf("Invalid selection.\n");
            Sleep(1000);
            break;
        }
    }
}

void reset_progress_menu(void) {
    options_menu();
}

void theme_menu(void) {
    while (1) {
        system("cls");
        printf(BG_LEATHER GOLD);
        printf("-----------------------------------------------------------------\n");
        printf("|                         T H E M E S                            |\n");
        printf("-----------------------------------------------------------------\n\n");

        printf("Current theme: %d\n\n", current_theme + 1);
        printf("CLASSIC [1] - Leather & Gold (Default)\n");
        printf("DARK VOID [2] - Void & Ivory\n");
        printf("CRIMSON NIGHT [3] - Oxblood & Crimson\n");
        printf("PATINA GREEN [4] - Patina & Mint\n");
        printf("STONE GRAY [5] - Stone & Ash\n");
        printf("GO BACK [B]\n\n");

        printf("ENTER SELECTION: ");
        char selection;
        if (scanf(" %c", &selection) != 1) {
            int c;
            while ((c = getchar()) != '\n' && c != EOF);
            continue;
        }
        int c;
        while ((c = getchar()) != '\n' && c != EOF);

        selection = tolower(selection);

        switch (selection) {
        case '1':
            current_theme = 0;
            printf("\nTheme set to CLASSIC\n");
            Sleep(1000);
            break;
        case '2':
            current_theme = 1;
            printf("\nTheme set to DARK VOID\n");
            Sleep(1000);
            break;
        case '3':
            current_theme = 2;
            printf("\nTheme set to CRIMSON NIGHT\n");
            Sleep(1000);
            break;
        case '4':
            current_theme = 3;
            printf("\nTheme set to PATINA GREEN\n");
            Sleep(1000);
            break;
        case '5':
            current_theme = 4;
            printf("\nTheme set to STONE GRAY\n");
            Sleep(1000);
            break;
        case 'b':
            return;
        default:
            printf("\nInvalid selection.\n");
            Sleep(1000);
            break;
        }
    }
}
