#include <stdio.h>
#include "admin_panel.h"
#include <ctype.h>
#include <stdlib.h>
#include <conio.h>
#include <windows.h>
#include "cmd_edit.h"
#include "credentials_manager.h"
#include "log_parser.h"

void show_panel(void) {
    char selection;

    while (1) {
        system("cls");
        printf("-----------------------------------------------------------------\n");
        printf("|               T H E   H O L L O W    C I P H E R               |\n");
        printf("-----------------------------------------------------------------\n\n");

        printf("                           ADMIN PANEL                          \n\n");

        printf("EDIT LEVELS [E]\n");
        printf("USER MANAGEMENT [U]\n");
        printf("VIEW EDIT LOGS [L]\n");
        printf("CHANGE ADMIN PASSWORD [P]\n");
        printf("EXIT [X]\n\n");

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
            select_level();
            break;

        case 'u':
            system("cls");
            user_management_panel();
            break;

        case 'l':
            system("cls");
            view_edit_logs();
            break;

        case 'p':
            system("cls");
            set_new_pass();
            printf("\nPress any key to continue...");
            _getch();
            break;

        case 'x':
            system("cls");
            return;

        default:
            printf("Invalid selection. Please try again.\n");
            Sleep(1000);
            break;
        }
    }
}

void user_management_panel(void) {
    char selection;

    while (1) {
        system("cls");
        printf("-----------------------------------------------------------------\n");
        printf("|                   U S E R   M A N A G E M E N T                |\n");
        printf("-----------------------------------------------------------------\n\n");

        printf("LIST USERS [L]\n");
        printf("ADD USER [A]\n");
        printf("REMOVE USER [R]\n");
        printf("GO BACK [X]\n\n");

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
        case 'l':
            system("cls");
            list_users();
            printf("\nPress any key to continue...");
            _getch();
            system("cls");
            break;

        case 'a':
            system("cls");
            add_new_user_interactive();
            printf("\nPress any key to continue...");
            _getch();
            system("cls");
            break;

        case 'r':
            system("cls");
            remove_user_interactive();
            printf("\nPress any key to continue...");
            _getch();
            system("cls");
            break;

        case 'x':
            system("cls");
            return; // Returns to show_panel()

        default:
            printf("Invalid selection.\n");
            Sleep(1000);
            break;
        }
    }
}
