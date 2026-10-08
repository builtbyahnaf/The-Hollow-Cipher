/*
 * File: game.c
 * Project: The Hollow Cipher
 *
 * Description:
 *   Core game state management — pure backend.  No UI code lives here.
 *   Contains initialization, shutdown, dynamic session allocation,
 *   and screen navigation logic.
 *
 *   This file includes ONLY standard C headers and game.h.
 *   It must NEVER include ui_engine.h, screens.h, windows.h, or any
 *   other frontend/platform header.
 */

#include "game.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#define DEFAULT_CASE_COUNT 3
#define INITIAL_NOTES_CAP  512

/* ====================  Game_Init  ====================
 * Initialize every field in GameState to safe starting defaults
 * and dynamically allocate the backend session container.
 *
 * Called once from WinMain BEFORE the window is created.
 */
void Game_Init(GameState *game)
{
    if (!game) return;

    game->running        = true;
    game->currentScreen  = SCREEN_BOOT;    /* first thing the player sees */
    game->previousScreen = SCREEN_BOOT;
    game->level          = 1;
    game->score          = 0;

    /* --- Default settings --- */
    game->settings.fps_limit     = 60;
    game->settings.master_volume = 80;
    game->settings.sfx_volume    = 70;
    game->settings.music_volume  = 60;
    game->settings.vsync         = 1;

    /* --- Dynamic Session Allocation ---
     * Allocates the persistent heap data structures for active cases,
     * session token, and detective journal.
     */
    game->session = (GameSession *)malloc(sizeof(GameSession));
    if (game->session)
    {
        game->session->status     = SESSION_STATUS_FRESH;
        game->session->difficulty = DIFFICULTY_DETECTIVE;

        /* Allocate dynamic session identifier string */
        game->session->session_uuid = (char *)malloc(64);
        if (game->session->session_uuid)
        {
            unsigned int seed = (unsigned int)time(NULL);
            snprintf(game->session->session_uuid, 64, "DET-KUET-%04X-%04X",
                     (seed & 0xFFFF), ((seed >> 16) & 0xFFFF));
        }

        /* Allocate dynamic array of CaseRecord items */
        game->session->total_cases = DEFAULT_CASE_COUNT;
        game->session->cases = (CaseRecord *)calloc((size_t)game->session->total_cases, sizeof(CaseRecord));
        if (game->session->cases)
        {
            for (int i = 0; i < game->session->total_cases; i++)
            {
                game->session->cases[i].case_id          = i + 1;
                game->session->cases[i].unlocked         = (i == 0); /* Case 1 unlocked by default */
                game->session->cases[i].solved           = false;
                game->session->cases[i].clues_discovered = 0;
                game->session->cases[i].best_score       = 0;
            }
        }

        /* Allocate dynamic investigation journal notes */
        game->session->notes_capacity      = INITIAL_NOTES_CAP;
        game->session->notes_size          = 0;
        game->session->investigation_notes = (char *)calloc(game->session->notes_capacity, sizeof(char));
        if (game->session->investigation_notes)
        {
            const char *init_note = "[SYSTEM] Case file opened. Cipher records initialized.";
            strncpy(game->session->investigation_notes, init_note, game->session->notes_capacity - 1);
            game->session->notes_size = strlen(game->session->investigation_notes);
        }
    }
}


/* ====================  Game_ChangeScreen  ====================
 * Request a transition to a different screen and update session lifecycle.
 */
void Game_ChangeScreen(GameState *game, ScreenID nextScreen)
{
    if (!game) return;

    game->previousScreen = game->currentScreen;
    game->currentScreen  = nextScreen;

    /* Maintain session lifecycle state */
    if (game->session)
    {
        if (nextScreen == SCREEN_EXIT)
        {
            game->session->status = SESSION_STATUS_RESOLVED;
        }
        else if (nextScreen == SCREEN_LEVEL_MAIN)
        {
            game->session->status = SESSION_STATUS_ACTIVE;
        }
        else if (nextScreen == SCREEN_MAIN_MENU || nextScreen == SCREEN_OPTIONS)
        {
            if (game->session->status == SESSION_STATUS_ACTIVE)
            {
                game->session->status = SESSION_STATUS_PAUSED;
            }
        }
    }

    /* SCREEN_EXIT is the shutdown sentinel */
    if (nextScreen == SCREEN_EXIT)
    {
        game->running = false;
    }
}


/* ====================  Game_Shutdown  ====================
 * Release all backend-allocated dynamic heap resources.
 * Prevents memory leaks upon application exit.
 */
void Game_Shutdown(GameState *game)
{
    if (!game || !game->session) return;

    if (game->session->session_uuid)
    {
        free(game->session->session_uuid);
        game->session->session_uuid = NULL;
    }

    if (game->session->cases)
    {
        free(game->session->cases);
        game->session->cases = NULL;
    }

    if (game->session->investigation_notes)
    {
        free(game->session->investigation_notes);
        game->session->investigation_notes = NULL;
    }

    free(game->session);
    game->session = NULL;
}


/* ====================  Session & Case Helpers  ==================== */

bool Game_IsCaseUnlocked(const GameState *game, int case_id)
{
    if (!game || !game->session || !game->session->cases)
    {
        return (case_id == 1); /* Safe fallback: level 1 always accessible */
    }

    if (case_id >= 1 && case_id <= game->session->total_cases)
    {
        return game->session->cases[case_id - 1].unlocked;
    }

    return false;
}

void Game_UnlockCase(GameState *game, int case_id)
{
    if (!game || !game->session || !game->session->cases) return;

    if (case_id >= 1 && case_id <= game->session->total_cases)
    {
        game->session->cases[case_id - 1].unlocked = true;
    }
}

void Game_MarkCaseSolved(GameState *game, int case_id, int score)
{
    if (!game || !game->session || !game->session->cases) return;

    if (case_id >= 1 && case_id <= game->session->total_cases)
    {
        game->session->cases[case_id - 1].solved = true;
        if (score > game->session->cases[case_id - 1].best_score)
        {
            game->session->cases[case_id - 1].best_score = score;
        }

        /* Auto-unlock subsequent case */
        if (case_id < game->session->total_cases)
        {
            game->session->cases[case_id].unlocked = true;
        }
    }
}

void Game_AppendNote(GameState *game, const char *note)
{
    if (!game || !game->session || !note || !game->session->investigation_notes) return;

    size_t add_len = strlen(note);
    if (game->session->notes_size + add_len + 2 >= game->session->notes_capacity)
    {
        size_t new_cap = (game->session->notes_capacity + add_len + 128) * 2;
        char *expanded = (char *)realloc(game->session->investigation_notes, new_cap);
        if (expanded)
        {
            game->session->investigation_notes = expanded;
            game->session->notes_capacity      = new_cap;
        }
        else
        {
            return; /* Memory reallocation failed, discard append to avoid corruption */
        }
    }

    strcat(game->session->investigation_notes, "\n");
    strcat(game->session->investigation_notes, note);
    game->session->notes_size = strlen(game->session->investigation_notes);
}

const char *Game_GetSessionUUID(const GameState *game)
{
    if (!game || !game->session || !game->session->session_uuid)
    {
        return "DET-GUEST-0000";
    }
    return game->session->session_uuid;
}

SessionStatus Game_GetSessionStatus(const GameState *game)
{
    if (!game || !game->session) return SESSION_STATUS_FRESH;
    return game->session->status;
}

void Game_SetSessionStatus(GameState *game, SessionStatus status)
{
    if (!game || !game->session) return;
    game->session->status = status;
}