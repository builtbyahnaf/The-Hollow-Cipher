/*
 * File: boot.c
 * Project: The Hollow Cipher
 *
 * Description:
 *   Retro boot / splash screen.
 *   Simulates a cold boot sequence of the retro investigation terminal ("DASHER OS").
 *   Displays cipher logos, system diagnosis status, and prompts the investigator
 *   to press ENTER to initiate terminal access.
 *
 * Architecture note:
 *   Frontend screen module. Implements the Screen contract (Boot_Draw, Boot_HandleKey).
 *   Reads state from GameState, but all rendering passes through ui_engine primitives.
 */

#include "screens.h"
#include "ui_engine.h"
#include <stdio.h>

/* ==================== SCREEN DRAWING ==================== */

/*
 * Boot_Draw
 *   Renders the boot sequence layout:
 *   - Diagnostic header
 *   - Retro ASCII logo / Title
 *   - System status box
 *   - Enter prompt
 */
void Boot_Draw(GameState *game)
{
    (void)game;

    int cols = ui_get_cols();
    int rows = ui_get_rows();

    int center_y = rows / 2;

    /* Top system status header */
    //ui_draw_text(1, 2, "[ DASHER TERMINAL ARCHITECTURE v3.7.1 - COLD BOOT ]", CLR_MUTED);
    ui_draw_hline(2, 2, cols - 4, CLR_BORDER);

    /* Main Title and Badge */
    ui_draw_text_centered(center_y - 7, "================================================", CLR_BORDER);
    ui_draw_text_centered(center_y - 6, "  T H E   H O L L O W   C I P H E R  ", CLR_GOLD);
    ui_draw_text_centered(center_y - 5, "================================================", CLR_BORDER);
    ui_draw_text_centered(center_y - 4, "-- RETRO EDITION --", CLR_CYAN);

    /* Diagnostic status panel */
    int panel_w = 56;
    int panel_h = 7;
    int panel_x = (cols - panel_w) / 2;
    int panel_y = center_y - 2;

    (void)panel_w; (void)panel_h; (void)panel_x; (void)panel_y;

    /*
    ui_draw_panel(panel_y, panel_x, panel_w, panel_h, "BOOT DIAGNOSTICS");
    ui_draw_text(panel_y + 2, panel_x + 3, "> KERNEL INTEGRITY........ [OK]", CLR_GREEN);
    ui_draw_text(panel_y + 3, panel_x + 3, "> CIPHER VAULT PROTOCOL... [ONLINE]", CLR_GREEN);
    ui_draw_text(panel_y + 4, panel_x + 3, "> ARCHIVE ENCRYPTION...... [AES-Retro]", CLR_SUBTITLE);
    ui_draw_text(panel_y + 5, panel_x + 3, "> SESSION STATE........... [READY]", CLR_ACCENT);
    */
    /* Action prompt */
    ui_draw_text_centered(center_y + 7, "> PRESS [ENTER] TO ENTER GAME <", CLR_YELLOW);
    ui_draw_text_centered(center_y + 9, "Press ESC to Exit", CLR_MUTED);

    /* Footer rule */
    ui_draw_hline(rows - 3, 2, cols - 4, CLR_BORDER);
   // ui_draw_text(rows - 2, 2, "CONFIDENTIAL // LAW ENFORCEMENT & DETECTIVE DIVISION ONLY", CLR_MUTED);
}

/* ==================== INPUT HANDLING ==================== */

/*
 * Boot_HandleKey
 *   Processes keyboard input during the boot screen.
 *   Pressing ENTER takes the user to the Main Menu.
 *   Pressing ESCAPE signals application exit.
 */
void Boot_HandleKey(GameState *game, KeyCode key)
{
    if (key == KEY_ENTER) {
        Game_ChangeScreen(game, SCREEN_MAIN_MENU);
    } else if (key == KEY_ESCAPE) {
        Game_ChangeScreen(game, SCREEN_EXIT);
    }
}