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
#include <ctype.h>
#include <stdbool.h>

/* ==================== CONSTANTS ==================== */

#define MAX_STATEMENT_LINES 200
#define MAX_LINE_LENGTH 512
#define MAX_QUESTION_LENGTH 256
#define INPUT_BUFFER_SIZE 256
#define MAX_ANSWER_LENGTH 256
#define MAX_EXPLANATION_LINES 100

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
static void load_solution_data(int level);
static bool answers_match(const char *response, const char *answer);
static void submit_answer(void);
static void draw_result_dialog(void);

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

/* Solution / answer-checking state */
static char g_solution_answer[MAX_ANSWER_LENGTH] = "";
static char g_solution_explanation[MAX_EXPLANATION_LINES][MAX_LINE_LENGTH];
static int g_solution_explanation_count = 0;
static char g_provided_response[INPUT_BUFFER_SIZE] = "";
static bool g_result_active = false;
static bool g_result_correct = false;
static uint64_t g_result_start = 0;
static int g_explanation_scroll = 0;

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

    /* Load the solution (answer + explanation) for answer checking */
    load_solution_data(level);

    g_data_loaded = true;
}

/* Load data/level_X/solution.txt.
 * Parses the [Answer] section (single logical line) and the
 * [Explanation] section (kept as individual lines for display). */
static void load_solution_data(int level)
{
    char filepath[256];
    snprintf(filepath, sizeof(filepath), "data/level_%d/solution.txt", level);

    g_solution_answer[0] = '\0';
    g_solution_explanation_count = 0;

    FILE *fp = fopen(filepath, "r");
    if (!fp) return;

    char line[MAX_LINE_LENGTH];
    int section = 0; /* 0 = none, 1 = [Answer], 2 = [Explanation] */

    while (fgets(line, sizeof(line), fp)) {
        line[strcspn(line, "\r\n")] = '\0';

        if (strncmp(line, "[Answer]", 8) == 0) {
            section = 1;
            continue;
        }
        if (strncmp(line, "[Explanation]", 13) == 0) {
            section = 2;
            continue;
        }

        /* Trim trailing whitespace */
        size_t len = strlen(line);
        while (len > 0 && (line[len - 1] == ' ' || line[len - 1] == '\t'))
            line[--len] = '\0';
        if (len == 0) continue;

        if (section == 1) {
            /* Answer may span multiple lines: join with a space */
            size_t cur = strlen(g_solution_answer);
            if (cur > 0 && cur < MAX_ANSWER_LENGTH - 1)
                g_solution_answer[cur++] = ' ';
            snprintf(g_solution_answer + cur, MAX_ANSWER_LENGTH - cur, "%s", line);
        } else if (section == 2 && g_solution_explanation_count < MAX_EXPLANATION_LINES) {
            size_t copy_len = strlen(line);
            if (copy_len >= MAX_LINE_LENGTH) copy_len = MAX_LINE_LENGTH - 1;
            memcpy(g_solution_explanation[g_solution_explanation_count], line, copy_len);
            g_solution_explanation[g_solution_explanation_count][copy_len] = '\0';
            g_solution_explanation_count++;
        }
    }

    fclose(fp);
}

/* Case-insensitive comparison of the player's response against the answer.
 * Both sides are trimmed of surrounding whitespace, lower-cased into
 * temporary buffers, then compared with strcmp() from string.h. */
static bool answers_match(const char *response, const char *answer)
{
    if (!response || !answer) return false;

    char ra[INPUT_BUFFER_SIZE];
    char aa[MAX_ANSWER_LENGTH];

    const char *rs = response;
    while (*rs == ' ' || *rs == '\t') rs++;
    size_t rl = strlen(rs);
    while (rl > 0 && (rs[rl - 1] == ' ' || rs[rl - 1] == '\t')) rl--;
    if (rl >= sizeof(ra)) rl = sizeof(ra) - 1;
    for (size_t i = 0; i < rl; i++)
        ra[i] = (char)tolower((unsigned char)rs[i]);
    ra[rl] = '\0';

    const char *as = answer;
    while (*as == ' ' || *as == '\t') as++;
    size_t al = strlen(as);
    while (al > 0 && (as[al - 1] == ' ' || as[al - 1] == '\t')) al--;
    if (al >= sizeof(aa)) al = sizeof(aa) - 1;
    for (size_t i = 0; i < al; i++)
        aa[i] = (char)tolower((unsigned char)as[i]);
    aa[al] = '\0';

    return strcmp(ra, aa) == 0;
}

/* Capture the current input buffer as the provided response,
 * check it against the solution, and show the result dialog. */
static void submit_answer(void)
{
    if (g_input_length == 0) return;

    strncpy(g_provided_response, g_input_buffer, INPUT_BUFFER_SIZE - 1);
    g_provided_response[INPUT_BUFFER_SIZE - 1] = '\0';

    g_result_correct = answers_match(g_provided_response, g_solution_answer);
    g_result_active = true;
    g_result_start = ui_get_ticks();
    g_explanation_scroll = 0;
    g_in_text_input = false;
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
    g_solution_answer[0] = '\0';
    g_solution_explanation_count = 0;
    g_provided_response[0] = '\0';
    g_result_active = false;
    g_result_correct = false;
    g_result_start = 0;
    g_explanation_scroll = 0;
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

    /* Panel title shows the [Question] from statement.txt */
    char title_buf[MAX_QUESTION_LENGTH + 32];
    if (g_in_text_input) {
        snprintf(title_buf, sizeof(title_buf), "INPUT ACTIVE [TAB to exit]");
    } else {
        snprintf(title_buf, sizeof(title_buf), "%s  [TAB to focus]", g_question_text);
    }
    ui_draw_panel(box_y, box_x, box_w, box_h, title_buf);

    /* Placeholder or input text */
    int text_row = box_y + 2;
    int text_x = box_x + 2;

    if (g_in_text_input) {
        /* Show current input with blinking cursor */
        char display_buf[INPUT_BUFFER_SIZE + 2];
        snprintf(display_buf, sizeof(display_buf), "%s_", g_input_buffer);
        ui_draw_text(text_row, text_x, display_buf, CLR_HIGHLIGHT_FG);

        /* Hint */
        ui_draw_text(text_row + 2, text_x, "[ENTER] Submit    [ESC] Cancel    [BACKSPACE] Delete    [TAB] Exit Input", CLR_MUTED);
    } else {
        /* Show placeholder */
        ui_draw_text(text_row, text_x, "Type answer here...", CLR_MUTED);

        /* Hint */
        ui_draw_text(text_row + 2, text_x, "[TAB] Focus Input    [1-5] Quick Menu    [ESC] Back to Case Select", CLR_MUTED);
    }

    /* Footer note: full name / spelling must match */
    ui_draw_text(box_y + 5, text_x,
                 "NOTE: Write the FULL name. Spelling must match exactly.", CLR_YELLOW);
}

/* ==================== RESULT DIALOG (answer check) ==================== */

/* Draw the congratulations / wrong-answer dialog with animations.
 * Uses ui_get_ticks() for a pulsing border, a typewriter title,
 * staggered content reveal and blinking prompts. */
static void draw_result_dialog(void)
{
    int cols = ui_get_cols();
    int rows = ui_get_rows();
    uint64_t now = ui_get_ticks();
    uint32_t elapsed = (uint32_t)(now - g_result_start);

    int dlg_w = cols - 8;
    if (dlg_w > 100) dlg_w = 100;
    if (dlg_w < 56) dlg_w = cols > 60 ? 60 : cols - 4;
    int dlg_h = rows - 8;
    if (dlg_h < 24) dlg_h = 24;
    if (dlg_h > rows - 2) dlg_h = rows - 2;
    int dlg_x = (cols - dlg_w) / 2;
    int dlg_y = (rows - dlg_h) / 2;
    if (dlg_y < 1) dlg_y = 1;

    /* Pulsing border color: gold<->green (correct), red<->amber (wrong) */
    COLORREF border;
    if (g_result_correct)
        border = ((elapsed / 300) % 2 == 0) ? CLR_GOLD : CLR_GREEN;
    else
        border = ((elapsed / 300) % 2 == 0) ? CLR_RED : CLR_ACCENT;

    ui_draw_panel(dlg_y, dlg_x, dlg_w, dlg_h, "CASE RESULT");
    /* Pulse bottom/side borders (top keeps the panel title badge) */
    ui_fill_rect(dlg_y + dlg_h - 1, dlg_x, dlg_w, 1, border);
    ui_fill_rect(dlg_y, dlg_x + 1, 1, dlg_h - 2, border);
    ui_fill_rect(dlg_y, dlg_x + dlg_w - 2, 1, dlg_h - 2, border);

    /* Title with typewriter reveal */
    const char *title = g_result_correct ? "CONGRATULATIONS!" : "WRONG ANSWER";
    int tlen = (int)strlen(title);
    int shown = 0;
    if (elapsed > 300) shown = (int)((elapsed - 300) / 60);
    if (shown > tlen) shown = tlen;
    char tbuf[64];
    memcpy(tbuf, title, shown);
    tbuf[shown] = '\0';

    COLORREF title_color = g_result_correct
        ? (((elapsed / 300) % 2 == 0) ? CLR_GOLD : CLR_GREEN)
        : CLR_RED;
    ui_draw_text_in_region(dlg_y + 2, dlg_x, dlg_w, tbuf, title_color);

    /* Blinking stars once the title is complete */
    if (shown >= tlen) {
        bool star_on = ((elapsed / 45) % 2 == 0);
        if (star_on) {
            ui_draw_text(dlg_y + 2, dlg_x + 2, "*", CLR_GOLD);
            ui_draw_text(dlg_y + 2, dlg_x + dlg_w - 3, "*", CLR_GOLD);
        }
    }

    /* Staggered content: each line appears 300ms after the previous */
    uint32_t content_start = 300 + (uint32_t)tlen * 60 + 200;

    if (g_result_correct) {
        /* --- Correct answer: celebration + explanation --- */
        char line_a[INPUT_BUFFER_SIZE + 32];
        char line_b[INPUT_BUFFER_SIZE + 32];
        snprintf(line_a, sizeof(line_a), "CORRECT ANSWER: %s", g_solution_answer);
        snprintf(line_b, sizeof(line_b), "YOUR ANSWER: %s", g_provided_response);

        if (elapsed > content_start)
            ui_draw_text(dlg_y + 4, dlg_x + 2, "You have solved the case.", CLR_GREEN);
        if (elapsed > content_start + 300)
            ui_draw_text(dlg_y + 5, dlg_x + 2, line_a, CLR_ACCENT);
        if (elapsed > content_start + 600)
            ui_draw_text(dlg_y + 6, dlg_x + 2, line_b, CLR_TEXT);

        /* Explanation section */
        int ex_first_row = dlg_y + 9;
        int ex_last_row  = dlg_y + dlg_h - 3;
        int ex_avail     = ex_last_row - ex_first_row + 1;
        if (ex_avail < 1) ex_avail = 1;

        if (elapsed > content_start + 900) {
            ui_draw_hline(dlg_y + 8, dlg_x + 2, dlg_w - 4, CLR_BORDER);
            ui_draw_text_centered(dlg_y + 8, "EXPLANATION", CLR_GOLD);

            /* Wrap the raw explanation lines at the dialog width */
            static char ex_wrapped[400][MAX_LINE_LENGTH];
            int ex_count = 0;
            int ex_width = dlg_w - 6;
            for (int i = 0; i < g_solution_explanation_count && ex_count < 400; i++) {
                int n = wrap_text_line(g_solution_explanation[i],
                                       &ex_wrapped[ex_count], 400 - ex_count, ex_width);
                ex_count += n;
            }

            int ex_max_scroll = ex_count - ex_avail;
            if (ex_max_scroll < 0) ex_max_scroll = 0;
            if (g_explanation_scroll > ex_max_scroll) g_explanation_scroll = ex_max_scroll;

            for (int i = 0; i < ex_avail; i++) {
                int src = g_explanation_scroll + i;
                if (src >= ex_count) break;
                ui_draw_text(ex_first_row + i, dlg_x + 3, ex_wrapped[src], CLR_TEXT);
            }

            if (ex_max_scroll > 0) {
                bool blink_on = ((elapsed / 400) % 2 == 0);
                if (blink_on) {
                    ui_draw_text_centered(ex_last_row + 1,
                        g_explanation_scroll > 0 ? "[UP/DOWN] Scroll" : "[DOWN] Scroll", CLR_YELLOW);
                }
            }
        }
    } else {
        /* --- Wrong answer: encourage a retry --- */
        char line_a[INPUT_BUFFER_SIZE + 32];
        snprintf(line_a, sizeof(line_a), "Your answer: %s", g_provided_response);

        if (elapsed > content_start)
            ui_draw_text(dlg_y + 4, dlg_x + 2, line_a, CLR_TEXT);
        if (elapsed > content_start + 300)
            ui_draw_text(dlg_y + 5, dlg_x + 2, "The evidence does not support this conclusion.", CLR_RED);
        if (elapsed > content_start + 600)
            ui_draw_text(dlg_y + 6, dlg_x + 2, "Review the statement, interrogations and past history.", CLR_TEXT);

        if (elapsed > content_start + 900) {
            bool blink_on = ((elapsed / 400) % 2 == 0);
            if (blink_on)
                ui_draw_text_centered(dlg_y + 8, "Press [ENTER] to try again", CLR_YELLOW);
        }
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
    if (!g_in_text_input && !g_result_active) {
        ui_draw_text(rows - 1, 4, "[UP/DOWN] Scroll    [LEFT/RIGHT] Menu    [TAB] Input    [ESC] Back", CLR_MUTED);
    }

    /* Result dialog (congratulations / wrong answer) drawn on top */
    if (g_result_active) {
        draw_result_dialog();
    }
}

/* ==================== INPUT HANDLING ==================== */

static void handle_text_input(KeyCode key)
{
    switch (key) {
        case KEY_ENTER:
            /* Submit answer: save as PROVIDED_RESPONSE and check it */
            submit_answer();
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
    /* Result dialog is modal: only allow dismiss/scroll keys */
    if (g_result_active) {
        switch (key) {
            case KEY_ENTER:
            case KEY_TAB:
                if (g_result_correct) {
                    /* Case solved: clear the dialog and the input */
                    g_result_active = false;
                    g_in_text_input = false;
                    g_input_buffer[0] = '\0';
                    g_input_length = 0;
                } else {
                    /* Try again: keep the previous text so it can be edited */
                    g_result_active = false;
                    g_in_text_input = true;
                }
                break;

            case KEY_ESCAPE:
                g_result_active = false;
                g_in_text_input = false;
                break;

            case KEY_UP:
                if (g_result_correct && g_explanation_scroll > 0)
                    g_explanation_scroll--;
                break;

            case KEY_DOWN:
                if (g_result_correct)
                    g_explanation_scroll++;
                break;

            default:
                break;
        }
        return;
    }

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