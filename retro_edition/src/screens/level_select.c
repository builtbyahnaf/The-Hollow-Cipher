/*
 * File: level_select.c
 * Project: The Hollow Cipher
 *
 * Description:
 *   Level / Case Selection Screen.
 *   Displays available investigation levels with lock/unlock status.
 *   Uses heap-allocated case metadata, typed enums/structs, and connects
 *   to backend GameSession progression state.
 *
 * Architecture note:
 *   Frontend screen module implementing LevelSelect_Draw and LevelSelect_HandleKey.
 *   Interacts with GameState via Game_ChangeScreen() and Game_IsCaseUnlocked().
 */

#include "screens.h"
#include "ui_engine.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_CASES 3

/* ==================== TYPEDEFS & DATA STRUCTURES ==================== */

typedef enum
{
    CASE_STATE_LOCKED = 0,
    CASE_STATE_UNLOCKED,
    CASE_STATE_SOLVED
} CaseAccessState;

typedef struct
{
    int             case_id;          /* 1-based case number                      */
    char           *title;            /* Dynamically allocated case title string  */
    char           *subtitle;         /* Dynamically allocated narrative blurb    */
    char           *badge;            /* Dynamically allocated status tag         */
    CaseAccessState access;
} CaseInfo;

typedef struct
{
    CaseInfo       *cases;            /* Dynamically allocated array of CaseInfo  */
    int             total_cases;
    int             selected_index;
    bool            initialized;
} LevelSelectMenu;

/* Static pointer to heap-allocated screen data */
static LevelSelectMenu *g_lvl_menu = NULL;

/* ==================== LIFECYCLE & MEMORY ALLOCATION ==================== */

static void LevelSelect_Destroy(void)
{
    if (!g_lvl_menu) return;

    if (g_lvl_menu->cases)
    {
        for (int i = 0; i < g_lvl_menu->total_cases; i++)
        {
            if (g_lvl_menu->cases[i].title)
            {
                free(g_lvl_menu->cases[i].title);
                g_lvl_menu->cases[i].title = NULL;
            }
            if (g_lvl_menu->cases[i].subtitle)
            {
                free(g_lvl_menu->cases[i].subtitle);
                g_lvl_menu->cases[i].subtitle = NULL;
            }
            if (g_lvl_menu->cases[i].badge)
            {
                free(g_lvl_menu->cases[i].badge);
                g_lvl_menu->cases[i].badge = NULL;
            }
        }
        free(g_lvl_menu->cases);
        g_lvl_menu->cases = NULL;
    }

    free(g_lvl_menu);
    g_lvl_menu = NULL;
}

static bool LevelSelect_EnsureInitialized(void)
{
    if (g_lvl_menu && g_lvl_menu->initialized) return true;

    g_lvl_menu = (LevelSelectMenu *)calloc(1, sizeof(LevelSelectMenu));
    if (!g_lvl_menu) return false;

    g_lvl_menu->total_cases    = MAX_CASES;
    g_lvl_menu->selected_index = 0;

    g_lvl_menu->cases = (CaseInfo *)calloc((size_t)g_lvl_menu->total_cases, sizeof(CaseInfo));
    if (!g_lvl_menu->cases)
    {
        free(g_lvl_menu);
        g_lvl_menu = NULL;
        return false;
    }

    static const char *raw_titles[MAX_CASES] = {
        "CASE 01: THE SILENT PATIENT",
        "CASE 02: THE MISSING NOTEBOOK",
        "CASE 03: THE PSYCHOPATH"
    };

    static const char *raw_subtitles[MAX_CASES] = {
        "A journalist's murder, a missing notebook and a shocked patient.",
        "The investigation deepens to uncover missing notebook.",
        "A private investigation from the confidential documents."
    };

    for (int i = 0; i < g_lvl_menu->total_cases; i++)
    {
        g_lvl_menu->cases[i].case_id = i + 1;
        g_lvl_menu->cases[i].access  = (i == 0) ? CASE_STATE_UNLOCKED : CASE_STATE_LOCKED;

        size_t title_len = strlen(raw_titles[i]) + 1;
        g_lvl_menu->cases[i].title = (char *)malloc(title_len);
        if (g_lvl_menu->cases[i].title)
        {
            memcpy(g_lvl_menu->cases[i].title, raw_titles[i], title_len);
        }

        size_t sub_len = strlen(raw_subtitles[i]) + 1;
        g_lvl_menu->cases[i].subtitle = (char *)malloc(sub_len);
        if (g_lvl_menu->cases[i].subtitle)
        {
            memcpy(g_lvl_menu->cases[i].subtitle, raw_subtitles[i], sub_len);
        }

        const char *initial_badge = (i == 0) ? "[ UNLOCKED ]" : "[ LOCKED  ]";
        size_t badge_len = strlen(initial_badge) + 1;
        g_lvl_menu->cases[i].badge = (char *)malloc(badge_len);
        if (g_lvl_menu->cases[i].badge)
        {
            memcpy(g_lvl_menu->cases[i].badge, initial_badge, badge_len);
        }
    }

    g_lvl_menu->initialized = true;
    return true;
}

/* Helper to synchronize unlock status with backend GameState */
static bool is_case_unlocked(const GameState *game, int case_index)
{
    if (game)
    {
        return Game_IsCaseUnlocked(game, case_index + 1);
    }
    return (case_index == 0);
}

/* ==================== SCREEN DRAWING ==================== */

void LevelSelect_Draw(GameState *game)
{
    if (!LevelSelect_EnsureInitialized()) return;

    int cols = ui_get_cols();
    int rows = ui_get_rows();

    /* Header rules and banner */
    ui_draw_hline(2, 2, cols - 4, CLR_BORDER);
    ui_draw_text_centered(4, "[ CASE SELECTION ]", CLR_GOLD);
    ui_draw_text_centered(5, "Use UP/DOWN to navigate. ENTER to open case. ESC to return.", CLR_MUTED);

    /* Level list panel with responsive layout protection */
    int panel_w = 80;
    if (panel_w > cols - 4) panel_w = cols - 4;
    if (panel_w < 40) panel_w = 40;

    int panel_h = g_lvl_menu->total_cases * 4 + 4;
    int panel_x = (cols - panel_w) / 2;
    if (panel_x < 2) panel_x = 2;
    int panel_y = 7;

    ui_draw_panel(panel_y, panel_x, panel_w, panel_h, "AVAILABLE CASES");

    for (int i = 0; i < g_lvl_menu->total_cases; i++)
    {
        int item_y = panel_y + 2 + i * 4;
        bool unlocked = is_case_unlocked(game, i);
        bool selected = (i == g_lvl_menu->selected_index);

        /* Case title bar highlight */
        COLORREF title_color = unlocked ? (selected ? CLR_HIGHLIGHT_FG : CLR_TEXT) : CLR_MUTED;
        if (selected && unlocked)
        {
            ui_fill_rect(item_y, panel_x + 2, panel_w - 4, 1, CLR_HIGHLIGHT_BG);
        }

        char prefix[8];
        snprintf(prefix, sizeof(prefix), "%s ", selected ? "> " : "  ");
        char title_buf[160];
        snprintf(title_buf, sizeof(title_buf), "%s%s", prefix, g_lvl_menu->cases[i].title);
        ui_draw_text(item_y, panel_x + 3, title_buf, title_color);

        /* Subtitle narrative description */
        COLORREF sub_color = unlocked ? CLR_SUBTITLE : CLR_MUTED;
        ui_draw_text(item_y + 1, panel_x + 5, g_lvl_menu->cases[i].subtitle, sub_color);

        /* Lock status badge */
        const char *lock_text = unlocked ? "[ UNLOCKED ]" : "[ LOCKED  ]";
        COLORREF    lock_color = unlocked ? CLR_GREEN : CLR_RED;
        int lock_col = panel_x + panel_w - (int)strlen(lock_text) - 4;
        if (lock_col > panel_x + 10)
        {
            ui_draw_text(item_y, lock_col, lock_text, lock_color);
        }

        if (i < g_lvl_menu->total_cases - 1)
        {
            ui_draw_hline(item_y + 2, panel_x + 3, panel_w - 6, CLR_BORDER);
        }
    }

    /* Footer instruction bar */
    ui_draw_hline(rows - 3, 2, cols - 4, CLR_BORDER);
    ui_draw_text(rows - 2, 4, "[UP/DOWN/1-3] Select Case    [ENTER] Open    [ESC] Back to Menu", CLR_MUTED);
}

/* ==================== INPUT HANDLING ==================== */

void LevelSelect_HandleKey(GameState *game, KeyCode key)
{
    if (!LevelSelect_EnsureInitialized()) return;

    switch (key)
    {
        case KEY_UP:
            g_lvl_menu->selected_index =
                (g_lvl_menu->selected_index - 1 + g_lvl_menu->total_cases) % g_lvl_menu->total_cases;
            break;

        case KEY_DOWN:
            g_lvl_menu->selected_index =
                (g_lvl_menu->selected_index + 1) % g_lvl_menu->total_cases;
            break;

        case KEY_1:
            g_lvl_menu->selected_index = 0;
            break;

        case KEY_2:
            if (g_lvl_menu->total_cases > 1) g_lvl_menu->selected_index = 1;
            break;

        case KEY_3:
            if (g_lvl_menu->total_cases > 2) g_lvl_menu->selected_index = 2;
            break;

        case KEY_ENTER:
            if (is_case_unlocked(game, g_lvl_menu->selected_index))
            {
                game->level = g_lvl_menu->cases[g_lvl_menu->selected_index].case_id;
                LevelSelect_Destroy();
                Game_ChangeScreen(game, SCREEN_LEVEL_MAIN);
            }
            break;

        case KEY_ESCAPE:
            LevelSelect_Destroy();
            Game_ChangeScreen(game, SCREEN_MAIN_MENU);
            break;

        default:
            break;
    }
}