#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "evidence_parser.h"

static int ev_count = 0;
static Evidence *ev_list = NULL;

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

static int is_evidence_header(const char *text) {
    while (isspace((unsigned char)*text)) {
        text++;
    }

    if (!isdigit((unsigned char)*text)) {
        return 0;
    }

    while (isdigit((unsigned char)*text)) {
        text++;
    }

    return *text == '.';
}

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
    size_t old_length = strlen(*description);
    size_t line_length = strlen(line);

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

        if (is_evidence_header(line)) {
            char *title = strchr(line, '.');

            if (title == NULL) {
                continue;
            }

            title++;
            trim_whitespace(title);

            if (strcmp(title, "Evidences") == 0) {
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

            ev_list[ev_count].name = copy_string(title);
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