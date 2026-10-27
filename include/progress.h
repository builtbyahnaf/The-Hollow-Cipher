#ifndef  PROGRESS_H
#define PROGRESS_H

#define MAX_LEVELS 2

void load_progress(void);
int is_level_unlocked(int level_index);
void set_level_unlocked(int level_index);
void reset_progress(void);

#endif