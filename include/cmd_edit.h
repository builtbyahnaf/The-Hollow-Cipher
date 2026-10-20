#ifndef CMD_EDIT_H
#define CMD_EDIT_H

void select_level(void);
void component_selection_dialogue(int LEVEL);
const char* get_component_filename(int component);
void open_editor(int LEVEL, int COMPONENT);

#endif