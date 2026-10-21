
#ifndef EVIDENCE_PARSER_H
#define EVIDENCE_PARSER_H

#include <stdio.h>

typedef struct
{
    char *name;
    char *description;
} Evidence;

Evidence *parse_evidence(FILE *file);
int evidence_count(void);
void free_evidence(void);

#endif
