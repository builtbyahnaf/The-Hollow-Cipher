/*
 * File: game.h
 * Project: The Hollow Cipher
 *
 * Description:
 *   Core game state and lifecycle declarations.  This is the BACKEND header.
 *
 *   ARCHITECTURE RULE — enforced throughout the project:
 *     • The frontend MAY include this header.
 *     • This header must NEVER include any UI or frontend header
 *       (no ui_engine.h, no windows.h, no screens.h).
 *
 *   This guarantees the same backend compiles unchanged for the
 *   Retro (Win32 + GDI) edition AND the future Raylib-based edition.
 */

#ifndef GAME_H
#define GAME_H

#include <stdbool.h>

/* ==================== SCREEN IDENTIFIERS ====================
 * Every screen the player can visit has a unique ID.
 * The frontend reads game->currentScreen to decide which screen
 * to draw and which input handler to invoke.
 *
 * Adding a new screen:
 *   1. Add an entry here.
 *   2. Create its _Draw / _HandleKey functions.
 *   3. Register it in the dispatch tables in main.c.
 */
typedef enum
{
    SCREEN_BOOT,           /* Splash / startup animation screen          */
    SCREEN_MAIN_MENU,      /* Main menu: play, options, about, exit      */
    SCREEN_LEVEL_SELECT,   /* Level / case selection         (stub)      */
    SCREEN_OPTIONS,        /* Player-adjustable settings                 */
    SCREEN_ABOUT,          /* Credits and project information            */
    SCREEN_INTERROGATION,  /* Interrogation gameplay scene   (stub)      */
    SCREEN_FORENSICS,      /* Forensics lab gameplay scene   (stub)      */
    SCREEN_EVIDENCE,       /* Evidence archive browser       (stub)      */
    SCREEN_HISTORY,        /* Completed case history log     (stub)      */
    SCREEN_LEVEL_MAIN,     /* Level/case main screen                   */
    SCREEN_EXIT            /* Sentinel — triggers app shutdown           */
} ScreenID;


/* ==================== GAME SETTINGS ====================
 * Player-configurable options.
 * Stored inside GameState so they:
 *   • Persist across screen transitions within a session
 *   • Travel with the game state when save/load is implemented
 *   • Can be reused by both Retro and Raylib editions
 */
typedef struct
{
    int fps_limit;         /* Target FPS cap: 30 – 144          (default 60)  */
    int master_volume;     /* Master audio volume: 0 – 100      (default 80)  */
    int sfx_volume;        /* Sound effects volume: 0 – 100     (default 70)  */
    int music_volume;      /* Background music volume: 0 – 100  (default 60)  */
    int vsync;             /* Vertical sync toggle: 0=off, 1=on (default 1)   */
} GameSettings;


/* ==================== GAME STATE ====================
 * The single source of truth for everything the game needs to know.
 * Passed by pointer to ALL backend and frontend systems.
 *   • The backend initializes and mutates it.
 *   • The frontend reads it for rendering decisions.
 */
typedef struct
{
    bool running;              /* false  →  the message loop should exit       */
    ScreenID currentScreen;    /* which screen the frontend is currently showing */
    ScreenID previousScreen;   /* remembered for "go back" navigation         */

    int level;                 /* current investigation level (1-based)       */
    int score;                 /* accumulated player score                    */

    GameSettings settings;     /* player-adjustable settings (see above)      */
} GameState;


/* ==================== GAME LIFECYCLE ====================
 * These three functions form the backbone of the application.
 *   WinMain calls them in order:
 *     Game_Init  →  [message loop runs]  →  Game_Shutdown
 */

/* Initialize all GameState fields to safe starting defaults.
 * Must be called once at application startup, BEFORE creating the window. */
void Game_Init(GameState *game);

/* Release any backend-allocated resources.
 * Must be called once at application shutdown, AFTER the message loop exits. */
void Game_Shutdown(GameState *game);


/* ==================== SCREEN NAVIGATION ====================
 * Game_ChangeScreen — Request a transition to a different screen.
 *
 * Saves the current screen into previousScreen so that screens can
 * implement "go back" behavior (e.g., ESC in Options → Main Menu).
 *
 * Setting nextScreen to SCREEN_EXIT also sets running = false,
 * which signals the Win32 message loop to post WM_CLOSE.
 */
void Game_ChangeScreen(GameState *game, ScreenID nextScreen);

#endif /* GAME_H */