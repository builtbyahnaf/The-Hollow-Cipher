/*
 * File: home.c
 * Project: The Hollow Cipher
 *
 * Description:
 *   Main Menu / Home Screen.
 *   Provides navigation across:
 *     1. Play (Level / Case select)
 *     2. Evidence Archives
 *     3. Options / Configuration
 *     4. About & Credits
 *     5. Exit Terminal
 *   Features:
 *     - Arcade style option blink on ENTER selection
 *     - Retro CRT screen transitions
 *     - Confirmation modal dialog for quitting
 *
 * Architecture note:
 *   Frontend screen module implementing Home_Draw and Home_HandleKey.
 */

#include "screens.h"
#include "ui_engine.h"
#include <stdio.h>
#include <string.h>

/* ==================== CONSTANTS & MENU DATA ==================== */

#define MENU_ITEM_COUNT 5

static const char *g_menu_labels[MENU_ITEM_COUNT] = {
    "1. PLAY",
    "2. EVIDENCE ARCHIVE",
    "3. OPTIONS",
    "4. ABOUT & CREDITS",
    "5. EXIT"
};

static const ScreenID g_menu_screens[MENU_ITEM_COUNT] = {
    SCREEN_LEVEL_SELECT,
    SCREEN_EVIDENCE,
    SCREEN_OPTIONS,
    SCREEN_ABOUT,
    SCREEN_EXIT
};

/* ==================== SCREEN STATE ==================== */

static int      g_selected_index = 0;
static bool     g_confirm_exit   = false;
static int      g_blink_option   = -1;
static uint64_t g_blink_start    = 0;
static const uint32_t BLINK_TIME = 240; /* ms */

/* ==================== SCREEN DRAWING ==================== */

void Home_Draw(GameState *game)
{
    (void)game;

    uint64_t now = ui_get_ticks();
    int cols = ui_get_cols();
    int rows = ui_get_rows();

    /* Process finished arcade option blink */
    if (g_blink_option >= 0) {
        uint32_t elapsed = (uint32_t)(now - g_blink_start);
        if (elapsed >= BLINK_TIME) {
            int opt = g_blink_option;
            g_blink_option = -1;
            if (g_menu_screens[opt] == SCREEN_EXIT) {
                g_confirm_exit = true;
            } else {
                ui_start_transition(g_menu_screens[opt]);
            }
        }
    }

    /* Header rule */
    ui_draw_hline(2, 2, cols - 4, CLR_BORDER);

    /* Retro Title Banner */
    int banner_y = 4;
    ui_draw_text_centered(banner_y + 0, "================================================", CLR_BORDER);
    ui_draw_text_centered(banner_y + 1, "  T H E   H O L L O W   C I P H E R  ", CLR_GOLD);
    ui_draw_text_centered(banner_y + 2, "================================================", CLR_BORDER);
    ui_draw_text_centered(banner_y + 4, "[ RETRO EDITION ]", CLR_CYAN);

    /* Main Menu Panel */
    int panel_w = 46;
    int panel_h = MENU_ITEM_COUNT + 4;
    int panel_x = (cols - panel_w) / 2;
    int panel_y = banner_y + 9;

    ui_draw_panel(panel_y, panel_x, panel_w, panel_h, "MAIN MENU");

    /* Draw each menu item with arcade blink support */
    for (int i = 0; i < MENU_ITEM_COUNT; i++) {
        int item_y = panel_y + 2 + i;
        bool is_selected = (i == g_selected_index);

        if (i == g_blink_option) {
            /* Rapid arcade flashing on ENTER press (45ms alternating flash) */
            uint32_t blink_elapsed = (uint32_t)(now - g_blink_start);
            bool flash = ((blink_elapsed / 45) % 2 == 0);
            COLORREF bg = flash ? RGB(255, 255, 255) : CLR_HIGHLIGHT_BG;
            COLORREF fg = flash ? RGB(0, 0, 0) : CLR_HIGHLIGHT_FG;

            ui_fill_rect(item_y, panel_x + 3, panel_w - 6, 1, bg);
            char buf[128];
            snprintf(buf, sizeof(buf), ">> %s <<", g_menu_labels[i]);
            ui_draw_text_in_region(item_y, panel_x + 3, panel_w - 6, buf, fg);
        } else {
            ui_draw_menu_item(item_y, panel_x + 3, panel_w - 6, g_menu_labels[i], is_selected);
        }
    }

    /* Footer instruction bar with pulsing hint */
    ui_draw_hline(rows - 3, 2, cols - 4, CLR_BORDER);
    bool hint_pulse = ((now / 500) % 2 == 0);
    COLORREF hint_color = hint_pulse ? CLR_MUTED : CLR_TEXT;
    ui_draw_text(rows - 2, 4, "[UP/DOWN/1-5] Navigate    [ENTER] Select    [ESC] Exit", hint_color);

    /* Render modal overlay if Exit Confirmation is open */
    if (g_confirm_exit) {
        int modal_w = 46;
        int modal_h = 7;
        int modal_x = (cols - modal_w) / 2;
        int modal_y = (rows - modal_h) / 2;

        /* Dark shadow backdrop */
        ui_fill_rect(modal_y - 1, modal_x - 2, modal_w + 4, modal_h + 2, CLR_BG);

        /* Dialog panel */
        ui_draw_panel(modal_y, modal_x, modal_w, modal_h, "CONFIRM");
        ui_draw_text_in_region(modal_y + 2, modal_x, modal_w, "Are you sure you want to exit?", CLR_TEXT);
        ui_draw_text_in_region(modal_y + 4, modal_x, modal_w, "[Y] YES, EXIT       [N] CANCEL", CLR_YELLOW);
    }
}

/* ==================== INPUT HANDLING ==================== */

void Home_HandleKey(GameState *game, KeyCode key)
{
    /* Suppress input while arcade blink is in flight */
    if (g_blink_option >= 0) return;

    /* If modal is open, intercept confirmation keys */
    if (g_confirm_exit) {
        if (key == KEY_Y || key == KEY_ENTER) {
            Game_ChangeScreen(game, SCREEN_EXIT);
        } else if (key == KEY_N || key == KEY_ESCAPE) {
            g_confirm_exit = false;
        }
        return;
    }

    /* Standard Menu Navigation */
    switch (key) {
        case KEY_UP:
            g_selected_index = (g_selected_index - 1 + MENU_ITEM_COUNT) % MENU_ITEM_COUNT;
            break;

        case KEY_DOWN:
            g_selected_index = (g_selected_index + 1) % MENU_ITEM_COUNT;
            break;

        case KEY_1: g_selected_index = 0; break;
        case KEY_2: g_selected_index = 1; break;
        case KEY_3: g_selected_index = 2; break;
        case KEY_4: g_selected_index = 3; break;
        case KEY_5: g_selected_index = 4; break;

        case KEY_ENTER:
            if (g_menu_screens[g_selected_index] == SCREEN_EXIT) {
                g_confirm_exit = true;
            } else {
                /* Start arcade option blink animation */
                g_blink_option = g_selected_index;
                g_blink_start  = ui_get_ticks();
            }
            break;

        case KEY_ESCAPE:
            g_confirm_exit = true;
            break;

        default:
            break;
    }
}