#include <stdio.h>
#include <windows.h>
#include <stdlib.h>

#include "vault_pass.h"
#include "color_palette.h"

static bool is_puzzle_passed = false;
static int puzzle_password = 0;

void handle_puzzle(int pass);

void set_puzzle_passed(bool p){
    is_puzzle_passed = p;
}

void show_vault_puzzle(){
    // Clear the screen and move the cursor to the top-left.
        system("cls");

        // Apply background and foreground colors from main.c
        printf(BG_LEATHER GOLD);

        puzzle_password = generate_pass();

        printf("-----------------------------------------------------------------\n");
        printf("|                  E V I D E N C E   V A U L T                   |\n");
        printf("-----------------------------------------------------------------\n\n");

        char ch;
        int delay = 30;
        char vault_text_1[] = "The evidences for this case are secured inside this vault. You have to guess the correct password by trial and error.\n\n"
                                "The rules are as follows:\n\n";
        char vault_text_2[] = "The digit is present in the password and it is in the right position.\n";
        char vault_text_3[] = "The digit is presemt in the password but it is in the wrong position.\n";
        char vault_text_4[] = "The digit is absent in the password.\n";

    for (int i = 0; vault_text_1[i] != '\0'; i++) {
        printf("%c", vault_text_1[i]);
        fflush(stdout);
        Sleep(delay);
    }
    printf(MINT_GREEN);
    printf("GREEN = " GOLD);
    for (int i = 0; vault_text_2[i] != '\0'; i++) {
        printf("%c", vault_text_2[i]);
        fflush(stdout);
        Sleep(delay);
    }
    printf(ACID_LIME);
    printf("YELLOW = " GOLD);
    for (int i = 0; vault_text_3[i] != '\0'; i++) {
        printf("%c", vault_text_3[i]);
        fflush(stdout);
        Sleep(delay);
    }
    printf(CRIMSON);
    printf("RED = " GOLD);
    for (int i = 0; vault_text_4[i] != '\0'; i++) {
        printf("%c", vault_text_4[i]);
        fflush(stdout);
        Sleep(delay);
    }
    

}
void handle_puzzle(int pass){
    if (is_puzzle_passed) {
        return;
    }
    int guess = 0;

    int pd[5] = {
        (pass / 10000) % 10,
        (pass / 1000) % 10,
        (pass / 100) % 10,
        (pass / 10) % 10,
        pass % 10
    };

    RETRY:

    printf("\n\nEnter your guess: ");
    scanf("%d", &guess);

    if(guess < 10000 || guess > 99999){
        printf("Please enter a 5-digit number.\n");
        goto RETRY;
    }

    int gd[5] = {
        (guess / 10000) % 10,
        (guess / 1000) % 10,
        (guess / 100) % 10,
        (guess / 10) % 10,
        guess % 10
    };

    int result[5] = {0}; // 0 = red (absent), 1 = yellow (wrong pos), 2 = green (correct pos)
    int pass_used[5] = {0};

    // First pass: exact matches (green)
    for (int i = 0; i < 5; i++) {
        if (gd[i] == pd[i]) {
            result[i] = 2;
            pass_used[i] = 1;
        }
    }

    // Second pass: wrong position matches (yellow)
    for (int i = 0; i < 5; i++) {
        if (result[i] == 0) {
            for (int j = 0; j < 5; j++) {
                if (!pass_used[j] && gd[i] == pd[j]) {
                    result[i] = 1;
                    pass_used[j] = 1;
                    break;
                }
            }
        }
    }

    // Display result with colors
    
    for (int i = 0; i < 5; i++) {
        if (result[i] == 2) {
            printf(MINT_GREEN "%d ", gd[i]);
        } else if (result[i] == 1) {
            printf(ACID_LIME "%d ", gd[i]);
        } else {
            printf(CRIMSON "%d ", gd[i]);
        }
        printf(GOLD);
    }
    printf("\nTry again.");

    // Check if solved
    if (result[0] == 2 && result[1] == 2 && result[2] == 2 && result[3] == 2 && result[4] == 2) {
        printf(MINT_GREEN "\nVault unlocked!\n");
        is_puzzle_passed = true;
        return;
    }

    goto RETRY;
}