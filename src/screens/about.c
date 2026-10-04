/*
 * File: about.c
 * Project: The Hollow Cipher
 *
 * Description:
 *   About & Credits Screen.
 *   Presents project backstory, development credits, academic attribution (KUET CSE 1-1),
 *   and software version architecture information.
 *
 * Architecture note:
 *   Frontend screen module implementing About_Draw and About_HandleKey.
 */

#include "screens.h"
#include "ui_engine.h"
#include <stdio.h>

/* ==================== SCREEN DRAWING ==================== */

/*
 * About_Draw
 *   Renders the about screen panels and development credits.
 */
void About_Draw(GameState *game)
{
    (void)game;

    int cols = ui_get_cols();
    int rows = ui_get_rows();

    /* Header rule & title */
    ui_draw_text(1, 2, "SYS::INFO // INTELLIGENCE DOSSIER & CREDITS", CLR_MUTED);
    ui_draw_hline(2, 2, cols - 4, CLR_BORDER);

    ui_draw_text_centered(4, "[ PROJECT DOSSIER ]", CLR_GOLD);

    /* Main Information Panel */
    int panel_w = 66;
    int panel_h = 16;
    int panel_x = (cols - panel_w) / 2;
    int panel_y = 6;

    ui_draw_panel(panel_y, panel_x, panel_w, panel_h, "THE HOLLOW CIPHER");

    ui_draw_text_centered(panel_y + 2, "AN INVESTIGATIVE CIPHER & FORENSICS ADVENTURE", CLR_CYAN);
    ui_draw_text_centered(panel_y + 3, "-- Retro Edition Variant v1.0.0 --", CLR_MUTED);

    ui_draw_hline(panel_y + 4, panel_x + 3, panel_w - 6, CLR_BORDER);

    /* Credits & Engineering */
    ui_draw_text(panel_y + 6, panel_x + 4, "DEVELOPMENT TEAM:", CLR_ACCENT);
    ui_draw_text(panel_y + 7, panel_x + 6, "Lead Developer : Md Ashraful Alam (Dasher)", CLR_TEXT);
    ui_draw_text(panel_y + 8, panel_x + 6, "Co-Developer   : Rahat Ul Islam (ZHR)", CLR_TEXT);

    ui_draw_text(panel_y + 10, panel_x + 4, "ACADEMIC AFFILIATION:", CLR_ACCENT);
    ui_draw_text(panel_y + 11, panel_x + 6, "Khulna University of Engineering & Technology (KUET)", CLR_GREEN);
    ui_draw_text(panel_y + 12, panel_x + 6, "Department of Computer Science & Engineering | CSE 1-1", CLR_TEXT);

    ui_draw_hline(panel_y + 13, panel_x + 3, panel_w - 6, CLR_BORDER);
    ui_draw_text_in_region(panel_y + 14, panel_x, panel_w,
                           "Engine: Pure C Decoupled Core + Win32 GDI Retro Layer", CLR_MUTED);

    /* Footer instruction */
    ui_draw_hline(rows - 3, 2, cols - 4, CLR_BORDER);
    ui_draw_text(rows - 2, 4, "Press [ESC] or [ENTER] to return to Main Menu", CLR_YELLOW);
}

/* ==================== INPUT HANDLING ==================== */

/*
 * About_HandleKey
 *   Pressing ESCAPE or ENTER (or any key) returns the user to the Main Menu.
 */
void About_HandleKey(GameState *game, KeyCode key)
{
    if (key == KEY_ESCAPE || key == KEY_ENTER || key != KEY_NONE) {
        Game_ChangeScreen(game, SCREEN_MAIN_MENU);
    }
}