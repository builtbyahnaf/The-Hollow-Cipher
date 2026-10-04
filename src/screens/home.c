/*
 * File: home.c
 * Project: The Hollow Cipher
 *
 * Description:
 *   Main Menu / Home Screen.
 *   Provides navigation across:
 *     1. New Investigation (Level / Case select)
 *     2. Evidence Archives
 *     3. Options / Configuration
 *     4. About & Credits
 *     5. Exit Terminal
 *   Includes in-GUI confirmation modal dialog for exiting.
 *
 * Architecture note:
 *   Frontend screen module implementing Home_Draw and Home_HandleKey.
 *   Interacts with GameState strictly via Game_ChangeScreen().
 */

#include "screens.h"
#include "ui_engine.h"
#include <stdio.h>
#include <string.h>

/* ==================== CONSTANTS & MENU DATA ==================== */

#define MENU_ITEM_COUNT 5

//To do: make it not go to "evidence archive typa shit"
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

static int  g_selected_index = 0;
static bool g_confirm_exit   = false;

/* ==================== SCREEN DRAWING ==================== */

/*
 * Home_Draw
 *   Paints the retro main terminal menu, including header logo,
 *   menu choices with highlight selection bar, status metrics, and exit modal.
 */
void Home_Draw(GameState *game)
{
    (void)game;

    int cols = ui_get_cols();
    int rows = ui_get_rows();

    /* Header rule & title */
    ui_draw_hline(2, 2, cols - 4, CLR_BORDER);

    /* Retro Title ASCII Banner */
    int banner_y = 4;
    /*
    ui_draw_text_centered(banner_y + 0, "  _____ _            _   _       _ _                 ", CLR_GOLD);
    ui_draw_text_centered(banner_y + 1, " |_   _| |__   ___  | | | | ___ | | | _____      __  ", CLR_GOLD);
    ui_draw_text_centered(banner_y + 2, "   | | | '_ \\ / _ \\ | |_| |/ _ \\| | |/ _ \\ \\ /\\ / /  ", CLR_GOLD);
    ui_draw_text_centered(banner_y + 3, "   | | | | | |  __/ |  _  | (_) | | | (_) \\ V  V /   ", CLR_GOLD);
    ui_draw_text_centered(banner_y + 4, "   |_| |_| |_|\\___| |_| |_|\\___/|_|_|\\___/ \\_/\\_/    ", CLR_GOLD);
    ui_draw_text_centered(banner_y + 5, "    ____ _____ ____  _   _ _____ ____               ", CLR_GOLD);
    ui_draw_text_centered(banner_y + 6, "   / ___|_   _|  _ \\| | | | ____|  _ \\              ", CLR_GOLD);
    ui_draw_text_centered(banner_y + 7, "  | |     | | | |_) | |_| |  _| | |_) |             ", CLR_GOLD);
    ui_draw_text_centered(banner_y + 8, "  | |___  | | |  __/|  _  | |___|  _ <              ", CLR_GOLD);
    ui_draw_text_centered(banner_y + 9, "   \\____| |_| |_|   |_| |_|_____|_| \\_\\             ", CLR_GOLD);

    ui_draw_text_centered(banner_y + 0, " _____ _   _ _____   _  _  ___  _     _     _____  _   _ ", CLR_GOLD);
    ui_draw_text_centered(banner_y + 1, " |_   _| | | | ____| | || |/ _ \\| |   | |   / / _ \\| | | |", CLR_GOLD);
    ui_draw_text_centered(banner_y + 2, "  | | | |_| |  _|   | || | | | | |   | |  / /| | | | |  | |", CLR_GOLD);
    ui_draw_text_centered(banner_y + 3, "   | | |  _  | |___  | __ | |_| | |___| |___\\ \\| |_| | |/\\| |", CLR_GOLD);
    ui_draw_text_centered(banner_y + 4, "   |_| |_| |_|_____| |_||_|\\___/|_____|_____|\\_\\\\___/|__/\\__|", CLR_GOLD);
    ui_draw_text_centered(banner_y + 5, "   ____ ___ ____  _   _ _____ ____                        ", CLR_GOLD);
    ui_draw_text_centered(banner_y + 6, "  / ___|_ _|  _ \\| | | | ____|  _ \\                       ", CLR_GOLD);
    ui_draw_text_centered(banner_y + 7, " | |    | || |_) | |_| |  _| | |_) |                      ", CLR_GOLD);
    ui_draw_text_centered(banner_y + 8, " | |___ | ||  __/|  _  | |___|  _ <                       ", CLR_GOLD);
    ui_draw_text_centered(banner_y + 9, "  \\____|___|_|   |_| |_|_____|_| \\_\\                      ", CLR_GOLD);
*/
    ui_draw_text_centered(banner_y + 0, " T H E   H O L L O W   C I P H E R", CLR_GOLD);
    ui_draw_text_centered(banner_y + 6, "[ RETRO EDITION ]", CLR_CYAN);

    /* Main Menu Panel */
    int panel_w = 46;
    int panel_h = MENU_ITEM_COUNT + 4;
    int panel_x = (cols - panel_w) / 2;
    int panel_y = banner_y + 13;

    ui_draw_panel(panel_y, panel_x, panel_w, panel_h, "MENU");

    /* Draw each menu item with highlight bar for currently selected item */
    for (int i = 0; i < MENU_ITEM_COUNT; i++) {
        int item_y = panel_y + 2 + i;
        bool is_selected = (i == g_selected_index);
        ui_draw_menu_item(item_y, panel_x + 3, panel_w - 6, g_menu_labels[i], is_selected);
    }

    /* Footer instruction bar */
    ui_draw_hline(rows - 3, 2, cols - 4, CLR_BORDER);
    ui_draw_text(rows - 2, 4, "[UP/DOWN] Navigate    [ENTER] Select    [ESC] Exit", CLR_MUTED);

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
        ui_draw_text_in_region(modal_y + 2, modal_x, modal_w, "Are you sure to exit the game?", CLR_TEXT);
        ui_draw_text_in_region(modal_y + 4, modal_x, modal_w, "[Y] YES, EXIT       [N] CANCEL", CLR_YELLOW);
    }
}

/* ==================== INPUT HANDLING ==================== */

/*
 * Home_HandleKey
 *   Handles keyboard navigation in the main menu:
 *   - Up/Down arrow selection
 *   - Quick numeric keys (1 through 5)
 *   - Enter key execution
 *   - Exit confirmation modal toggles
 */
void Home_HandleKey(GameState *game, KeyCode key)
{
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
                Game_ChangeScreen(game, g_menu_screens[g_selected_index]);
            }
            break;

        case KEY_ESCAPE:
            g_confirm_exit = true;
            break;

        default:
            break;
    }
}