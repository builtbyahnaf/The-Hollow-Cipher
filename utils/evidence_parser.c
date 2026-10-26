#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>

#include "evidence_parser.h"
#include "parser_utils.h"

static int ev_count = 0;
static Evidence *ev_list = NULL;

static void clear_evidence(void) {
    for (int i = 0; i < ev_count; i++) {
        free(ev_list[i].name);
        free(ev_list[i].description);
    }

    free(ev_list);

    ev_list = NULL;
    ev_count = 0;
}

static int append_description(char **description, const char *line) {
    int old_length = strlen(*description);
    int line_length = strlen(line);

    char *temp = realloc(
        *description,
        old_length + line_length + 1
    );

    if (temp == NULL) {
        return 0;
    }

    *description = temp;

    memcpy(
        *description + old_length,
        line,
        line_length + 1
    );

    return 1;
}

Evidence *parse_evidence(FILE *file_path) {
    char line[2048];

    clear_evidence();

    if (file_path == NULL) {
        return NULL;
    }

    while (fgets(line, sizeof(line), file_path) != NULL) {
        remove_newline(line);

        if (is_header(line)) {
            char *name = line;

            if (strcmp(name, "[Evidences]") == 0) {
                continue;
            }

            Evidence *temp = realloc(
                ev_list,
                (ev_count + 1) * sizeof(Evidence)
            );

            if (temp == NULL) {
                clear_evidence();
                fclose(file_path);
                return NULL;
            }

            ev_list = temp;

            ev_list[ev_count].name = copy_string(name);
            ev_list[ev_count].description = copy_string("");

            if (ev_list[ev_count].name == NULL ||
                ev_list[ev_count].description == NULL) {

                free(ev_list[ev_count].name);
                free(ev_list[ev_count].description);

                clear_evidence();
                fclose(file_path);
                return NULL;
            }

            ev_count++;
        } else if (ev_count > 0) {
            if (!append_description(
                    &ev_list[ev_count - 1].description,
                    line) ||
                !append_description(
                    &ev_list[ev_count - 1].description,
                    "\n")) {

                clear_evidence();
                fclose(file_path);
                return NULL;
            }
        }
    }

    fclose(file_path);

    if (ev_count == 0) {
        return NULL;
    }

    return ev_list;
}

int evidence_count(void) {
    return ev_count;
}

void free_evidence(void) {
    clear_evidence();
}