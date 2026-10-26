#ifndef PARSER_UTILS_H
#define PARSER_UTILS_H

char *copy_string(const char *source);
void remove_newline(char *text);
bool is_header(const char *text);

#endif