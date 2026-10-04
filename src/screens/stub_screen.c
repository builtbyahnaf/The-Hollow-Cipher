/*
 * File: stub_screen.c
 * Project: The Hollow Cipher
 *
 * Description:
 *   Universal retro placeholder screen for modules pending feature completion:
 *     - SCREEN_LEVEL_SELECT
 *     - SCREEN_INTERROGATION
 *     - SCREEN_FORENSICS
 *     - SCREEN_EVIDENCE
 *     - SCREEN_HISTORY
 *   Renders a themed dossier panel identifying the requested module, noting
 *   its implementation status, and providing immediate ESC/ENTER navigation back.
 *
 * Architecture note:
 *   Frontend utility screen. Reads game->currentScreen to render context-aware labels.
 */

#include "screens.h"
#include "ui_engine.h"
#include <stdio.h>

/* Helper to map ScreenID to human-readable title */
static const char *get_screen_name(ScreenID id)
{
    switch (id) {
        case SCREEN_LEVEL_SELECT:   return "CASE DOSSIER // LEVEL SELECT";
        case SCREEN_INTERROGATION:  return "SUSPECT INTERROGATION CHAMBER";
        case SCREEN_FORENSICS:      return "FORENSICS LAB & DECRYPTOR";
        case SCREEN_EVIDENCE:       return "EVIDENCE ARCHIVE & VAULT";
        case SCREEN_HISTORY:        return "CASE LOG & CLOSED INVESTIGATIONS";
        default:                    return "RESTRICTED CLASSIFIED ARCHIVE";
    }
}

/*
 * StubScreen_Draw
 *   Paints a retro diagnostic panel notifying the detective that the module
 *   is currently undergoing active forensic development.
 */
void StubScreen_Draw(GameState *game)
{
    int cols = ui_get_cols();
    int rows = ui_get_rows();

    const char *module_name = get_screen_name(game->currentScreen);

    /* Header rule & title */
    ui_draw_text(1, 2, "SYS::RESTRICTED // ACCESS RESTRICTED", CLR_MUTED);
    ui_draw_hline(2, 2, cols - 4, CLR_BORDER);

    /* Center Alert Box */
    int panel_w = 62;
    int panel_h = 13;
    int panel_x = (cols - panel_w) / 2;
    int panel_y = (rows - panel_h) / 2;

    ui_draw_panel(panel_y, panel_x, panel_w, panel_h, "SECURITY NOTICE");

    ui_draw_text_centered(panel_y + 2, module_name, CLR_GOLD);
    ui_draw_hline(panel_y + 3, panel_x + 4, panel_w - 8, CLR_BORDER);

    ui_draw_text_centered(panel_y + 5, "[ MODULE CURRENTLY UNDER INVESTIGATION ]", CLR_RED);
    ui_draw_text_centered(panel_y + 6, "Forensic data files are currently locked in evidence.", CLR_TEXT);
    ui_draw_text_centered(panel_y + 7, "Full interrogation and case solving logic will unlock", CLR_MUTED);
    ui_draw_text_centered(panel_y + 8, "as subsequent case files are decrypted.", CLR_MUTED);

    ui_draw_hline(panel_y + 9, panel_x + 4, panel_w - 8, CLR_BORDER);
    ui_draw_text_centered(panel_y + 11, "> Press [ESC] or [ENTER] to return <", CLR_YELLOW);

    /* Footer rule */
    ui_draw_hline(rows - 3, 2, cols - 4, CLR_BORDER);
    ui_draw_text(rows - 2, 4, "STATUS: PENDING DECRYPTION // DASHER OS ENCRYPTION PROTOCOL", CLR_MUTED);
}

/*
 * StubScreen_HandleKey
 *   Pressing ESCAPE or ENTER returns the user to the previous screen.
 *   Uses game->previousScreen which is set by Game_ChangeScreen.
 */
void StubScreen_HandleKey(GameState *game, KeyCode key)
{
    if (key == KEY_ESCAPE || key == KEY_ENTER) {
        Game_ChangeScreen(game, game->previousScreen);
    }
}
