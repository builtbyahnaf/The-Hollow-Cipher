/*
 * File: home.c
 * Project: The Hollow Cipher
 *
 * Description:
 *   Main Menu / Home Screen.
 *   Provides navigation across:
 *     1. Play (Level / Case select)
 *     2. Evidence Archives
 *     3. Options / Configuration
 *     4. About & Credits
 *     5. Exit Terminal
 *   Features:
 *     - Heap-managed dynamic menu structure and typed enums/structs
 *     - Arcade style option blink on ENTER selection
 *     - Retro CRT screen transitions
 *     - Confirmation modal dialog for quitting
 *
 * Architecture note:
 *   Frontend screen module implementing Home_Draw and Home_HandleKey.
 */

#include "screens.h"
#include "ui_engine.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* ==================== TYPEDEFS & DATA STRUCTURES ==================== */

typedef enum
{
    HOME_OPT_PLAY = 0,
    HOME_OPT_EVIDENCE,
    HOME_OPT_OPTIONS,
    HOME_OPT_ABOUT,
    HOME_OPT_EXIT,
    HOME_MENU_COUNT
} HomeMenuOption;

typedef enum
{
    HOME_MODAL_NONE = 0,
    HOME_MODAL_CONFIRM_EXIT
} HomeModalDialog;

typedef struct
{
    int            index;
    char          *label;            /* Dynamically allocated item string */
    ScreenID       target_screen;
    KeyCode        shortcut_key;
} HomeMenuItem;

typedef struct
{
    HomeMenuItem   *items;            /* Dynamically allocated array of menu items */
    int             item_count;
    int             selected_index;
    int             blink_option;
    uint64_t        blink_start;
    uint32_t        blink_duration_ms;
    HomeModalDialog active_modal;
    bool            initialized;
} HomeScreenState;

/* Static pointer to heap-allocated screen state */
static HomeScreenState *g_home = NULL;

/* ==================== LIFECYCLE & MEMORY ALLOCATION ==================== */

static void Home_Destroy(void)
{
    if (!g_home) return;

    if (g_home->items)
    {
        for (int i = 0; i < g_home->item_count; i++)
        {
            if (g_home->items[i].label)
            {
                free(g_home->items[i].label);
                g_home->items[i].label = NULL;
            }
        }
        free(g_home->items);
        g_home->items = NULL;
    }

    free(g_home);
    g_home = NULL;
}

static bool Home_EnsureInitialized(void)
{
    if (g_home && g_home->initialized) return true;

    /* Allocate state container on heap */
    g_home = (HomeScreenState *)calloc(1, sizeof(HomeScreenState));
    if (!g_home) return false;

    g_home->item_count        = HOME_MENU_COUNT;
    g_home->selected_index    = 0;
    g_home->blink_option      = -1;
    g_home->blink_start       = 0;
    g_home->blink_duration_ms = 240; /* 240ms arcade selection flash */
    g_home->active_modal      = HOME_MODAL_NONE;

    /* Allocate dynamic items array */
    g_home->items = (HomeMenuItem *)calloc((size_t)g_home->item_count, sizeof(HomeMenuItem));
    if (!g_home->items)
    {
        free(g_home);
        g_home = NULL;
        return false;
    }

    static const char *labels[HOME_MENU_COUNT] = {
        "1. PLAY",
        "2. EVIDENCE ARCHIVE",
        "3. OPTIONS",
        "4. ABOUT & CREDITS",
        "5. EXIT"
    };

    static const ScreenID screens[HOME_MENU_COUNT] = {
        SCREEN_LEVEL_SELECT,
        SCREEN_EVIDENCE,
        SCREEN_OPTIONS,
        SCREEN_ABOUT,
        SCREEN_EXIT
    };

    static const KeyCode shortcuts[HOME_MENU_COUNT] = {
        KEY_1, KEY_2, KEY_3, KEY_4, KEY_5
    };

    for (int i = 0; i < g_home->item_count; i++)
    {
        g_home->items[i].index         = i;
        g_home->items[i].target_screen = screens[i];
        g_home->items[i].shortcut_key  = shortcuts[i];

        size_t len = strlen(labels[i]) + 1;
        g_home->items[i].label = (char *)malloc(len);
        if (g_home->items[i].label)
        {
            memcpy(g_home->items[i].label, labels[i], len);
        }
    }

    g_home->initialized = true;
    return true;
}

/* ==================== SCREEN DRAWING ==================== */

void Home_Draw(GameState *game)
{
    (void)game;

    if (!Home_EnsureInitialized()) return;

    uint64_t now  = ui_get_ticks();
    int      cols = ui_get_cols();
    int      rows = ui_get_rows();

    /* Process finished arcade option blink */
    if (g_home->blink_option >= 0)
    {
        uint32_t elapsed = (uint32_t)(now - g_home->blink_start);
        if (elapsed >= g_home->blink_duration_ms)
        {
            int opt = g_home->blink_option;
            g_home->blink_option = -1;

            if (g_home->items[opt].target_screen == SCREEN_EXIT)
            {
                g_home->active_modal = HOME_MODAL_CONFIRM_EXIT;
            }
            else
            {
                ui_start_transition(g_home->items[opt].target_screen);
            }
        }
    }

    /* Header rule */
    ui_draw_hline(2, 2, cols - 4, CLR_BORDER);

    /* Retro Title Banner */
    int banner_y = 4;
    ui_draw_text_centered(banner_y + 0, "================================================", CLR_BORDER);
    ui_draw_text_centered(banner_y + 1, "  T H E   H O L L O W   C I P H E R  ", CLR_GOLD);
    ui_draw_text_centered(banner_y + 2, "================================================", CLR_BORDER);
    ui_draw_text_centered(banner_y + 4, "[ RETRO EDITION ]", CLR_CYAN);

    /* Main Menu Panel with safe coordinate clamping */
    int panel_w = 46;
    if (panel_w > cols - 4) panel_w = cols - 4;
    if (panel_w < 30) panel_w = 30;

    int panel_h = g_home->item_count + 4;
    int panel_x = (cols - panel_w) / 2;
    if (panel_x < 2) panel_x = 2;
    int panel_y = banner_y + 9;

    ui_draw_panel(panel_y, panel_x, panel_w, panel_h, "MAIN MENU");

    /* Draw each menu item with arcade blink support */
    for (int i = 0; i < g_home->item_count; i++)
    {
        int item_y = panel_y + 2 + i;
        bool is_selected = (i == g_home->selected_index);

        if (i == g_home->blink_option)
        {
            /* Rapid arcade flashing on ENTER press (45ms alternating flash) */
            uint32_t blink_elapsed = (uint32_t)(now - g_home->blink_start);
            bool     flash         = ((blink_elapsed / 45) % 2 == 0);
            COLORREF bg            = flash ? RGB(255, 255, 255) : CLR_HIGHLIGHT_BG;
            COLORREF fg            = flash ? RGB(0, 0, 0) : CLR_HIGHLIGHT_FG;

            ui_fill_rect(item_y, panel_x + 3, panel_w - 6, 1, bg);
            char buf[128];
            snprintf(buf, sizeof(buf), ">> %s <<", g_home->items[i].label);
            ui_draw_text_in_region(item_y, panel_x + 3, panel_w - 6, buf, fg);
        }
        else
        {
            ui_draw_menu_item(item_y, panel_x + 3, panel_w - 6, g_home->items[i].label, is_selected);
        }
    }

    /* Footer instruction bar with pulsing hint */
    ui_draw_hline(rows - 3, 2, cols - 4, CLR_BORDER);
    bool hint_pulse = ((now / 500) % 2 == 0);
    COLORREF hint_color = hint_pulse ? CLR_MUTED : CLR_TEXT;
    ui_draw_text(rows - 2, 4, "[UP/DOWN/1-5] Navigate    [ENTER] Select    [ESC] Exit", hint_color);

    /* Render modal overlay if Exit Confirmation is open */
    if (g_home->active_modal == HOME_MODAL_CONFIRM_EXIT)
    {
        int modal_w = 46;
        if (modal_w > cols - 4) modal_w = cols - 4;
        int modal_h = 7;
        int modal_x = (cols - modal_w) / 2;
        int modal_y = (rows - modal_h) / 2;
        if (modal_x < 2) modal_x = 2;
        if (modal_y < 2) modal_y = 2;

        /* Dark shadow backdrop */
        ui_fill_rect(modal_y - 1, modal_x - 2, modal_w + 4, modal_h + 2, CLR_BG);

        /* Dialog panel */
        ui_draw_panel(modal_y, modal_x, modal_w, modal_h, "CONFIRM");
        ui_draw_text_in_region(modal_y + 2, modal_x, modal_w, "Are you sure you want to exit?", CLR_TEXT);
        ui_draw_text_in_region(modal_y + 4, modal_x, modal_w, "[Y] YES, EXIT       [N] CANCEL", CLR_YELLOW);
    }
}

/* ==================== INPUT HANDLING ==================== */

void Home_HandleKey(GameState *game, KeyCode key)
{
    if (!Home_EnsureInitialized()) return;

    /* Suppress input while arcade blink is in flight */
    if (g_home->blink_option >= 0) return;

    /* If modal is open, intercept confirmation keys */
    if (g_home->active_modal == HOME_MODAL_CONFIRM_EXIT)
    {
        if (key == KEY_Y || key == KEY_ENTER)
        {
            Home_Destroy();
            Game_ChangeScreen(game, SCREEN_EXIT);
        }
        else if (key == KEY_N || key == KEY_ESCAPE)
        {
            g_home->active_modal = HOME_MODAL_NONE;
        }
        return;
    }

    /* Standard Menu Navigation */
    switch (key)
    {
        case KEY_UP:
            g_home->selected_index = (g_home->selected_index - 1 + g_home->item_count) % g_home->item_count;
            break;

        case KEY_DOWN:
            g_home->selected_index = (g_home->selected_index + 1) % g_home->item_count;
            break;

        case KEY_1:
        case KEY_2:
        case KEY_3:
        case KEY_4:
        case KEY_5:
            for (int i = 0; i < g_home->item_count; i++)
            {
                if (g_home->items[i].shortcut_key == key)
                {
                    g_home->selected_index = i;
                    break;
                }
            }
            break;

        case KEY_ENTER:
            if (g_home->items[g_home->selected_index].target_screen == SCREEN_EXIT)
            {
                g_home->active_modal = HOME_MODAL_CONFIRM_EXIT;
            }
            else
            {
                /* Start arcade option blink animation */
                g_home->blink_option = g_home->selected_index;
                g_home->blink_start  = ui_get_ticks();
            }
            break;

        case KEY_ESCAPE:
            g_home->active_modal = HOME_MODAL_CONFIRM_EXIT;
            break;

        default:
            break;
    }
}