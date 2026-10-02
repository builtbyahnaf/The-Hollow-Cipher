#ifndef GAME_H
#define GAME_H

#include <stdbool.h>

typedef enum
{
    SCREEN_BOOT,
    SCREEN_MAIN_MENU,
    SCREEN_EXIT
} ScreenID;

typedef struct
{
    bool running;
    ScreenID currentScreen;

    int level;
    int score;

} GameState;


/* Game lifecycle */

void Game_Init(GameState *game);
void Game_Run(GameState *game);
void Game_Shutdown(GameState *game);


/* Screen control */

void Game_ChangeScreen(GameState *game, ScreenID nextScreen);

#endif