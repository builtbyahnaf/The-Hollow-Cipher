/*
 * File: screens.h
 * Project: The Hollow Cipher
 *
 * Description:
 *   Declares the Draw and HandleKey functions for every game screen.
 *   Each screen is implemented in its own .c file under src/screens/.
 *
 *   Screen contract (every screen must provide these two functions):
 *
 *     Xxx_Draw(game)          — Render the screen into the back-buffer.
 *                               Called from WM_PAINT after ui_begin_paint().
 *
 *     Xxx_HandleKey(game, key) — Process one keyboard event.
 *                               Called from WM_KEYDOWN after key translation.
 *                               May call Game_ChangeScreen() to navigate.
 *
 *   This is a FRONTEND header.  It includes both game.h (for GameState)
 *   and ui_engine.h (for KeyCode).  The backend must never include this.
 */

#ifndef SCREENS_H
#define SCREENS_H

#include "game.h"        /* GameState, ScreenID */
#include "ui_engine.h"   /* KeyCode            */

/* ---------- Boot / Splash Screen (boot.c) ---------- */
void Boot_Draw(GameState *game);
void Boot_HandleKey(GameState *game, KeyCode key);

/* ---------- Home / Main Menu (home.c) ---------- */
void Home_Draw(GameState *game);
void Home_HandleKey(GameState *game, KeyCode key);

/* ---------- Options / Settings (options.c) ---------- */
void Options_Draw(GameState *game);
void Options_HandleKey(GameState *game, KeyCode key);

/* ---------- About / Credits (about.c) ---------- */
void About_Draw(GameState *game);
void About_HandleKey(GameState *game, KeyCode key);

/* ---------- Stub Screen (stub_screen.c) ----------
 * Generic placeholder for screens not yet implemented.
 * Shows the screen name + "NOT YET IMPLEMENTED" message.
 */
void StubScreen_Draw(GameState *game);
void StubScreen_HandleKey(GameState *game, KeyCode key);

/* ---------- Level Select Screen (level_select.c) ---------- */
void LevelSelect_Draw(GameState *game);
void LevelSelect_HandleKey(GameState *game, KeyCode key);

/* ---------- Level Main Screen (level_main.c) ---------- */
void LevelMain_Draw(GameState *game);
void LevelMain_HandleKey(GameState *game, KeyCode key);
void LevelMain_HandleChar(GameState *game, char c);

/* ---------- Evidence Archive (evidence.c) ---------- */
void Evidence_Draw(GameState *game);
void Evidence_HandleKey(GameState *game, KeyCode key);

/* ---------- Past History (history.c) ---------- */
void History_Draw(GameState *game);
void History_HandleKey(GameState *game, KeyCode key);

/* ---------- Interrogation (interrogation.c) ---------- */
void Interrogation_Draw(GameState *game);
void Interrogation_HandleKey(GameState *game, KeyCode key);

#endif /* SCREENS_H */