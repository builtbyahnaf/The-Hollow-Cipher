/*
 * File: boot.c
 * Project: The Hollow Cipher
 *
 * Description:
 *   Retro boot / splash screen.
 *   Features:
 *     - Smooth typewriter character-by-character reveal for "THE HOLLOW CIPHER"
 *     - Blinking phosphor block caret during typing
 *     - Arcade style flashing prompt on ENTER press
 *     - Seamless CRT shutter transition into the Main Menu
 *
 * Architecture note:
 *   Frontend screen module. Implements Boot_Draw and Boot_HandleKey.
 */

#include "screens.h"
#include "ui_engine.h"
#include <stdio.h>
#include <string.h>

/* ==================== CONSTANTS & STATE ==================== */

static const char *TITLE_TEXT = "T H E   H O L L O W   C I P H E R";

static uint64_t g_boot_start_time = 0;
static bool     g_fast_forward    = false;
static int      g_blink_counter   = -1;
static uint64_t g_blink_start     = 0;
static const uint32_t BLINK_TIME  = 240; /* ms */

/* ==================== SCREEN DRAWING ==================== */

void Boot_Draw(GameState *game)
{
    (void)game;

    uint64_t now = ui_get_ticks();
    if (g_boot_start_time == 0) {
        g_boot_start_time = now;
    }

    int cols = ui_get_cols();
    int rows = ui_get_rows();
    int center_y = rows / 2;

    /* Top decorative header rule */
    
    ui_draw_hline(2, 2, cols - 4, CLR_BORDER);

    /* Decorative top title border */
    ui_draw_text_centered(center_y - 7, "================================================", CLR_BORDER);

    /* ---------- TYPEWRITER ANIMATION FOR GAME TITLE ---------- */
    uint64_t elapsed = now - g_boot_start_time;
    int total_chars = (int)strlen(TITLE_TEXT);

    /* 40ms per character -> reveals over ~1.2s */
    int chars_revealed = g_fast_forward ? total_chars : (int)(elapsed / 40);
    if (chars_revealed > total_chars) chars_revealed = total_chars;
    bool is_typing_done = (chars_revealed >= total_chars);

    char typed_buffer[128];
    memset(typed_buffer, 0, sizeof(typed_buffer));
    if (chars_revealed > 0) {
        memcpy(typed_buffer, TITLE_TEXT, (size_t)chars_revealed);
        typed_buffer[chars_revealed] = '\0';
    }

    if (!is_typing_done) {
        /* While typing, append an arcade blinking phosphor cursor block */
        bool cursor_blink = ((now / 120) % 2 == 0);
        if (cursor_blink) {
            strcat(typed_buffer, "_");
        }
        ui_draw_text_centered(center_y - 6, typed_buffer, CLR_YELLOW);
    } else {
        /* Fully revealed title in polished prestige gold */
        ui_draw_text_centered(center_y - 6, typed_buffer, CLR_GOLD);
    }

    /* Decorative bottom title border */
    ui_draw_text_centered(center_y - 5, "================================================", CLR_BORDER);

    /* Subtitle appears smoothly after typing finishes */
    if (is_typing_done) {
        ui_draw_text_centered(center_y - 4, "-- RETRO EDITION --", CLR_CYAN);

    }

    /* ---------- ARCADE BLINKING ENTER PROMPT ---------- */
    if (is_typing_done) {
        /* Check if player pressed ENTER and arcade blink is playing */
        if (g_blink_counter >= 0) {
            uint32_t blink_elapsed = (uint32_t)(now - g_blink_start);
            if (blink_elapsed >= BLINK_TIME) {
                g_blink_counter = -1;
                ui_start_transition(SCREEN_MAIN_MENU);
            } else {
                /* Rapid arcade flash (60ms cycle) */
                bool flash = ((blink_elapsed / 45) % 2 == 0);
                COLORREF prompt_clr = flash ? RGB(255, 255, 255) : CLR_GOLD;
                ui_draw_text_centered(center_y + 7, ">>> ACCESS GRANTED - LOADING TERMINAL <<<", prompt_clr);
            }
        } else {
            /* Standard idle arcade pulse */
            bool pulse = ((now / 400) % 2 == 0);
            COLORREF prompt_clr = pulse ? CLR_YELLOW : CLR_MUTED;
            ui_draw_text_centered(center_y + 7, "> PRESS [ENTER] TO ACCESS TERMINAL <", prompt_clr);
            ui_draw_text_centered(center_y + 9, "[ESC] Terminate Session", CLR_MUTED);
        }
    } else {
        /* Hint during typing */
        ui_draw_text_centered(center_y + 7, "Press [ENTER] to Skip Reveal", CLR_MUTED);
    }

    /* Footer rule */
    ui_draw_hline(rows - 3, 2, cols - 4, CLR_BORDER);
    ui_draw_text(rows - 2, 4, "Project THE HOLLOW CIPHER // KUET CSE 1-1", CLR_MUTED);
}

/* ==================== INPUT HANDLING ==================== */

void Boot_HandleKey(GameState *game, KeyCode key)
{
    if (key == KEY_ENTER) {
        int total_chars = (int)strlen(TITLE_TEXT);
        uint64_t now = ui_get_ticks();
        int chars_revealed = (int)((now - g_boot_start_time) / 40);

        /* If typing is still in progress, fast-forward to the end on first Enter */
        if (!g_fast_forward && chars_revealed < total_chars) {
            g_fast_forward = true;
            return;
        }

        /* If already revealed, start arcade blink confirmation */
        if (g_blink_counter < 0) {
            g_blink_counter = 1;
            g_blink_start   = ui_get_ticks();
        }
    } else if (key == KEY_ESCAPE) {
        Game_ChangeScreen(game, SCREEN_EXIT);
    }
}