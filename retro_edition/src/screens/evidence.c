/*
 * File: evidence.c
 * Project: The Hollow Cipher
 *
 * Description:
 *   Evidence Archive Screen.
 *   Loads and displays evidence data from data/level_X/evidence.txt.
 *   Each numbered item (e.g. "1. The Murder Knife") becomes a list entry.
 *   Selecting an item shows its full description in a detail view.
 *   Features arcade blink animation on selection.
 *
 * Architecture note:
 *   Frontend screen module implementing Evidence_Draw and Evidence_HandleKey.
 */

#include "screens.h"
#include "ui_engine.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

/* ==================== CONSTANTS ==================== */

#define MAX_EVIDENCE_ITEMS 20
#define MAX_WRAP_LINES 200
#define MAX_LINE_LENGTH 512

/* ==================== SCREEN STATE ==================== */

typedef struct {
    char title[128];
    char content[1024];
} EvidenceItem;

static EvidenceItem g_evidence_items[MAX_EVIDENCE_ITEMS];
static int g_evidence_count = 0;
static int g_selected_index = 0;
static int g_scroll_offset = 0;
static bool g_data_loaded = false;
static bool g_in_detail_view = false;
static int g_detail_scroll = 0;

/* Wrap buffer for the currently-selected detail item */
static char g_wrap_buf[MAX_WRAP_LINES][MAX_LINE_LENGTH];
static int g_wrap_count = 0;
static int g_wrap_item = -1;
static int g_wrap_width = -1;

/* Arcade blink animation state */
static int      g_blink_active = -1;
static uint64_t g_blink_start  = 0;
static const uint32_t BLINK_TIME = 240;

/* ==================== TEXT WRAPPING ==================== */

static int wrap_single_line(const char *input, char out[][MAX_LINE_LENGTH], int max_out_lines, int max_width)
{
    if (!input || max_width <= 0) return 0;
    int len = (int)strlen(input);
    if (len == 0) {
        out[0][0] = '\0';
        return 1;
    }
    if (len <= max_width) {
        size_t copy = (len < MAX_LINE_LENGTH - 1) ? (size_t)len : MAX_LINE_LENGTH - 1;
        memcpy(out[0], input, copy);
        out[0][copy] = '\0';
        return 1;
    }

    int count = 0;
    const char *start = input;
    const char *end = input + len;

    while (start < end && count < max_out_lines) {
        int remaining = (int)(end - start);
        int take = (remaining > max_width) ? max_width : remaining;

        if (take == max_width && remaining > max_width) {
            const char *sp = start + max_width - 1;
            while (sp > start && *sp != ' ') sp--;
            if (sp > start) take = (int)(sp - start);
        }

        if (take > MAX_LINE_LENGTH - 1) take = MAX_LINE_LENGTH - 1;
        memcpy(out[count], start, take);
        out[count][take] = '\0';
        start += take;
        while (start < end && *start == ' ') start++;
        count++;
    }
    return count;
}

/* Wrap the selected item's content into g_wrap_buf */
static void wrap_selected_item(int max_width)
{
    if (g_selected_index < 0 || g_selected_index >= g_evidence_count) return;
    const char *text = g_evidence_items[g_selected_index].content;

    /* Split by newlines first, then wrap each logical line */
    int total = 0;
    const char *ptr = text;
    char line[MAX_LINE_LENGTH];

    while (*ptr && total < MAX_WRAP_LINES) {
        int len = 0;
        while (*ptr && *ptr != '\n' && len < MAX_LINE_LENGTH - 1) {
            line[len++] = *ptr++;
        }
        line[len] = '\0';
        if (*ptr == '\n') ptr++;

        int wrapped = wrap_single_line(line, &g_wrap_buf[total], MAX_WRAP_LINES - total, max_width);
        total += wrapped;
    }

    g_wrap_count = total;
    g_wrap_item = g_selected_index;
    g_wrap_width = max_width;
}

/* ==================== FILE LOADING ==================== */

static void load_evidence_data(int level)
{
    if (g_data_loaded) return;

    char filepath[256];
    snprintf(filepath, sizeof(filepath), "data/level_%d/evidence.txt", level);

    FILE *fp = fopen(filepath, "r");
    if (!fp) {
        snprintf(g_evidence_items[0].title, sizeof(g_evidence_items[0].title), "ERROR");
        snprintf(g_evidence_items[0].content, sizeof(g_evidence_items[0].content),
                 "Could not load: %s", filepath);
        g_evidence_count = 1;
        g_data_loaded = true;
        return;
    }

    char line[MAX_LINE_LENGTH];
    int current = -1;

    while (fgets(line, sizeof(line), fp) && current < MAX_EVIDENCE_ITEMS - 1) {
        line[strcspn(line, "\n")] = '\0';

        if (strncmp(line, "[Evidences]", 11) == 0) continue;

        /* Detect new item: starts with a digit followed by '.' */
        if (strlen(line) >= 2 && line[0] >= '1' && line[0] <= '9' && line[1] == '.') {
            current++;
            if (current >= MAX_EVIDENCE_ITEMS) break;

            /* Skip past "N." and any whitespace to get the title */
            int start = 2;
            while (line[start] == ' ' || line[start] == '\t') start++;

            size_t tlen = strlen(line + start);
            if (tlen >= sizeof(g_evidence_items[current].title)) tlen = sizeof(g_evidence_items[current].title) - 1;
            memcpy(g_evidence_items[current].title, line + start, tlen);
            g_evidence_items[current].title[tlen] = '\0';
            g_evidence_items[current].content[0] = '\0';
        } else if (current >= 0 && strlen(line) > 0) {
            /* Append to current item's content */
            int clen = (int)strlen(g_evidence_items[current].content);
            int space = (int)sizeof(g_evidence_items[current].content) - clen - 2;
            if (space > 0) {
                if (clen > 0) g_evidence_items[current].content[clen++] = '\n';
                size_t llen = strlen(line);
                if ((int)llen > space) llen = space;
                memcpy(g_evidence_items[current].content + clen, line, llen);
                g_evidence_items[current].content[clen + llen] = '\0';
            }
        }
    }

    g_evidence_count = current + 1;
    fclose(fp);
    g_data_loaded = true;
}

static void reset_evidence_data(void)
{
    g_evidence_count = 0;
    g_selected_index = 0;
    g_scroll_offset = 0;
    g_data_loaded = false;
    g_in_detail_view = false;
    g_detail_scroll = 0;
    g_wrap_item = -1;
    g_wrap_count = 0;
    g_wrap_width = -1;
    g_blink_active = -1;
}

/* ==================== DRAWING ==================== */

static void draw_list(int cols, int rows)
{
    int panel_x = 2, panel_y = 2;
    int panel_w = cols - 4, panel_h = rows - 6;

    ui_draw_panel(panel_y, panel_x, panel_w, panel_h, "EVIDENCE ARCHIVE");

    int content_rows = panel_h - 2;
    int start = g_scroll_offset;
    int end = start + content_rows;
    if (end > g_evidence_count) end = g_evidence_count;

    uint64_t now = ui_get_ticks();

    for (int i = start; i < end; i++) {
        int row = panel_y + 1 + (i - start);
        bool selected = (i == g_selected_index);
        COLORREF color = selected ? CLR_HIGHLIGHT_FG : CLR_TEXT;

        if (selected) {
            ui_fill_rect(row, panel_x + 2, panel_w - 4, 1, CLR_HIGHLIGHT_BG);
        }

        char display[256];
        snprintf(display, sizeof(display), "%s%s", selected ? "> " : "  ", g_evidence_items[i].title);
        ui_draw_text(row, panel_x + 2, display, color);
    }

    if (g_scroll_offset > 0)
        ui_draw_text_centered(panel_y, "▲ SCROLL UP ▲", CLR_MUTED);
    if (end < g_evidence_count)
        ui_draw_text_centered(panel_y + panel_h - 1, "▼ SCROLL DOWN ▼", CLR_MUTED);

    (void)now;
}

static void draw_detail(int cols, int rows)
{
    int panel_x = 2, panel_y = 2;
    int panel_w = cols - 4, panel_h = rows - 6;

    ui_draw_panel(panel_y, panel_x, panel_w, panel_h, g_evidence_items[g_selected_index].title);

    /* Re-wrap if needed */
    int cw = panel_w - 6;
    if (g_wrap_item != g_selected_index || g_wrap_width != cw) {
        wrap_selected_item(cw);
        g_detail_scroll = 0;
    }

    int content_rows = panel_h - 4;
    int start = g_detail_scroll;
    int end = start + content_rows;
    if (end > g_wrap_count) end = g_wrap_count;

    int draw_row = panel_y + 2;
    for (int i = start; i < end; i++) {
        ui_draw_text(draw_row++, panel_x + 3, g_wrap_buf[i], CLR_TEXT);
    }

    if (g_detail_scroll > 0)
        ui_draw_text_centered(panel_y, "▲ SCROLL UP ▲", CLR_MUTED);
    if (end < g_wrap_count)
        ui_draw_text_centered(panel_y + panel_h - 1, "▼ SCROLL DOWN ▼", CLR_MUTED);

    ui_draw_text_centered(panel_y + panel_h - 2, "[ESC] Back    [UP/DOWN] Scroll", CLR_MUTED);
}

void Evidence_Draw(GameState *game)
{
    (void)game;
    load_evidence_data(game->level);

    int cols = ui_get_cols();
    int rows = ui_get_rows();
    uint64_t now = ui_get_ticks();

    /* Process blink animation */
    if (g_blink_active >= 0) {
        uint32_t elapsed = (uint32_t)(now - g_blink_start);
        if (elapsed >= BLINK_TIME) {
            g_blink_active = -1;
            if (g_in_detail_view) {
                g_in_detail_view = false;
            } else {
                g_in_detail_view = true;
                g_detail_scroll = 0;
            }
        }
    }

    char header[64];
    snprintf(header, sizeof(header), "SYS::CASE_%02d // EVIDENCE ARCHIVE", game->level);
    ui_draw_text(1, 2, header, CLR_MUTED);
    ui_draw_hline(2, 2, cols - 4, CLR_BORDER);

    /* Blink flash overlay */
    if (g_blink_active >= 0) {
        uint32_t elapsed = (uint32_t)(now - g_blink_start);
        bool flash = ((elapsed / 45) % 2 == 0);
        COLORREF bg = flash ? RGB(255, 255, 255) : CLR_HIGHLIGHT_BG;
        COLORREF fg = flash ? RGB(0, 0, 0) : CLR_HIGHLIGHT_FG;
        int item_row = 2 + 1 + g_selected_index;
        ui_fill_rect(item_row, 4, cols - 8, 1, bg);
        char buf[256];
        snprintf(buf, sizeof(buf), ">> %s <<", g_evidence_items[g_selected_index].title);
        ui_draw_text(item_row, 5, buf, fg);
    } else if (g_in_detail_view) {
        draw_detail(cols, rows);
    } else {
        draw_list(cols, rows);
    }

    /* Footer with pulsing hint */
    ui_draw_hline(rows - 2, 2, cols - 4, CLR_BORDER);
    bool pulse = ((now / 500) % 2 == 0);
    COLORREF hint_clr = pulse ? CLR_MUTED : CLR_TEXT;
    const char *hint = g_in_detail_view
        ? "[ESC] Back    [UP/DOWN] Scroll"
        : "[UP/DOWN] Navigate    [ENTER] View Detail    [ESC] Back to Case";
    ui_draw_text(rows - 1, 4, hint, hint_clr);
}

void Evidence_HandleKey(GameState *game, KeyCode key)
{
    if (g_blink_active >= 0) return;

    if (g_in_detail_view) {
        int content_rows = ui_get_rows() - 10;
        int max_s = g_wrap_count - content_rows;
        if (max_s < 0) max_s = 0;

        switch (key) {
            case KEY_ESCAPE:
                g_in_detail_view = false;
                break;
            case KEY_UP:
                if (g_detail_scroll > 0) g_detail_scroll--;
                break;
            case KEY_DOWN:
                if (g_detail_scroll < max_s) g_detail_scroll++;
                break;
            default: break;
        }
        return;
    }

    int content_rows = ui_get_rows() - 8;
    int max_s = g_evidence_count - content_rows;
    if (max_s < 0) max_s = 0;

    switch (key) {
        case KEY_UP:
            if (g_selected_index > 0) {
                g_selected_index--;
                if (g_selected_index < g_scroll_offset) g_scroll_offset = g_selected_index;
            }
            break;
        case KEY_DOWN:
            if (g_selected_index < g_evidence_count - 1) {
                g_selected_index++;
                if (g_selected_index >= g_scroll_offset + content_rows)
                    g_scroll_offset = g_selected_index - content_rows + 1;
            }
            break;
        case KEY_ENTER:
            if (g_evidence_count > 0) {
                g_blink_active = g_selected_index;
                g_blink_start = ui_get_ticks();
            }
            break;
        case KEY_ESCAPE:
            reset_evidence_data();
            Game_ChangeScreen(game, game->previousScreen);
            break;
        default: break;
    }
}