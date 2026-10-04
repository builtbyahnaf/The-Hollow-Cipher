/*
 * File: level_main.c
 * Project: The Hollow Cipher
 *
 * Description:
 *   Level / Case Main Screen.
 *   Loads and displays the case statement from data/level_X/statement.txt.
 *   Provides navigation to Evidence, Interrogation, Past History, Forensics.
 *   Shows a text input box at bottom with the [Question] as placeholder.
 *
 * Architecture note:
 *   Frontend screen module implementing LevelMain_Draw and LevelMain_HandleKey.
 *   Reads level data files from disk. Interacts with GameState for level tracking.
 */

#include "screens.h"
#include "ui_engine.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

/* ==================== CONSTANTS ==================== */

#define MAX_STATEMENT_LINES 200
#define MAX_LINE_LENGTH 512
#define MAX_QUESTION_LENGTH 256
#define INPUT_BUFFER_SIZE 256

#define OPTION_COUNT 5
static const char *g_option_labels[OPTION_COUNT] = {
    "EVIDENCE",
    "INTERROGATION",
    "PAST HISTORY",
    "FORENSICS",
    "QUIT GAME"
};

static const ScreenID g_option_screens[OPTION_COUNT] = {
    SCREEN_EVIDENCE,
    SCREEN_INTERROGATION,
    SCREEN_HISTORY,
    SCREEN_FORENSICS,
    SCREEN_MAIN_MENU
};

/* ==================== FORWARD DECLARATIONS ==================== */

static int wrap_text_line(const char *input, char output_lines[][MAX_LINE_LENGTH], int max_lines, int max_width);
static void preprocess_wrapped_lines(int max_width);
static void draw_statement_area(void);
static void draw_options_bar(void);
static void draw_input_box(void);
static void handle_text_input(KeyCode key);
static void reset_level_data(void);
static void load_level_data(int level);

/* ==================== SCREEN STATE ==================== */

static char g_statement_lines[MAX_STATEMENT_LINES][MAX_LINE_LENGTH];
static int g_statement_line_count = 0;
static char g_question_text[MAX_QUESTION_LENGTH] = "";
static int g_scroll_offset = 0;
static int g_selected_option = 0;
static bool g_in_text_input = false;
static char g_input_buffer[INPUT_BUFFER_SIZE] = "";
static int g_input_length = 0;
static bool g_data_loaded = false;
static int g_last_wrap_width = -1;  /* Track last wrap width for resize handling */

/* ==================== FILE LOADING ==================== */

static void load_level_data(int level)
{
    if (g_data_loaded) return;

    char filepath[256];
    snprintf(filepath, sizeof(filepath), "data/level_%d/statement.txt", level);

    FILE *fp = fopen(filepath, "r");
    if (!fp) {
        snprintf(g_statement_lines[0], MAX_LINE_LENGTH, "ERROR: Could not load case file: %s", filepath);
        g_statement_line_count = 1;
        strcpy(g_question_text, "What happened?");
        g_data_loaded = true;
        return;
    }

    char line[MAX_LINE_LENGTH];
    bool found_question = false;

    while (fgets(line, sizeof(line), fp) && g_statement_line_count < MAX_STATEMENT_LINES) {
        /* Remove trailing newline */
        line[strcspn(line, "\n")] = '\0';

        /* Check for [Question] tag */
        if (strncmp(line, "[Question]", 10) == 0) {
            found_question = true;
            const char *q = line + 10;
            while (*q == ' ' || *q == '\t') q++;
            strncpy(g_question_text, q, MAX_QUESTION_LENGTH - 1);
            g_question_text[MAX_QUESTION_LENGTH - 1] = '\0';
        } else if (g_statement_line_count < MAX_STATEMENT_LINES) {
            size_t len = strlen(line);
            if (len >= MAX_LINE_LENGTH) len = MAX_LINE_LENGTH - 1;
            memcpy(g_statement_lines[g_statement_line_count], line, len);
            g_statement_lines[g_statement_line_count][len] = '\0';
            g_statement_line_count++;
        }
    }

    fclose(fp);

    if (!found_question) {
        strcpy(g_question_text, "What is your theory?");
    }

    /* Wrap long lines to fit panel width */
    int cols = ui_get_cols();
    int panel_w = cols - 4;
    int content_width = panel_w - 4; /* panel borders + padding */
    preprocess_wrapped_lines(content_width);

    g_data_loaded = true;
}

static void reset_level_data(void)
{
    g_statement_line_count = 0;
    g_question_text[0] = '\0';
    g_scroll_offset = 0;
    g_selected_option = 0;
    g_in_text_input = false;
    g_input_buffer[0] = '\0';
    g_input_length = 0;
    g_data_loaded = false;
    g_last_wrap_width = -1;
}

/* ==================== TEXT WRAPPING HELPER ==================== */

/* Wrap a single line to fit within max_width columns.
 * Returns number of wrapped lines written to output_lines.
 * Modifies the original line in g_statement_lines by replacing it with wrapped lines.
 */
static int wrap_text_line(const char *input, char output_lines[][MAX_LINE_LENGTH], int max_lines, int max_width)
{
    if (!input || max_width <= 0) return 0;

    int len = (int)strlen(input);
    if (len == 0) {
        output_lines[0][0] = '\0';
        return 1;
    }

    if (len <= max_width) {
        strncpy(output_lines[0], input, MAX_LINE_LENGTH - 1);
        output_lines[0][MAX_LINE_LENGTH - 1] = '\0';
        return 1;
    }

    int line_count = 0;
    const char *start = input;
    const char *end = input + len;

    while (start < end && line_count < max_lines) {
        int remaining = (int)(end - start);
        int take = (remaining > max_width) ? max_width : remaining;

        /* If we can fit more, try to break at word boundary */
        if (take == max_width && remaining > max_width) {
            /* Look for last space within the range */
            const char *space = start + max_width - 1;
            while (space > start && *space != ' ') space--;
            if (space > start) {
                take = (int)(space - start);
            }
        }

        /* Copy the segment */
        if (take > MAX_LINE_LENGTH - 1) take = MAX_LINE_LENGTH - 1;
        memcpy(output_lines[line_count], start, take);
        output_lines[line_count][take] = '\0';

        /* Skip whitespace at start of next line */
        start += take;
        while (start < end && *start == ' ') start++;

        line_count++;
    }

    return line_count;
}

/* Preprocess all statement lines to wrap long lines */
static void preprocess_wrapped_lines(int max_width)
{
    /* Temporary storage for wrapped lines */
    static char wrapped[MAX_STATEMENT_LINES * 3][MAX_LINE_LENGTH];
    int wrapped_count = 0;

    for (int i = 0; i < g_statement_line_count && wrapped_count < MAX_STATEMENT_LINES * 3; i++) {
        int lines = wrap_text_line(g_statement_lines[i], &wrapped[wrapped_count], MAX_STATEMENT_LINES * 3 - wrapped_count, max_width);
        wrapped_count += lines;
    }

    /* Copy back to g_statement_lines */
    g_statement_line_count = (wrapped_count < MAX_STATEMENT_LINES) ? wrapped_count : MAX_STATEMENT_LINES;
    for (int i = 0; i < g_statement_line_count; i++) {
        size_t len = strlen(wrapped[i]);
        if (len >= MAX_LINE_LENGTH) len = MAX_LINE_LENGTH - 1;
        memcpy(g_statement_lines[i], wrapped[i], len);
        g_statement_lines[i][len] = '\0';
    }
}

/* ==================== DRAWING HELPERS ==================== */

static void draw_statement_area(void)
{
    int cols = ui_get_cols();
    int rows = ui_get_rows();

    /* Statement panel - takes most of the screen above options */
    int panel_x = 2;
    int panel_y = 2;
    int panel_w = cols - 4;
    int panel_h = rows - 18; /* Leave space for options + input box */

    ui_draw_panel(panel_y, panel_x, panel_w, panel_h, "CASE STATEMENT");

    /* Calculate visible lines */
    int content_rows = panel_h - 2; /* minus top/bottom border */
    int start_line = g_scroll_offset;
    int end_line = start_line + content_rows;
    if (end_line > g_statement_line_count) end_line = g_statement_line_count;

    /* Draw visible lines - text is already wrapped during load */
    for (int i = start_line; i < end_line; i++) {
        int draw_row = panel_y + 1 + (i - start_line);
        COLORREF color = CLR_TEXT;

        /* Highlight [Time], [Location], [Statement] tags */
        if (strncmp(g_statement_lines[i], "[Time]", 6) == 0 ||
            strncmp(g_statement_lines[i], "[Location]", 10) == 0 ||
            strncmp(g_statement_lines[i], "[Statement]", 11) == 0) {
            color = CLR_ACCENT;
        } else if (strncmp(g_statement_lines[i], "---", 3) == 0) {
            color = CLR_MUTED;
        }

        ui_draw_text(draw_row, panel_x + 2, g_statement_lines[i], color);
    }

    /* Scroll indicators */
    if (g_scroll_offset > 0) {
        ui_draw_text_centered(panel_y, "▲ MORE ABOVE ▲", CLR_MUTED);
    }
    if (end_line < g_statement_line_count) {
        ui_draw_text_centered(panel_y + panel_h - 1, "▼ MORE BELOW ▼", CLR_MUTED);
    }
}

static void draw_options_bar(void)
{
    int cols = ui_get_cols();
    int rows = ui_get_rows();

    int bar_y = rows - 15;
    int bar_h = 5;
    int bar_x = 2;
    int bar_w = cols - 4;

    ui_draw_panel(bar_y, bar_x, bar_w, bar_h, "INVESTIGATION MENU");

    int option_spacing = (bar_w - 4) / OPTION_COUNT;
    for (int i = 0; i < OPTION_COUNT; i++) {
        int opt_x = bar_x + 2 + i * option_spacing;
        bool selected = (i == g_selected_option) && !g_in_text_input;

        if (selected) {
            ui_fill_rect(bar_y + 2, opt_x, option_spacing - 1, 1, CLR_HIGHLIGHT_BG);
            ui_draw_text_in_region(bar_y + 2, opt_x, option_spacing - 1, g_option_labels[i], CLR_HIGHLIGHT_FG);
        } else {
            ui_draw_text_in_region(bar_y + 2, opt_x, option_spacing - 1, g_option_labels[i], CLR_TEXT);
        }
    }
}

static void draw_input_box(void)
{
    int cols = ui_get_cols();
    int rows = ui_get_rows();

    int box_y = rows - 9;
    int box_h = 7;
    int box_x = 2;
    int box_w = cols - 4;

    /* Use different border color when focused */
    const char *title = g_in_text_input ? "INPUT ACTIVE [TAB to exit]" : "ANSWER / THEORY [TAB to focus]";
    ui_draw_panel(box_y, box_x, box_w, box_h, title);

    /* Placeholder or input text */
    int text_row = box_y + 2;
    int text_x = box_x + 2;

    if (g_in_text_input) {
        /* Show current input with blinking cursor */
        char display_buf[INPUT_BUFFER_SIZE + 2];
        snprintf(display_buf, sizeof(display_buf), "%s_", g_input_buffer);
        ui_draw_text(text_row, text_x, display_buf, CLR_HIGHLIGHT_FG);

        /* Hint */
        ui_draw_text(text_row + 2, text_x, "[TYPE] Enter answer    [ENTER] Submit    [ESC] Cancel    [BACKSPACE] Delete    [TAB] Exit Input", CLR_MUTED);
    } else {
        /* Show placeholder */
        char placeholder[512];
        snprintf(placeholder, sizeof(placeholder), "[%s]", g_question_text);
        ui_draw_text(text_row, text_x, placeholder, CLR_MUTED);

        /* Hint */
        ui_draw_text(text_row + 2, text_x, "[TAB] Focus Input    [1-5] Quick Menu    [ESC] Back to Case Select", CLR_MUTED);
    }
}

/* ==================== SCREEN DRAWING ==================== */

void LevelMain_Draw(GameState *game)
{
    load_level_data(game->level);

    int cols = ui_get_cols();
    int rows = ui_get_rows();

    /* Re-wrap text if panel width changed (e.g., window resize) */
    int panel_w = cols - 4;
    int content_width = panel_w - 4;
    if (g_last_wrap_width != content_width) {
        preprocess_wrapped_lines(content_width);
        g_last_wrap_width = content_width;
        /* Clamp scroll offset */
        int content_rows = (rows - 18) - 2;
        int max_scroll = g_statement_line_count - content_rows;
        if (max_scroll < 0) max_scroll = 0;
        if (g_scroll_offset > max_scroll) g_scroll_offset = max_scroll;
    }

    /* Header */
    char header[64];
    
    ui_draw_text(1, 2, header, CLR_MUTED);
    ui_draw_hline(2, 2, cols - 4, CLR_BORDER);

    /* Title */
    ui_draw_text_centered(0, g_statement_lines[0], CLR_GOLD);

    draw_statement_area();
    draw_options_bar();
    draw_input_box();

    /* Footer */
    ui_draw_hline(rows - 2, 2, cols - 4, CLR_BORDER);
    if (!g_in_text_input) {
        ui_draw_text(rows - 1, 4, "[UP/DOWN] Scroll    [LEFT/RIGHT] Menu    [TAB] Input    [ESC] Back", CLR_MUTED);
    }
}

/* ==================== INPUT HANDLING ==================== */

static void handle_text_input(KeyCode key)
{
    switch (key) {
        case KEY_ENTER:
            /* Submit answer - for now just exit input mode */
            g_in_text_input = false;
            break;

        case KEY_ESCAPE:
            /* Cancel input */
            g_input_buffer[0] = '\0';
            g_input_length = 0;
            g_in_text_input = false;
            break;

        case KEY_LEFT:
            /* Backspace */
            if (g_input_length > 0) {
                g_input_length--;
                g_input_buffer[g_input_length] = '\0';
            }
            break;

        default:
            /* Handle character input - simplified for arrow keys only */
            /* In a full implementation, WM_CHAR would handle actual text */
            break;
    }
}

void LevelMain_HandleChar(GameState *game, char c)
{
    (void)game;
    if (!g_in_text_input) return;

    /* Handle backspace (ASCII 8) */
    if (c == '\b' || c == 127) {
        if (g_input_length > 0) {
            g_input_length--;
            g_input_buffer[g_input_length] = '\0';
        }
        return;
    }

    /* Handle printable characters */
    if (c >= 32 && c <= 126 && g_input_length < INPUT_BUFFER_SIZE - 1) {
        g_input_buffer[g_input_length++] = c;
        g_input_buffer[g_input_length] = '\0';
    }
}

void LevelMain_HandleKey(GameState *game, KeyCode key)
{
    if (g_in_text_input) {
        handle_text_input(key);
        return;
    }

    int rows = ui_get_rows();
    (void)rows;
    int panel_h = rows - 18;
    int content_rows = panel_h - 2;
    int max_scroll = g_statement_line_count - content_rows;
    if (max_scroll < 0) max_scroll = 0;

    switch (key) {
        case KEY_UP:
            if (g_scroll_offset > 0) {
                g_scroll_offset--;
            }
            break;

        case KEY_DOWN:
            if (g_scroll_offset < max_scroll) {
                g_scroll_offset++;
            }
            break;

        case KEY_LEFT:
            g_selected_option = (g_selected_option - 1 + OPTION_COUNT) % OPTION_COUNT;
            break;

        case KEY_RIGHT:
            g_selected_option = (g_selected_option + 1) % OPTION_COUNT;
            break;

        case KEY_ENTER:
            if (g_selected_option == OPTION_COUNT - 1) {
                /* QUIT GAME -> back to case select */
                reset_level_data();
                Game_ChangeScreen(game, SCREEN_LEVEL_SELECT);
            } else if (g_option_screens[g_selected_option] != SCREEN_MAIN_MENU) {
                Game_ChangeScreen(game, g_option_screens[g_selected_option]);
            }
            break;

        case KEY_ESCAPE:
            reset_level_data();
            Game_ChangeScreen(game, SCREEN_LEVEL_SELECT);
            break;

        case KEY_1: g_selected_option = 0; break;
        case KEY_2: g_selected_option = 1; break;
        case KEY_3: g_selected_option = 2; break;
        case KEY_4: g_selected_option = 3; break;
        case KEY_5: g_selected_option = 4; break;

        case KEY_TAB:
            if (g_in_text_input) {
                g_in_text_input = false;
            } else {
                g_in_text_input = true;
                g_input_buffer[0] = '\0';
                g_input_length = 0;
            }
            break;

        default:
            break;
    }
}

/* Need to add KEY_TAB to ui_engine.h KeyCode enum */