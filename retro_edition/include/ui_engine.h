/*
 * File: ui_engine.h
 * Project: The Hollow Cipher
 *
 * Description:
 *   Retro-themed GUI drawing layer built on Win32 GDI.
 *   Provides double-buffered rendering, a monospace character grid,
 *   color palette constants, panel/text/menu drawing primitives,
 *   and keyboard input translation.
 *
 *   This is a FRONTEND header.  It includes <windows.h> and other
 *   platform-specific headers.  Screen .c files include this to draw.
 *   The backend (game.h / game.c) must NEVER include this header.
 *
 *   How the paint cycle works (see ui_engine.c for full details):
 *     WM_PAINT  →  ui_begin_paint()    — creates the back-buffer
 *                  ui_fill_background() — dark bg + CRT scanlines
 *                  Xxx_Draw(game)       — screen renders into buffer
 *                  ui_end_paint()       — BitBlt to screen, cleanup
 *
 *   The character grid:
 *     The engine divides the window into a grid of fixed-width cells
 *     (measured from the monospace font metrics).  All row/col params
 *     in drawing functions are 0-indexed grid cells, just like a
 *     terminal — making screen layouts easy to reason about.
 */

#ifndef UI_ENGINE_H
#define UI_ENGINE_H

#include <wchar.h>
#include <windows.h>
#include <stdbool.h>

/* ==================== COLOR PALETTE ====================
 * Retro dark-terminal color scheme.  All values are COLORREF (Win32 RGB).
 * These mirror the original TUI ANSI color theme for visual consistency.
 *
 * Usage: pass any of these constants wherever a COLORREF is expected.
 */
#define CLR_BG            RGB(18, 18, 24)       /* Window background (very dark blue-grey)  */
#define CLR_PANEL_BG      RGB(28, 28, 38)       /* Panel interior fill (slightly lighter)   */
#define CLR_BORDER        RGB(0, 180, 216)      /* Panel borders and separators (cyan)      */
#define CLR_TITLE         RGB(0, 255, 180)      /* Primary headings (bright teal-green)     */
#define CLR_SUBTITLE      RGB(120, 170, 255)    /* Secondary headings (soft blue)           */
#define CLR_TEXT          RGB(200, 210, 220)     /* Body text / default color (light grey)   */
#define CLR_MUTED         RGB(90, 100, 115)     /* Dimmed / hint text (dark grey)           */
#define CLR_ACCENT        RGB(255, 180, 0)      /* Important highlights (warm amber)        */
#define CLR_GREEN         RGB(80, 255, 120)     /* Success / positive feedback              */
#define CLR_RED           RGB(255, 80, 120)     /* Error / danger                           */
#define CLR_YELLOW        RGB(255, 255, 100)    /* Warnings / prompts                      */
#define CLR_CYAN          RGB(0, 220, 255)      /* Informational accents                   */
#define CLR_GOLD          RGB(255, 215, 0)      /* Title / prestige text                   */
#define CLR_HIGHLIGHT_BG  RGB(0, 140, 180)      /* Selected menu-item background            */
#define CLR_HIGHLIGHT_FG  RGB(255, 255, 255)    /* Selected menu-item text (bright white)   */


/* ==================== KEY CODES ====================
 * Abstracted keyboard input codes.  Screens receive these instead of
 * raw Win32 virtual key codes, keeping screen logic portable.
 * When building the Raylib edition, you just rewrite ui_translate_key()
 * to map Raylib keys → the same KeyCode enum.
 */
typedef enum
{
    KEY_NONE = 0,
    KEY_UP, KEY_DOWN, KEY_LEFT, KEY_RIGHT,
    KEY_ENTER, KEY_ESCAPE, KEY_TAB,
    KEY_1, KEY_2, KEY_3, KEY_4, KEY_5,
    KEY_Y, KEY_N
} KeyCode;


/* ==================== INITIALIZATION / SHUTDOWN ====================
 * ui_init     — Call once AFTER CreateWindowEx.  Loads the retro font,
 *               measures character metrics, calculates the grid, and
 *               optionally enables the dark title-bar on Windows 10+.
 * ui_shutdown — Call once BEFORE the process exits.  Frees the font.
 */
void ui_init(HWND hwnd);
void ui_shutdown(void);


/* ==================== PAINT CYCLE ====================
 * Every WM_PAINT handler should bracket its drawing calls between
 * ui_begin_paint() and ui_end_paint().
 *
 * Internally these manage:
 *   • BeginPaint / EndPaint
 *   • A memory-DC back-buffer for flicker-free rendering
 *   • Automatic BitBlt to the screen in ui_end_paint()
 *
 * All ui_draw_* calls between begin/end draw into the back-buffer.
 */
void ui_begin_paint(HWND hwnd);
void ui_end_paint(void);


/* ==================== BACKGROUND ====================
 * ui_fill_background — Fills the entire back-buffer with CLR_BG and
 *                      adds a subtle CRT scanline effect (thin darker
 *                      lines every 4th pixel row).
 *                      Call this FIRST after ui_begin_paint().
 */
void ui_fill_background(void);


/* ==================== GRID QUERIES ====================
 * The UI uses a fixed-width character grid (like a terminal).
 * All row/col parameters in drawing functions are 0-indexed grid cells.
 *
 * ui_get_cols — Number of character columns that fit in the window.
 * ui_get_rows — Number of character rows that fit in the window.
 * ui_on_resize — Notify the engine that the window was resized.
 *                Recalculates grid metrics immediately.
 */
int ui_get_cols(void);
int ui_get_rows(void);
void ui_on_resize(HWND hwnd);


/* ==================== TEXT DRAWING ====================
 * Text is rendered in the monospace retro font at grid positions.
 * All row/col values are 0-indexed character-cell coordinates.
 *
 * ui_draw_text           — Draw text at exact (row, col) grid position.
 * ui_draw_text_centered  — Draw text horizontally centered in the window.
 * ui_draw_text_in_region — Center text within a column sub-range,
 *                          useful for centering inside panels.
 */
void ui_draw_text(int row, int col, const char *text, COLORREF color);
void ui_draw_text_centered(int row, const char *text, COLORREF color);
void ui_draw_text_in_region(int row, int region_col, int region_width,
                            const char *text, COLORREF color);


/* ==================== PANEL DRAWING ====================
 * ui_draw_panel — Bordered, filled rectangle with optional title.
 *
 *   row, col       — top-left corner in grid cells
 *   width, height  — size in grid cells
 *   title          — string centered on the top edge, or NULL for none
 *
 * The panel uses CLR_PANEL_BG for fill and CLR_BORDER for the border.
 * The title is drawn in CLR_ACCENT with a background patch that
 * "breaks" the top border line (classic framed-label look).
 */
void ui_draw_panel(int row, int col, int width, int height,
                   const char *title);


/* ==================== LINE DRAWING ====================
 * ui_draw_hline — Horizontal separator line, 1px thick.
 *                 Drawn at the vertical center of the specified row,
 *                 spanning 'width' cells starting from 'col'.
 */
void ui_draw_hline(int row, int col, int width, COLORREF color);


/* ==================== MENU ITEM ====================
 * ui_draw_menu_item — Selectable menu entry with highlight support.
 *
 * When selected == true:
 *   → CLR_HIGHLIGHT_BG background bar + " > label" in CLR_HIGHLIGHT_FG
 * When selected == false:
 *   → No background + "   label" in CLR_TEXT
 *
 * 'width' determines the highlight-bar width in grid cells.
 */
void ui_draw_menu_item(int row, int col, int width, const char *text,
                       bool selected);


/* ==================== RECTANGLE FILL ====================
 * ui_fill_rect — Solid-color fill for a rectangular grid area.
 *                Useful for overlay backgrounds (e.g., behind dialogs)
 *                or clearing specific regions.
 */
void ui_fill_rect(int row, int col, int width, int height, COLORREF color);


/* ==================== INPUT TRANSLATION ====================
 * ui_translate_key — Convert a Win32 virtual-key code (from WM_KEYDOWN's
 *                    wParam) into a portable KeyCode.  Returns KEY_NONE
 *                    for unrecognized keys.
 */
KeyCode ui_translate_key(WPARAM wParam);


/* ==================== SCREEN TRANSITIONS ====================
 * Retro CRT raster / shutter transition system.
 * Smoothly wipes between screens with authentic arcade phosphor scanline shutter animation.
 */
#include <stdint.h>
#include "game.h"

void ui_start_transition(ScreenID target_screen);
bool ui_is_transitioning(void);
void ui_update_transition(GameState *game);
void ui_draw_transition(void);

/* ==================== ANIMATION HELPERS ====================
 * Time and color utilities for smooth, frame-independent arcade animations.
 */
uint64_t ui_get_ticks(void);
COLORREF ui_color_lerp(COLORREF c1, COLORREF c2, float t);

/* ==================== UTILITY ====================
 * ui_sleep_ms — Blocking sleep.  Wraps Win32 Sleep().
 *               (Retained from the TUI engine for any future use.)
 */
void ui_sleep_ms(int ms);

#endif /* UI_ENGINE_H */