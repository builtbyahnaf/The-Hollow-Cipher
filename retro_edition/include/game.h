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
#include <stddef.h>

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


/* ==================== GAME DIFFICULTY & STATUS ENUMS ==================== */
typedef enum
{
    DIFFICULTY_RECRUIT = 0,     /* Guided investigation hints                 */
    DIFFICULTY_DETECTIVE,       /* Standard retro detective experience         */
    DIFFICULTY_HARD_BOILED      /* Uncompromising noir difficulty              */
} GameDifficulty;

typedef enum
{
    SESSION_STATUS_FRESH = 0,   /* Newly initialized investigation             */
    SESSION_STATUS_ACTIVE,      /* Active case investigation underway          */
    SESSION_STATUS_PAUSED,      /* Suspended in menus                          */
    SESSION_STATUS_RESOLVED     /* All active dossiers cleared                 */
} SessionStatus;


/* ==================== GAME SETTINGS ====================
 * Player-adjustable settings.
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


/* ==================== DYNAMIC CASE RECORD ====================
 * Runtime tracking and unlock state for each investigation case.
 */
typedef struct
{
    int  case_id;          /* 1-based case index                          */
    bool unlocked;         /* true if accessible by detective             */
    bool solved;           /* true if case conclusion reached             */
    int  clues_discovered; /* number of pieces of evidence found          */
    int  best_score;       /* maximum score awarded for this case         */
} CaseRecord;


/* ==================== DYNAMIC BACKEND SESSION ====================
 * Heap-allocated session data container managed by GameState.
 * Encapsulates dynamic arrays, session UUID, and detective notebook.
 */
typedef struct
{
    SessionStatus   status;
    GameDifficulty  difficulty;
    char           *session_uuid;        /* Dynamically allocated UUID string      */
    CaseRecord     *cases;               /* Dynamically allocated CaseRecord array */
    int             total_cases;         /* Number of registered cases             */
    char           *investigation_notes; /* Dynamically allocated notes buffer     */
    size_t          notes_size;          /* Current length of notes string         */
    size_t          notes_capacity;      /* Capacity of notes buffer               */
} GameSession;


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

    GameSession *session;      /* Dynamically allocated backend session state */
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

/* ==================== SESSION & CASE HELPERS ==================== */
bool          Game_IsCaseUnlocked(const GameState *game, int case_id);
void          Game_UnlockCase(GameState *game, int case_id);
void          Game_MarkCaseSolved(GameState *game, int case_id, int score);
void          Game_AppendNote(GameState *game, const char *note);
const char   *Game_GetSessionUUID(const GameState *game);
SessionStatus Game_GetSessionStatus(const GameState *game);
void          Game_SetSessionStatus(GameState *game, SessionStatus status);

#endif /* GAME_H */