#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "parser_utils.h"
#include "interrogation_parser.h"
#include "past_history_parser.h"
#include "evidence_parser.h"

// Creates a dynamically allocated copy of a string.
char *copy_string(const char *source) {
    char *copy = malloc(strlen(source) + 1);

    if (copy == NULL) {
        return NULL;
    }

    strcpy(copy, source);
    return copy;
}

// Removes a line's newline characters without removing other text.
void remove_newline(char *text) {
    //strcspn takes 2 params: the first one is the text TO BE scanned and the second one contains
    //characters TO scan for. It returns an index of the first match. If it can't find a math, it
    //returns the length of the string.
    text[strcspn(text, "\r\n")] = '\0';
}


// Checks whether a line is a bracketed header.
// Examples: [Interrogation], [ Jawer Chakrovarty ]
bool is_header(const char *text) {
    //If the headers start with spaces or tabs, this loop skips and fetches the first available character
    while (isspace((unsigned char)*text)) {
        text++;
    }

    int length = strlen(text);

    // A header must begin with '[' and end with ']'.
    if (length >= 2 && text[0] == '[' && text[length - 1] == ']') return true;
    else return false;
}
