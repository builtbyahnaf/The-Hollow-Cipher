/*
 * File: game.c
 * Project: The Hollow Cipher
 *
 * Description:
 *   Core game state management — pure backend.  No UI code lives here.
 *   Contains initialization, shutdown, and screen navigation logic.
 *
 *   This file includes ONLY game.h.  It must NEVER include ui_engine.h,
 *   screens.h, windows.h, or any other frontend/platform header.
 *   This is what makes the backend reusable across Retro and Raylib editions.
 */

#include "game.h"


/* ====================  Game_Init  ====================
 * Initialize every field in GameState to safe starting defaults.
 *
 * Called once from WinMain BEFORE the window is created.
 * After this call, the game is ready for the frontend to start drawing.
 */
void Game_Init(GameState *game)
{
    game->running        = true;
    game->currentScreen  = SCREEN_BOOT;    /* first thing the player sees */
    game->previousScreen = SCREEN_BOOT;
    game->level          = 1;
    game->score          = 0;

    /* --- Default settings ---
     * These live in GameState so they survive screen transitions
     * and can eventually be serialized to a save file.
     */
    game->settings.fps_limit     = 60;
    game->settings.master_volume = 80;
    game->settings.sfx_volume    = 70;
    game->settings.music_volume  = 60;
    game->settings.vsync         = 1;
}


/* ====================  Game_ChangeScreen  ====================
 * Request a transition to a different screen.
 *
 * How it works:
 *   1. Stores the current screen as previousScreen.
 *      → Screens can use game->previousScreen for "go back" (e.g., ESC).
 *   2. Sets currentScreen to the requested nextScreen.
 *      → The frontend reads this in WM_PAINT and WM_KEYDOWN
 *        to dispatch to the correct screen's Draw/HandleKey.
 *   3. If nextScreen is SCREEN_EXIT, sets running = false.
 *      → The WndProc checks this flag after key dispatch and
 *        posts WM_CLOSE to terminate the message loop.
 *
 * Note: this function does NOT draw anything.  Drawing happens
 * on the next WM_PAINT, which is triggered by InvalidateRect().
 */
void Game_ChangeScreen(GameState *game, ScreenID nextScreen)
{
    game->previousScreen = game->currentScreen;
    game->currentScreen  = nextScreen;

    /* SCREEN_EXIT is the shutdown sentinel */
    if (nextScreen == SCREEN_EXIT)
    {
        game->running = false;
    }
}


/* ====================  Game_Shutdown  ====================
 * Release any backend-owned resources.
 *
 * Called once from WinMain AFTER the message loop has exited.
 * Currently a no-op but exists as a hook for future cleanup:
 *   • Freeing loaded case data
 *   • Closing open save files
 *   • Writing settings to disk
 */
void Game_Shutdown(GameState *game)
{
    (void)game;  /* suppress unused-parameter warning — nothing to free yet */
}