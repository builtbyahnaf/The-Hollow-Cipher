#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <cmd_edit.h>
#include <string.h>
#include <time.h>
#include "admin_panel.h"

enum LEVELS {
    LEVEL_1 = 1,
    LEVEL_2 = 2
};

enum OPTIONS {
    EVIDENCE = 11,
    INTERROGATION = 22,
    PAST_HISTORY = 33,
    SOLUTION = 44,
    STATEMENT = 55
};

typedef struct {
    //Holds for only 0-63. Which is enough for here.
    int selected_menu : 6;

    //WARNING: Holds only 0-2. TODO: Replace later to extend compatibility
    int selected_level : 2;
} EditSelection;



void component_selection_dialogue(int LEVEL) {
    char selection;

    while (1) {
        printf("\n \n");

        printf("               SELECT THE COMPONENT YOU WANT TO EDIT              ");

        printf("\n \n");

        printf("COMPONENT 1: EVIDENCE [E]\n");
        printf("COMPONENT 2: INTERROGATION [I]\n");
        printf("COMPONENT 3: PAST HISTORY [H]\n");
        printf("COMPONENT 4: SOLUTION [A]\n");
        printf("COMPONENT 5: STATEMENT [S]\n\n");
        printf("GO BACK [X]\n");

        printf("ENTER SELECTION: ");
        scanf(" %c", &selection);

        if (selection < 'A' || selection > 'z') {
            printf("Invalid selection. Please try again.\n");
            continue;
        } else if (selection >= 'A' && selection <= 'Z') {
            selection = tolower(selection);
        }

        switch (selection) {
        case 'e':
            system("cls");
            printf("Evidence file opened in notepad for you to edit:\n");
            open_editor(LEVEL, EVIDENCE);
            break;

        case 'i':
            system("cls");
            printf("Interrogation file opened in notepad for you to edit:\n");
            open_editor(LEVEL, INTERROGATION);
            break;
        case 'h':
            system("cls");
            printf("Past History file opened in notepad for you to edit:\n");
            open_editor(LEVEL, PAST_HISTORY);
            break;
        case 'a':
            system("cls");
            printf("Solution file opened in notepad for you to edit:\n");
            open_editor(LEVEL, SOLUTION);
            break;
        case 's':
            system("cls");
            printf("Statement file opened in notepad for you to edit:\n");
            open_editor(LEVEL, STATEMENT);
            break;
        case 'x':
            system("cls");
            select_level();
            return;

        default:
            printf("INVALID SELECTION\n");
            break;
        }
    }
}

void select_level() {
    int selection;

    printf("-----------------------------------------------------------------\n");
    printf("|               T H E   H O L L O W    C I P H E R               |\n");
    printf("-----------------------------------------------------------------\n");

    printf("\n \n");

    printf("               SELECT THE LEVEL YOU WANT TO EDIT              ");

    printf("\n \n");

    RETRY1:
    printf("TO GO BACK, ENTER 0 (ZERO)\n\n");
    printf("LEVEL 1: THE SILENT PATIENT\n");
    printf("LEVEL 2: THE MISSING NOTEBOOK\n");

    printf("ENTER SELECTION: ");
    scanf("%d", &selection);

    if (selection < 0 || selection > 2) {
        printf("Error: Invalid level\n");
        goto RETRY1;
    }

    if (selection == 0)
    {
        system("cls");
        show_panel();
        return;
    }
    
    system("cls");
    component_selection_dialogue(selection);
}

const char* get_component_filename(int component) {
    switch (component) {
        case EVIDENCE: return "evidence.txt";
        case INTERROGATION: return "interrogation.txt";
        case PAST_HISTORY: return "past_history.txt";
        case SOLUTION: return "solution.txt";
        case STATEMENT: return "statement.txt";
        default: return NULL;
    }
}

void write_log(int LEVEL, int COMPONENT) {
    const char* filename = get_component_filename(COMPONENT);
    if (!filename) {
        return;
    }

    char filepath[256];
    snprintf(filepath, sizeof(filepath), "data/level_%d/%s", LEVEL, filename);

    time_t now = time(NULL);
    struct tm* tm_info = localtime(&now);

    char time_str[128];
    strftime(time_str, sizeof(time_str), "%d, %B, %Y, %A, %I:%M:%S %p", tm_info);

    char new_log[512];
    snprintf(new_log, sizeof(new_log), "Edited %s on (%s)\n", filepath, time_str);

    FILE* log_file = fopen("data/edit_log.thcdb", "r");
    char* existing_logs = NULL;
    size_t existing_size = 0;

    if (log_file) {
        fseek(log_file, 0, SEEK_END);
        existing_size = ftell(log_file);
        rewind(log_file);

        if (existing_size > 0) {
            existing_logs = malloc(existing_size + 1);
            if (existing_logs) {
                fread(existing_logs, 1, existing_size, log_file);
                existing_logs[existing_size] = '\0';
            }
        }
        fclose(log_file);
    }

    log_file = fopen("data/edit_log.thcdb", "w");
    if (!log_file) {
        free(existing_logs);
        return;
    }

    fputs(new_log, log_file);
    if (existing_logs) {
        fputs(existing_logs, log_file);
        free(existing_logs);
    }

    fclose(log_file);
}

void open_editor(int LEVEL, int COMPONENT) {
    if (LEVEL < 1 || LEVEL > 2) {
        printf("Error: Invalid level\n");
        return;
    }

    const char* filename = get_component_filename(COMPONENT);
    if (!filename) {
        printf("Error: Invalid component\n");
        return;
    }

    char filepath[256];
    snprintf(filepath, sizeof(filepath), "data/level_%d/%s", LEVEL, filename);

    FILE* test = fopen(filepath, "r");
    if (!test) {
        printf("Error: File not found: %s\n", filepath);
        return;
    }
    fclose(test);

    char command[512];
    snprintf(command, sizeof(command), "notepad.exe \"%s\"", filepath);

    printf("Opening: %s\n", filepath);
    system(command);

    write_log(LEVEL, COMPONENT);

    printf("File editing complete. Changes saved.\n");
}




