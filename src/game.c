
/*
 * File: filename.c
 * Project: The Hollow Cipher
 * Description: Brief description of this file.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <math.h>
#include <time.h>

#include "game.h"
#include "screens.h"
//#include "filename.h"       // This file's header
//#include "game.h"           // GameState / game logic
//#include "ui_engine.h"      // UI utilities
// #include "raylib.h"      // Include directly if needed


/* ==================== CONSTANTS ==================== */

#define MAX_TEXT 256


/* ==================== PRIVATE DATA ==================== */

// Variables / structs used only inside this .c file


/* ==================== PRIVATE FUNCTIONS ==================== */

// Functions used only inside this .c file
static void helperFunction(void);


/* ==================== PUBLIC FUNCTIONS ==================== */


void Game_Init(GameState *game)
{
    game->running = true;

    game->currentScreen = SCREEN_BOOT;

    game->level = 1;
    game->score = 0;
}


void Game_Run(GameState *game)
{
    while (game->running)
    {
        switch (game->currentScreen)
        {
            case SCREEN_BOOT:
                Boot_Draw(game);
                Boot_Update(game);
                break;

            case SCREEN_MAIN_MENU:
                // MainMenu_Draw(game);
                // MainMenu_Update(game);
                break;

            case SCREEN_EXIT:
                game->running = false;
                break;
        }
    }
}


void Game_ChangeScreen(GameState *game, ScreenID nextScreen)
{
    game->currentScreen = nextScreen;
}


void Game_Shutdown(GameState *game)
{
    (void)game;

    printf("\nShutting down The Hollow Cipher...\n");
}


/* ==================== PRIVATE FUNCTIONS ==================== */

static void helperFunction(void)
{
    // TODO: implementation
}

