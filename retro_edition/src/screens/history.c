/*
 * File: history.c
 * Project: The Hollow Cipher
 *
 * Description:
 *   Past History / Case Log Screen.
 *   Loads data/level_X/past_history.txt, parses [Name] sections.
 *   Each section title becomes a list entry; detail shows full text.
 *   Features arcade blink animation on selection.
 *
 * Architecture note:
 *   Frontend screen module implementing History_Draw and History_HandleKey.
 */

#include "screens.h"
#include "ui_engine.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

/* ==================== CONSTANTS ==================== */

#define MAX_ENTRIES 30
#define MAX_WRAP_LINES 300
#define MAX_LINE_LENGTH 512

/* ==================== SCREEN STATE ==================== */

typedef struct {
    char title[128];
    char content[2048];
} Entry;

static Entry g_entries[MAX_ENTRIES];
static int g_count = 0;
static int g_selected = 0;
static int g_scroll = 0;
static bool g_data_loaded = false;
static bool g_in_detail = false;
static int g_detail_scroll = 0;

static char g_wrap_buf[MAX_WRAP_LINES][MAX_LINE_LENGTH];
static int g_wrap_count = 0;
static int g_wrap_item = -1;
static int g_wrap_width = -1;

static int      g_blink_active = -1;
static uint64_t g_blink_start  = 0;
static const uint32_t BLINK_TIME = 240;

/* ==================== TEXT WRAPPING ==================== */

static int wrap_single_line(const char *input, char out[][MAX_LINE_LENGTH], int max_out, int max_width)
{
    if (!input || max_width <= 0) return 0;
    int len = (int)strlen(input);
    if (len == 0) { out[0][0] = '\0'; return 1; }
    if (len <= max_width) {
        size_t c = (len < MAX_LINE_LENGTH - 1) ? (size_t)len : MAX_LINE_LENGTH - 1;
        memcpy(out[0], input, c); out[0][c] = '\0';
        return 1;
    }
    int cnt = 0;
    const char *start = input, *end = input + len;
    while (start < end && cnt < max_out) {
        int rem = (int)(end - start);
        int take = (rem > max_width) ? max_width : rem;
        if (take == max_width && rem > max_width) {
            const char *sp = start + max_width - 1;
            while (sp > start && *sp != ' ') sp--;
            if (sp > start) take = (int)(sp - start);
        }
        if (take > MAX_LINE_LENGTH - 1) take = MAX_LINE_LENGTH - 1;
        memcpy(out[cnt], start, take); out[cnt][take] = '\0';
        start += take;
        while (start < end && *start == ' ') start++;
        cnt++;
    }
    return cnt;
}

static void wrap_selected_item(int max_width)
{
    if (g_selected < 0 || g_selected >= g_count) return;
    const char *text = g_entries[g_selected].content;
    int total = 0;
    const char *ptr = text;
    char line[MAX_LINE_LENGTH];

    while (*ptr && total < MAX_WRAP_LINES) {
        int len = 0;
        while (*ptr && *ptr != '\n' && len < MAX_LINE_LENGTH - 1) line[len++] = *ptr++;
        line[len] = '\0';
        if (*ptr == '\n') ptr++;
        int w = wrap_single_line(line, &g_wrap_buf[total], MAX_WRAP_LINES - total, max_width);
        total += w;
    }
    g_wrap_count = total;
    g_wrap_item = g_selected;
    g_wrap_width = max_width;
}

/* ==================== FILE LOADING ==================== */

static void load_data(int level)
{
    if (g_data_loaded) return;
    char path[256];
    snprintf(path, sizeof(path), "data/level_%d/past_history.txt", level);

    FILE *fp = fopen(path, "r");
    if (!fp) {
        snprintf(g_entries[0].title, sizeof(g_entries[0].title), "ERROR");
        snprintf(g_entries[0].content, sizeof(g_entries[0].content), "Could not load: %s", path);
        g_count = 1;
        g_data_loaded = true;
        return;
    }

    char line[MAX_LINE_LENGTH];
    int cur = -1;
    bool in_section = false;

    while (fgets(line, sizeof(line), fp) && cur < MAX_ENTRIES - 1) {
        line[strcspn(line, "\n")] = '\0';

        if (strncmp(line, "[Past History]", 14) == 0) { in_section = true; continue; }
        if (!in_section) continue;

        /* New section header: line starts with '[' */
        if (strlen(line) >= 3 && line[0] == '[') {
            cur++;
            if (cur >= MAX_ENTRIES) break;
            size_t len = strlen(line);
            if (len >= sizeof(g_entries[cur].title)) len = sizeof(g_entries[cur].title) - 1;
            memcpy(g_entries[cur].title, line, len);
            g_entries[cur].title[len] = '\0';
            g_entries[cur].content[0] = '\0';
        } else if (cur >= 0 && strlen(line) > 0) {
            int clen = (int)strlen(g_entries[cur].content);
            int space = (int)sizeof(g_entries[cur].content) - clen - 2;
            if (space > 0) {
                if (clen > 0) g_entries[cur].content[clen++] = '\n';
                size_t llen = strlen(line);
                if ((int)llen > space) llen = space;
                memcpy(g_entries[cur].content + clen, line, llen);
                g_entries[cur].content[clen + llen] = '\0';
            }
        }
    }

    g_count = cur + 1;
    fclose(fp);
    g_data_loaded = true;
}

static void reset_data(void)
{
    g_count = 0; g_selected = 0; g_scroll = 0;
    g_data_loaded = false; g_in_detail = false;
    g_detail_scroll = 0; g_wrap_item = -1; g_wrap_count = 0; g_wrap_width = -1;
    g_blink_active = -1;
}

/* ==================== DRAWING ==================== */

static void draw_list(int cols, int rows)
{
    int px = 2, py = 2, pw = cols - 4, ph = rows - 6;
    ui_draw_panel(py, px, pw, ph, "CASE LOG & PERSONNEL FILES");

    int crows = ph - 2;
    int start = g_scroll, end = start + crows;
    if (end > g_count) end = g_count;

    for (int i = start; i < end; i++) {
        int r = py + 1 + (i - start);
        bool sel = (i == g_selected);
        COLORREF clr = sel ? CLR_HIGHLIGHT_FG : CLR_TEXT;
        if (sel) ui_fill_rect(r, px + 2, pw - 4, 1, CLR_HIGHLIGHT_BG);
        char d[256];
        snprintf(d, sizeof(d), "%s%s", sel ? "> " : "  ", g_entries[i].title);
        ui_draw_text(r, px + 2, d, clr);
    }

    if (g_scroll > 0) ui_draw_text_centered(py, "▲ SCROLL UP ▲", CLR_MUTED);
    if (end < g_count) ui_draw_text_centered(py + ph - 1, "▼ SCROLL DOWN ▼", CLR_MUTED);
}

static void draw_detail(int cols, int rows)
{
    int px = 2, py = 2, pw = cols - 4, ph = rows - 6;
    ui_draw_panel(py, px, pw, ph, g_entries[g_selected].title);

    int cw = pw - 6;
    if (g_wrap_item != g_selected || g_wrap_width != cw) {
        wrap_selected_item(cw);
        g_detail_scroll = 0;
    }

    int crows = ph - 4;
    int start = g_detail_scroll, end = start + crows;
    if (end > g_wrap_count) end = g_wrap_count;

    int dr = py + 2;
    for (int i = start; i < end; i++) {
        COLORREF clr = CLR_TEXT;
        if (strstr(g_wrap_buf[i], "Investigative Direction") != NULL) clr = CLR_ACCENT;
        ui_draw_text(dr++, px + 3, g_wrap_buf[i], clr);
    }

    if (g_detail_scroll > 0) ui_draw_text_centered(py, "▲ SCROLL UP ▲", CLR_MUTED);
    if (end < g_wrap_count) ui_draw_text_centered(py + ph - 1, "▼ SCROLL DOWN ▼", CLR_MUTED);
    ui_draw_text_centered(py + ph - 2, "[ESC] Back    [UP/DOWN] Scroll", CLR_MUTED);
}

void History_Draw(GameState *game)
{
    (void)game;
    load_data(game->level);

    int cols = ui_get_cols(), rows = ui_get_rows();
    uint64_t now = ui_get_ticks();

    if (g_blink_active >= 0) {
        uint32_t el = (uint32_t)(now - g_blink_start);
        if (el >= BLINK_TIME) {
            g_blink_active = -1;
            g_in_detail = true;
            g_detail_scroll = 0;
        }
    }

    char hdr[64];
    snprintf(hdr, sizeof(hdr), "SYS::CASE_%02d // PERSONNEL & CASE HISTORY", game->level);
    ui_draw_text(1, 2, hdr, CLR_MUTED);
    ui_draw_hline(2, 2, cols - 4, CLR_BORDER);

    if (g_blink_active >= 0) {
        uint32_t el = (uint32_t)(now - g_blink_start);
        bool fl = ((el / 45) % 2 == 0);
        COLORREF bg = fl ? RGB(255, 255, 255) : CLR_HIGHLIGHT_BG;
        COLORREF fg = fl ? RGB(0, 0, 0) : CLR_HIGHLIGHT_FG;
        int ir = 2 + 1 + g_selected;
        ui_fill_rect(ir, 4, cols - 8, 1, bg);
        char b[256];
        snprintf(b, sizeof(b), ">> %s <<", g_entries[g_selected].title);
        ui_draw_text(ir, 5, b, fg);
    } else if (g_in_detail) {
        draw_detail(cols, rows);
    } else {
        draw_list(cols, rows);
    }

    ui_draw_hline(rows - 2, 2, cols - 4, CLR_BORDER);
    bool pulse = ((now / 500) % 2 == 0);
    COLORREF hc = pulse ? CLR_MUTED : CLR_TEXT;
    const char *hint = g_in_detail
        ? "[ESC] Back    [UP/DOWN] Scroll"
        : "[UP/DOWN] Navigate    [ENTER] View Profile    [ESC] Back to Case";
    ui_draw_text(rows - 1, 4, hint, hc);
}

void History_HandleKey(GameState *game, KeyCode key)
{
    if (g_blink_active >= 0) return;

    if (g_in_detail) {
        int crows = ui_get_rows() - 10;
        int ms = g_wrap_count - crows;
        if (ms < 0) ms = 0;
        switch (key) {
            case KEY_ESCAPE: g_in_detail = false; break;
            case KEY_UP: if (g_detail_scroll > 0) g_detail_scroll--; break;
            case KEY_DOWN: if (g_detail_scroll < ms) g_detail_scroll++; break;
            default: break;
        }
        return;
    }

    int crows = ui_get_rows() - 8;
    int ms = g_count - crows;
    if (ms < 0) ms = 0;

    switch (key) {
        case KEY_UP:
            if (g_selected > 0) {
                g_selected--;
                if (g_selected < g_scroll) g_scroll = g_selected;
            }
            break;
        case KEY_DOWN:
            if (g_selected < g_count - 1) {
                g_selected++;
                if (g_selected >= g_scroll + crows) g_scroll = g_selected - crows + 1;
            }
            break;
        case KEY_ENTER:
            if (g_count > 0) {
                g_blink_active = g_selected;
                g_blink_start = ui_get_ticks();
            }
            break;
        case KEY_ESCAPE:
            reset_data();
            Game_ChangeScreen(game, game->previousScreen);
            break;
        default: break;
    }
}