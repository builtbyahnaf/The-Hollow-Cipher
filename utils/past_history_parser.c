#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>

#include "past_history_parser.h"
#include "parser_utils.h"

static int history_count = 0;
static PastHistory *ph_list = NULL;

static void clear_history(void) {
    for (int i = 0; i < history_count; i++) {
        gc_untrack(ph_list[i].name);
        SAFE_FREE(ph_list[i].name);
        gc_untrack(ph_list[i].history);
        SAFE_FREE(ph_list[i].history);
    }

    SAFE_FREE(ph_list);

    ph_list = NULL;
    history_count = 0;
}

static int append_history(char **history, const char *line) {
    int old_length = strlen(*history);
    int line_length = strlen(line);

    char *temp = realloc(
        *history,
        old_length + line_length + 1
    );

    if (temp == NULL) {
        return 0;
    }

    gc_untrack(*history);
    *history = temp;
    gc_track(*history);

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
            char *name = line;

            if (strcmp(name, "[Past History]") == 0 ||
                strcmp(name, "[Investigative Direction]") == 0) {
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

                clear_history();
                fclose(file_path);
                return NULL;
            }

            history_count++;
        } else if (history_count > 0) {
            if (!append_history(&ph_list[history_count - 1].history, line) ||
                !append_history(&ph_list[history_count - 1].history, "\n")) {

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
    gc_collect();
}
