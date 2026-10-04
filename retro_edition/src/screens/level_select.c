/*
 * File: level_select.c
 * Project: The Hollow Cipher
 *
 * Description:
 *   Level / Case Selection Screen.
 *   Displays available investigation levels with lock/unlock status.
 *   Level 1 is unlocked by default; subsequent levels unlock on completion.
 *
 * Architecture note:
 *   Frontend screen module implementing LevelSelect_Draw and LevelSelect_HandleKey.
 *   Interacts with GameState via Game_ChangeScreen().
 */

#include "screens.h"
#include "ui_engine.h"
#include <stdio.h>
#include <string.h>

/* ==================== CONSTANTS & LEVEL DATA ==================== */

#define MAX_LEVELS 3

static const char *g_level_names[MAX_LEVELS] = {
    "CASE 01: THE SILENT PATIENT",
    "CASE 02: THE MISSING NOTEBOOK",
    "CASE 03: THE PSYCHOPATH"
};

static const char *g_level_subtitles[MAX_LEVELS] = {
    "A journalist's murder, a missing notebook and a shocked patient.",
    "The investigation deepens to uncover missing notebook.",
    "A private investigation from the confidential documents."
};

/* ==================== SCREEN STATE ==================== */

static int g_selected_level = 0;

/* Helper to check if a level is unlocked.
 * For now: Level 1 (index 0) is always unlocked.
 * Future: Check game->progress or save data.
 */
static bool is_level_unlocked(int level_index)
{
    return level_index == 0;
}

/* ==================== SCREEN DRAWING ==================== */

void LevelSelect_Draw(GameState *game)
{
    (void)game;

    int cols = ui_get_cols();
    int rows = ui_get_rows();

    /* Header */

    ui_draw_hline(2, 2, cols - 4, CLR_BORDER);

    ui_draw_text_centered(4, "[ CASE SELECTION ]", CLR_GOLD);
    ui_draw_text_centered(5, "Use UP/DOWN to navigate. ENTER to open case. ESC to return.", CLR_MUTED);

    /* Level list panel */
    int panel_w = 80;
    int panel_h = MAX_LEVELS * 4 + 4;
    int panel_x = (cols - panel_w) / 2;
    int panel_y = 7;

    ui_draw_panel(panel_y, panel_x, panel_w, panel_h, "AVAILABLE CASES");

    for (int i = 0; i < MAX_LEVELS; i++) {
        int item_y = panel_y + 2 + i * 4;
        bool unlocked = is_level_unlocked(i);
        bool selected = (i == g_selected_level);

        /* Level title */
        COLORREF title_color = unlocked ? (selected ? CLR_HIGHLIGHT_FG : CLR_TEXT) : CLR_MUTED;
        if (selected && unlocked) {
            ui_fill_rect(item_y, panel_x + 2, panel_w - 4, 1, CLR_HIGHLIGHT_BG);
        }

        char prefix[8];
        snprintf(prefix, sizeof(prefix), "%s ", selected ? "> " : "  ");
        char title_buf[128];
        snprintf(title_buf, sizeof(title_buf), "%s%s", prefix, g_level_names[i]);
        ui_draw_text(item_y, panel_x + 3, title_buf, title_color);

        /* Subtitle */
        COLORREF sub_color = unlocked ? CLR_SUBTITLE : CLR_MUTED;
        ui_draw_text(item_y + 1, panel_x + 5, g_level_subtitles[i], sub_color);

        /* Lock status */
        const char *lock_text = unlocked ? "[ UNLOCKED ]" : "[ LOCKED  ]";
        COLORREF lock_color = unlocked ? CLR_GREEN : CLR_RED;
        int lock_col = panel_x + panel_w - (int)strlen(lock_text) - 4;
        ui_draw_text(item_y, lock_col, lock_text, lock_color);

        if (i < MAX_LEVELS - 1) {
            ui_draw_hline(item_y + 2, panel_x + 3, panel_w - 6, CLR_BORDER);
        }
    }

    /* Footer instruction */
    ui_draw_hline(rows - 3, 2, cols - 4, CLR_BORDER);
    ui_draw_text(rows - 2, 4, "[UP/DOWN] Select Case    [ENTER] Open    [ESC] Back to Menu", CLR_MUTED);
}

/* ==================== INPUT HANDLING ==================== */

void LevelSelect_HandleKey(GameState *game, KeyCode key)
{
    switch (key) {
        case KEY_UP:
            g_selected_level = (g_selected_level - 1 + MAX_LEVELS) % MAX_LEVELS;
            break;

        case KEY_DOWN:
            g_selected_level = (g_selected_level + 1) % MAX_LEVELS;
            break;

        case KEY_ENTER:
            if (is_level_unlocked(g_selected_level)) {
                game->level = g_selected_level + 1;
                Game_ChangeScreen(game, SCREEN_LEVEL_MAIN);
            }
            break;

        case KEY_ESCAPE:
            Game_ChangeScreen(game, SCREEN_MAIN_MENU);
            break;

        default:
            break;
    }
}