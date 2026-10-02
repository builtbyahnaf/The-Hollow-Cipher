
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
void Boot_Draw(GameState *game)
{
    (void)game;

    printf("\033[2J\033[H");

    printf("============================================\n");
    printf("              THE HOLLOW CIPHER             \n");
    printf("============================================\n\n");

    printf("              [ SYSTEM BOOT ]               \n\n");

    printf("        A confidential investigation         \n");
    printf("        awaits your attention...\n\n");

    printf("        Press ENTER to continue...\n");
}


void Boot_Update(GameState *game)
{
    getchar();

    Game_ChangeScreen(game, SCREEN_MAIN_MENU);
}



/* ==================== PRIVATE FUNCTIONS ==================== */

static void helperFunction(void)
{
    // TODO: implementation
}

