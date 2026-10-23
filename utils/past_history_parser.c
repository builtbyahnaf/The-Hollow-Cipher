#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "past_history_parser.h"

static int history_count = 0;
static PastHistory *ph_list = NULL;

static char *copy_string(const char *source) {
    char *copy = malloc(strlen(source) + 1);

    if (copy == NULL) {
        return NULL;
    }

    strcpy(copy, source);
    return copy;
}

static char *trim_whitespace(char *text) {
    while (isspace((unsigned char)*text)) {
        text++;
    }

    if (*text == '\0') {
        return text;
    }

    char *end = text + strlen(text) - 1;

    while (end > text && isspace((unsigned char)*end)) {
        end--;
    }

    *(end + 1) = '\0';

    return text;
}

static void remove_newline(char *text) {
    text[strcspn(text, "\r\n")] = '\0';
}

static int is_header(const char *text) {
    while (isspace((unsigned char)*text)) {
        text++;
    }

    size_t length = strlen(text);

    return length >= 2 &&
           text[0] == '[' &&
           text[length - 1] == ']';
}

static void clear_history(void) {
    for (int i = 0; i < history_count; i++) {
        free(ph_list[i].name);
        free(ph_list[i].history);
    }

    free(ph_list);

    ph_list = NULL;
    history_count = 0;
}

static int append_history(char **history, const char *line) {
    size_t old_length = strlen(*history);
    size_t line_length = strlen(line);

    char *temp = realloc(
        *history,
        old_length + line_length + 1
    );

    if (temp == NULL) {
        return 0;
    }

    *history = temp;

    memcpy(
        *history + old_length,
        line,
        line_length + 1
    );

    return 1;
}

PastHistory *parse_past_history(FILE *file_path) {
    char line[2048];

    clear_history();

    if (file_path == NULL) {
        return NULL;
    }

    while (fgets(line, sizeof(line), file_path) != NULL) {
        remove_newline(line);

        if (is_header(line)) {
            line[strlen(line) - 1] = '\0';
            char *name = trim_whitespace(line + 1);

            if (strcmp(name, "Past History") == 0 ||
                strcmp(name, "Investigative Direction") == 0) {
                continue;
            }

            PastHistory *temp = realloc(
                ph_list,
                (history_count + 1) * sizeof(PastHistory)
            );

            if (temp == NULL) {
                clear_history();
                fclose(file_path);
                return NULL;
            }

            ph_list = temp;

            ph_list[history_count].name = copy_string(name);
            ph_list[history_count].history = copy_string("");

            if (ph_list[history_count].name == NULL ||
                ph_list[history_count].history == NULL) {

                free(ph_list[history_count].name);
                free(ph_list[history_count].history);

                clear_history();
                fclose(file_path);
                return NULL;
            }

            history_count++;
        } else if (history_count > 0) {
            if (!append_history(
                    &ph_list[history_count - 1].history,
                    line) ||
                !append_history(
                    &ph_list[history_count - 1].history,
                    "\n")) {

                clear_history();
                fclose(file_path);
                return NULL;
            }
        }
    }

    fclose(file_path);

    if (history_count == 0) {
        return NULL;
    }

    return ph_list;
}

int past_history_count(void) {
    return history_count;
}

void free_past_history(void) {
    clear_history();
}