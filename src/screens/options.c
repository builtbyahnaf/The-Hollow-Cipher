/*
 * File: options.c
 * Project: The Hollow Cipher
 *
 * Description:
 *   Terminal Options & Configuration Screen.
 *   Directly manipulates GameSettings stored in GameState:
 *     - Target Frame Rate (30, 60, 120, 144 FPS)
 *     - Master Volume (0% - 100%)
 *     - Sound Effects Volume (0% - 100%)
 *     - Music Volume (0% - 100%)
 *     - VSync (Enabled / Disabled)
 *
 * Architecture note:
 *   Frontend screen module. It modifies game->settings (backend data structure),
 *   demonstrating clean separation: UI controls the backend state, but does not embed
 *   any logic that would break outside of Win32.
 */

#include "screens.h"
#include "ui_engine.h"
#include <stdio.h>

/* ==================== CONSTANTS ==================== */

#define SETTING_ROWS 5

static const char *g_setting_names[SETTING_ROWS] = {
    "FPS LIMIT",
    "MASTER AUDIO",
    "SOUND EFFECTS",
    "MUSIC VOLUME",
    "VERTICAL SYNC"
};

static int g_selected_row = 0;

/* ==================== SCREEN DRAWING ==================== */

/*
 * Options_Draw
 *   Paints the retro options panel, displaying current settings values,
 *   navigation hints, and adjustable sliders/switches.
 */
void Options_Draw(GameState *game)
{
    int cols = ui_get_cols();
    int rows = ui_get_rows();

    /* Header rule & title */
    ui_draw_text(1, 2, "SYS::CONFIG // HARDWARE & AUDIO PROFILE", CLR_MUTED);
    ui_draw_hline(2, 2, cols - 4, CLR_BORDER);

    ui_draw_text_centered(4, "[ TERMINAL CONFIGURATION ]", CLR_GOLD);
    ui_draw_text_centered(5, "Adjust investigation workstation preferences", CLR_MUTED);

    /* Options Panel */
    int panel_w = 60;
    int panel_h = SETTING_ROWS * 2 + 5;
    int panel_x = (cols - panel_w) / 2;
    int panel_y = 7;

    ui_draw_panel(panel_y, panel_x, panel_w, panel_h, "PREFERENCES");

    /* Prepare value strings from GameState settings */
    char value_buffers[SETTING_ROWS][32];

    snprintf(value_buffers[0], sizeof(value_buffers[0]), "< %d FPS >", game->settings.fps_limit);
    snprintf(value_buffers[1], sizeof(value_buffers[1]), "< %d %% >", game->settings.master_volume);
    snprintf(value_buffers[2], sizeof(value_buffers[2]), "< %d %% >", game->settings.sfx_volume);
    snprintf(value_buffers[3], sizeof(value_buffers[3]), "< %d %% >", game->settings.music_volume);
    snprintf(value_buffers[4], sizeof(value_buffers[4]), "< %s >", game->settings.vsync ? "ENABLED" : "DISABLED");

    /* Render each setting row */
    for (int i = 0; i < SETTING_ROWS; i++) {
        int item_y = panel_y + 2 + (i * 2);
        bool is_selected = (i == g_selected_row);

        COLORREF label_color = is_selected ? CLR_HIGHLIGHT_FG : CLR_TEXT;
        COLORREF val_color   = is_selected ? CLR_ACCENT : CLR_CYAN;

        if (is_selected) {
            /* Highlight backdrop for active setting row */
            ui_fill_rect(item_y, panel_x + 2, panel_w - 4, 1, CLR_HIGHLIGHT_BG);
            char label[64];
            snprintf(label, sizeof(label), " > %s", g_setting_names[i]);
            ui_draw_text(item_y, panel_x + 3, label, label_color);
        } else {
            char label[64];
            snprintf(label, sizeof(label), "   %s", g_setting_names[i]);
            ui_draw_text(item_y, panel_x + 3, label, label_color);
        }

        /* Right-align value string */
        int val_len = (int)strlen(value_buffers[i]);
        int val_col = panel_x + panel_w - val_len - 4;
        ui_draw_text(item_y, val_col, value_buffers[i], val_color);
    }

    /* Internal panel hint */
    ui_draw_hline(panel_y + panel_h - 3, panel_x + 2, panel_w - 4, CLR_BORDER);
    ui_draw_text_in_region(panel_y + panel_h - 2, panel_x, panel_w,
                           "[LEFT/RIGHT] Modify Value", CLR_MUTED);

    /* Footer instruction */
    ui_draw_hline(rows - 3, 2, cols - 4, CLR_BORDER);
    ui_draw_text(rows - 2, 4, "[UP/DOWN] Select Setting    [LEFT/RIGHT] Change    [ESC/ENTER] Save & Return", CLR_MUTED);
}

/* ==================== INPUT HANDLING ==================== */

/*
 * Options_HandleKey
 *   Processes options navigation and value tuning:
 *   - Up/Down changes selected setting
 *   - Left/Right increments or decrements the corresponding setting
 *   - Enter/Escape returns to the Main Menu
 */
void Options_HandleKey(GameState *game, KeyCode key)
{
    switch (key) {
        case KEY_UP:
            g_selected_row = (g_selected_row - 1 + SETTING_ROWS) % SETTING_ROWS;
            break;

        case KEY_DOWN:
            g_selected_row = (g_selected_row + 1) % SETTING_ROWS;
            break;

        case KEY_LEFT:
            switch (g_selected_row) {
                case 0: /* FPS Limit */
                    if (game->settings.fps_limit == 144) game->settings.fps_limit = 120;
                    else if (game->settings.fps_limit == 120) game->settings.fps_limit = 60;
                    else if (game->settings.fps_limit == 60) game->settings.fps_limit = 30;
                    break;
                case 1: /* Master Volume */
                    if (game->settings.master_volume >= 5) game->settings.master_volume -= 5;
                    break;
                case 2: /* SFX Volume */
                    if (game->settings.sfx_volume >= 5) game->settings.sfx_volume -= 5;
                    break;
                case 3: /* Music Volume */
                    if (game->settings.music_volume >= 5) game->settings.music_volume -= 5;
                    break;
                case 4: /* VSync */
                    game->settings.vsync = !game->settings.vsync;
                    break;
            }
            break;

        case KEY_RIGHT:
            switch (g_selected_row) {
                case 0: /* FPS Limit */
                    if (game->settings.fps_limit == 30) game->settings.fps_limit = 60;
                    else if (game->settings.fps_limit == 60) game->settings.fps_limit = 120;
                    else if (game->settings.fps_limit == 120) game->settings.fps_limit = 144;
                    break;
                case 1: /* Master Volume */
                    if (game->settings.master_volume <= 95) game->settings.master_volume += 5;
                    break;
                case 2: /* SFX Volume */
                    if (game->settings.sfx_volume <= 95) game->settings.sfx_volume += 5;
                    break;
                case 3: /* Music Volume */
                    if (game->settings.music_volume <= 95) game->settings.music_volume += 5;
                    break;
                case 4: /* VSync */
                    game->settings.vsync = !game->settings.vsync;
                    break;
            }
            break;

        case KEY_ENTER:
        case KEY_ESCAPE:
            Game_ChangeScreen(game, SCREEN_MAIN_MENU);
            break;

        default:
            break;
    }
}