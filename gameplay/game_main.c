#include <stdio.h>
#include <string.h>
#include "progress.h"
#include <stdlib.h>
#include <windows.h>
#include <conio.h>
#include <ctype.h>
#include "level_select.h"
#include "main.h"
#include "interrogation.h"
#include "evidence.h"
#include "past_history.h"
#include "evidence_vault.h"
#include "game_main.h"
#include "color_palette.h"
#include "ans_validator.h"
#include "parser_utils.h"

extern int get_typewriter_delay(void);
extern void show_level_selection(void);
void submit_answer(int level);

void load_level(int level, bool view_suspense_and_anim) {
    if (view_suspense_and_anim) {
        suspense_view();
    }
    
    system("cls");
    printf("-----------------------------------------------------------------\n");
    printf("|                         L E V E L : %d                         |\n", level);
    printf("-----------------------------------------------------------------\n\n");

    char path[86];
    snprintf(path, sizeof(path), "data/level_%d/statement.txt", level);

    FILE *fp = fopen(path, "r");
    if (!fp) {
        printf("Error: Could not open level %d statement file\n", level);
        Sleep(1500);
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
    return;
    //gc_collect();
    //system("cls");
}

void suspense_view(void) {
    system("cls");

    char suspense_text[] = "Embrace yourself before unraveling a dark mystery...";

    printf("-----------------------------------------------------------------\n");
    printf("|               T H E   H O L L O W    C I P H E R               |\n");
    printf("-----------------------------------------------------------------\n\n");

    int delay = get_typewriter_delay();

    for (int i = 0; suspense_text[i] != '\0'; i++) {
        printf("%c", suspense_text[i]);
        fflush(stdout);
        Sleep(delay);
    }
    Sleep(500);
}

void handle_menu(int level) {
    char selection;

    while (1) {
        printf("\n\n");
        Sleep(200);
        printf("EVIDENCES [E]\n");
        Sleep(200);
        printf("INTERROGATIONS [I]\n");
        Sleep(200);
        printf("PAST HISTORIES [H]\n");
        Sleep(200);
        printf("ENTER YOUR ANSWER [A]\n");
        Sleep(200);
        printf("EXIT [X]\n\n");
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
        case 'e':
            system("cls");
            if (is_vault_unlocked()) {
                evidence_menu_init(level);
            } else {
                show_vault_puzzle(level);
            }
            break;

        case 'i':
            system("cls");
            interrogation_menu_init(level);
            system("cls");
            break;

        case 'h':
            system("cls");
            past_history_menu_init(level);
            system("cls");
            break;

        case 'a':
            system("cls");
            submit_answer(level);
            system("cls");
            break;

        case 'x':
            printf("\nAre you sure you want to exit to level selection? (Y/N): ");
            char confirm;
            if (scanf(" %c", &confirm) == 1) {
                while ((c = getchar()) != '\n' && c != EOF);
                if (confirm == 'y' || confirm == 'Y') {
                    system("cls");
                    gc_collect();
                    show_level_selection();
                    return; // Cleanly returns to load_level, which returns to show_level_selection
                }
            } else {
                while ((c = getchar()) != '\n' && c != EOF);
            }
            system("cls");
            break;

        default:
            printf("Invalid selection. Please try again.\n");
            Sleep(1000);
            system("cls");
            break;
        }
    }
}

void submit_answer(int level) {
    char response[256];
    printf("-----------------------------------------------------------------\n");
    printf("|                    S U B M I T   A N S W E R                   |\n");
    printf("-----------------------------------------------------------------\n\n");
    printf("Enter your answer:\n\n");
    
    if (fgets(response, sizeof(response), stdin) == NULL) {
        printf("Error reading input.\n");
        Sleep(1500);
        return;
    }
    
    response[strcspn(response, "\r\n")] = '\0';
    
    if (strlen(response) == 0) {
        printf("\nAnswer cannot be empty.\n");
        Sleep(1500);
        return;
    }
    
    char sol_path[86];
    snprintf(sol_path, sizeof(sol_path), "data/level_%d/solution.txt", level);
    
    FILE *sol_fp = fopen(sol_path, "r");
    if (!sol_fp) {
        printf("\nError: Could not open solution file.\n");
        Sleep(1500);
        return;
    }
    
    printf("\nChecking your answer...\n");
    Sleep(1000);
    
    if (is_ans_correct(response, sol_fp)) {
        fclose(sol_fp);
        sol_fp = NULL;

        int delay = get_typewriter_delay();

        char correct_text[] = "\n>>> CORRECT! <<<\n";
        printf(GREEN);
        for (int i = 0; correct_text[i] != '\0'; i++) {
            printf("%c", correct_text[i]);
            fflush(stdout);
            Sleep(delay);
        }
        Sleep(300);
        printf(BG_LEATHER GOLD);
        printf("\nWell done, detective. You've solved the case.\n");
        
        // Unlock next level
        if (level < MAX_LEVELS) {
            set_level_unlocked(level);
            printf("\nLevel %d unlocked!\n", level + 1);
        } else {
            printf("\nYou have completed all available levels!\n");
        }
        
        // Show explanation with typewriter animation
        sol_fp = fopen(sol_path, "r");
        if (sol_fp) {
            char *explanation = parse_solution_desc(sol_fp);
            fclose(sol_fp);

            if (explanation) {
                printf("\n--- EXPLANATION ---\n\n");

                char *exp_text = strstr(explanation, "[Explanation]");
                if (exp_text) {
                    exp_text += 13;
                    while (*exp_text == '\r' || *exp_text == '\n') {
                        exp_text++;
                    }
                } else {
                    exp_text = explanation;
                }

                for (int i = 0; exp_text[i] != '\0'; i++) {
                    printf("%c", exp_text[i]);
                    fflush(stdout);
                    Sleep(3);
                }
                printf("\n");

                free(explanation);
            }
        }
    } else {
        if (sol_fp) {
            fclose(sol_fp);
        }
        printf(RED "\n>>> INCORRECT <<<\n" GOLD);
        printf("\nThat's not the right answer. Keep investigating...\n");
    }
    
    printf("\nPress any key to continue...");
    _getch();
    show_level_selection();
}

static int typewriter_delay = 30; // Default delay in ms

void text_speed_menu(void) {
    while (1) {
        system("cls");
        printf(BG_LEATHER GOLD);
        printf("-----------------------------------------------------------------\n");
        printf("|                      T E X T   S P E E D                       |\n");
        printf("-----------------------------------------------------------------\n\n");
        
        printf("Current speed: %d ms per character\n\n", typewriter_delay);
        printf("FASTER [F] - Decrease delay\n");
        printf("SLOWER [S] - Increase delay\n");
        printf("DEFAULT [D] - Reset to 30ms\n");
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
        case 'f':
            if (typewriter_delay > 5) {
                typewriter_delay -= 5;
                printf("\nSpeed increased! New delay: %dms\n", typewriter_delay);
            } else {
                printf("\nAlready at maximum speed!\n");
            }
            Sleep(1000);
            break;
        case 's':
            if (typewriter_delay < 200) {
                typewriter_delay += 5;
                printf("\nSpeed decreased! New delay: %dms\n", typewriter_delay);
            } else {
                printf("\nAlready at minimum speed!\n");
            }
            Sleep(1000);
            break;
        case 'd':
            typewriter_delay = 30;
            printf("\nReset to default (30ms)\n");
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

int get_typewriter_delay(void) {
    return typewriter_delay;
}

void statistics_menu(void) {
    system("cls");
    printf(BG_LEATHER GOLD);
    printf("-----------------------------------------------------------------\n");
    printf("|                      S T A T I S T I C S                       |\n");
    printf("-----------------------------------------------------------------\n\n");
    
    load_progress();
    
    int unlocked_count = 0;
    for (int i = 0; i < MAX_LEVELS; i++) {
        if (is_level_unlocked(i)) unlocked_count++;
    }
    
    printf("Levels unlocked: %d / %d\n", unlocked_count, MAX_LEVELS);
    printf("Completion: %d%%\n\n", (unlocked_count * 100) / MAX_LEVELS);
    
    for (int i = 0; i < MAX_LEVELS; i++) {
        printf("Level %d: %s\n", i + 1, is_level_unlocked(i) ? "UNLOCKED" : "LOCKED");
    }
    
    printf("\nText speed: %d ms/char\n", typewriter_delay);
    
    // Check vault status
    printf("Evidence Vault: %s\n", is_vault_unlocked() ? "UNLOCKED" : "LOCKED");
    
    printf("\nPress any key to continue...");
    _getch();
}
