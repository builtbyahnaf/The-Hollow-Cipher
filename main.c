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

extern void load_progress(void);
extern int is_level_unlocked(int level_index);
extern void set_level_unlocked(int level_index);
extern void reset_progress(void);

void options_menu(void);
void reset_progress_menu(void);

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

    for (int j = 0; j < 5; j++) {
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
            printf("RESET PROGRESS [R]\n");
            fflush(stdout);
            Sleep(250);
            break;
        case 4:
            printf("EXIT [E]\n\n");
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
    case 'p':
        system("cls");
        show_level_selection();
        break;

    case 'o':
        system("cls");
        options_menu();
        break;

    case 'a':
    system("cls");
    char version_info[] = "THE HOLLOW CIPHER version 1.0.0-stable\n\n";
    char about_section[] = "The Hollow Cipher is an interactive detective and mystery-solving game developed using the C programming language for KUET CSE 1-1 Project. The game places the player in the role of an investigator who must study the available evidence, question suspects, and connect conflicting pieces of information to uncover the truth.\n"
                            "The project is built around a narrative-driven investigation. Instead of relying solely on action or random gameplay, it encourages logical reasoning, attention to detail, and critical analysis. The game flow is story-driven, where each level acts as a step towards the final dramatic conclusion.\n\n";
    char credits_section[] = "================== C R E D I T S ====================\n\n"
                            "Muhammad Tahmid Ahnaf : Project architecture and planning, overall game experience and quality, administrative actions, vault locking and unlocking mechanisms, animations, consecutive story building, integration of gameplay systems and debugging.\n"
                            "Rubiya Rahman Mela : Assisted in integration of gameplay systems, file parsing logic, vault password generation, solution validation and beta testing.";
    for (int i = 0; version_info[i] != '\0'; i++) {
            printf("%c", version_info[i]);
            fflush(stdout);
            Sleep(10);
        }
    printf("%s", about_section);
    for (int i = 0; credits_section[i] != '\0'; i++) {
            printf("%c", credits_section[i]);
            fflush(stdout);
            Sleep(1);
        }
    printf("\n\nPress ESC to go back...");
    int key = _getch();

        if (key == 27) {
            system("cls");
            game_init();
        }
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

    case 'r':
        system("cls");
        reset_progress_menu();
        break;

    default:
    printf("Invalid parameters!\n");
        break;
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
        printf("GO BACK [B]\n\n");

        printf("ENTER SELECTION: ");
        scanf(" %c", &selection);

        if (selection >= 'A' && selection <= 'Z') {
            selection = tolower(selection);
        }

        switch (selection) {
        case 'r':
            system("cls");
            printf("\nAre you sure you want to reset all progress? (Y/N): ");
            char confirm;
            scanf(" %c", &confirm);
            if (confirm == 'y' || confirm == 'Y') {
                reset_progress();
                Sleep(1500);
            }
            break;

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