
#ifndef PAST_HISTORY_PARSER_H
#define PAST_HISTORY_PARSER_H

#include <stdio.h>

typedef struct
{
    char *name;
    char *history;
} PastHistory;

PastHistory *parse_past_history(FILE *file);
int past_history_count(void);
void free_past_history(void);

#endif
