/*
 * File: ui_engine.c
 * Project: The Hollow Cipher
 *
 * Description:
 *   Retro GUI drawing engine implementation using Win32 API and GDI.
 *   Provides:
 *     - Monospace character grid system with configurable margins and cell metrics
 *     - Hardware-free double buffering (memory DC + compatible bitmap) to eliminate flickering
 *     - CRT scanline visual overlay for retro computer terminal ambiance
 *     - Color palette management using standard Win32 COLORREF
 *     - Text rendering, panel frames, horizontal rules, and menu bars
 *     - Virtual-key translation into platform-agnostic KeyCode
 *
 *   Architecture note:
 *     This is strictly a FRONTEND rendering module. It depends on Windows GDI and ui_engine.h.
 *     Core game logic remains completely decoupled and oblivious to GDI.
 */

#include "ui_engine.h"
#include <wchar.h>
#include <stdio.h>
#include <string.h>

/* ==================== PRIVATE STATE ==================== */

static HWND       g_hwnd           = NULL;
static HFONT      g_retro_font     = NULL;
static int        g_char_width     = 10;
static int        g_char_height    = 20;
static int        g_cols           = 80;
static int        g_rows           = 30;
static int        g_margin_x       = 16;
static int        g_margin_y       = 16;
static RECT       g_client_rect    = {0, 0, 0, 0};

/* Paint cycle handles */
static PAINTSTRUCT g_ps;
static HDC         g_hdc_screen    = NULL;
static HDC         g_hdc_mem       = NULL;
static HBITMAP     g_hbm_mem       = NULL;
static HBITMAP     g_hbm_old       = NULL;
static HFONT       g_hfont_old     = NULL;

/* ==================== INITIALIZATION / SHUTDOWN ==================== */

/*
 * ui_init
 *   Initializes font resources and measures character dimensions.
 *   Creates a crisp monospace retro-terminal font (Consolas / Lucida Console / Courier).
 */
void ui_init(HWND hwnd)
{
    g_hwnd = hwnd;

    /* Create crisp retro monospace font (Consolas is available on modern Windows;
     * fallback to Lucida Console or Terminal) */
    g_retro_font = CreateFontA(
        20,                         /* Height (pixels) */
        10,                         /* Width (pixels) */
        0,                          /* Escapement */
        0,                          /* Orientation */
        FW_BOLD,                    /* Weight: Bold gives punchy retro CRT aesthetic */
        FALSE,                      /* Italic */
        FALSE,                      /* Underline */
        FALSE,                      /* Strikeout */
        DEFAULT_CHARSET,            /* Charset */
        OUT_DEFAULT_PRECIS,
        CLIP_DEFAULT_PRECIS,
        CLEARTYPE_QUALITY,          /* Crisp glyph rendering */
        FIXED_PITCH | FF_MODERN,    /* Monospaced pitch */
        "Consolas"                  /* Preferred font */
    );

    /* Measure actual character cell dimensions from a temporary DC */
    HDC temp_dc = GetDC(hwnd);
    if (temp_dc) {
        HFONT old_font = (HFONT)SelectObject(temp_dc, g_retro_font);
        TEXTMETRICA tm;
        if (GetTextMetricsA(temp_dc, &tm)) {
            g_char_height = tm.tmHeight;
            g_char_width  = tm.tmAveCharWidth;
        }
        SelectObject(temp_dc, old_font);
        ReleaseDC(hwnd, temp_dc);
    }

    /* Update client area and calculate grid bounds */
    GetClientRect(hwnd, &g_client_rect);
    int client_w = g_client_rect.right - g_client_rect.left;
    int client_h = g_client_rect.bottom - g_client_rect.top;

    if (g_char_width > 0 && g_char_height > 0) {
        g_cols = (client_w - (2 * g_margin_x)) / g_char_width;
        g_rows = (client_h - (2 * g_margin_y)) / g_char_height;
        if (g_cols < 40) g_cols = 40;
        if (g_rows < 20) g_rows = 20;
    }
}

/*
 * ui_shutdown
 *   Releases font objects and cleans up GDI memory.
 */
void ui_shutdown(void)
{
    if (g_retro_font) {
        DeleteObject(g_retro_font);
        g_retro_font = NULL;
    }
}

/* ==================== PAINT CYCLE (DOUBLE BUFFERING) ==================== */

/*
 * ui_begin_paint
 *   Called inside WM_PAINT. Sets up an off-screen compatible memory DC and bitmap
 *   so that all subsequent drawing calls are buffered in RAM, avoiding any screen tearing or flicker.
 */
void ui_begin_paint(HWND hwnd)
{
    g_hwnd = hwnd;
    g_hdc_screen = BeginPaint(hwnd, &g_ps);

    GetClientRect(hwnd, &g_client_rect);
    int width  = g_client_rect.right - g_client_rect.left;
    int height = g_client_rect.bottom - g_client_rect.top;

    /* Recompute column and row capacity based on current window size */
    if (g_char_width > 0 && g_char_height > 0) {
        g_cols = (width - (2 * g_margin_x)) / g_char_width;
        g_rows = (height - (2 * g_margin_y)) / g_char_height;
    }

    /* Create compatible DC and back-buffer bitmap */
    g_hdc_mem = CreateCompatibleDC(g_hdc_screen);
    g_hbm_mem = CreateCompatibleBitmap(g_hdc_screen, width, height);
    g_hbm_old = (HBITMAP)SelectObject(g_hdc_mem, g_hbm_mem);

    /* Setup font and text attributes on the memory DC */
    g_hfont_old = (HFONT)SelectObject(g_hdc_mem, g_retro_font);
    SetBkMode(g_hdc_mem, TRANSPARENT);
}

/*
 * ui_end_paint
 *   Blits the off-screen buffer to the screen DC in a single hardware-accelerated pass,
 *   then safely disposes of the back-buffer resources and ends the Win32 paint event.
 */
void ui_end_paint(void)
{
    if (g_hdc_screen && g_hdc_mem) {
        int width  = g_client_rect.right - g_client_rect.left;
        int height = g_client_rect.bottom - g_client_rect.top;

        /* Single BitBlt to display surface */
        BitBlt(g_hdc_screen, 0, 0, width, height, g_hdc_mem, 0, 0, SRCCOPY);

        /* Cleanup memory DC */
        SelectObject(g_hdc_mem, g_hfont_old);
        SelectObject(g_hdc_mem, g_hbm_old);
        DeleteObject(g_hbm_mem);
        DeleteDC(g_hdc_mem);
    }

    EndPaint(g_hwnd, &g_ps);

    g_hdc_screen = NULL;
    g_hdc_mem    = NULL;
    g_hbm_mem    = NULL;
}

/* ==================== BACKGROUND & SCANLINES ==================== */

/*
 * ui_fill_background
 *   Paints the retro dark background and simulates CRT monitor horizontal scanlines
 *   by drawing subtle dim lines at repeating pixel intervals.
 */
void ui_fill_background(void)
{
    if (!g_hdc_mem) return;

    /* Solid dark background fill */
    HBRUSH bg_brush = CreateSolidBrush(CLR_BG);
    FillRect(g_hdc_mem, &g_client_rect, bg_brush);
    DeleteObject(bg_brush);

    /* Subtle CRT scanline effect: draw every 3rd line with dark translucent tint */
    HPEN scanline_pen = CreatePen(PS_SOLID, 1, RGB(10, 10, 14));
    HPEN old_pen = (HPEN)SelectObject(g_hdc_mem, scanline_pen);

    int width = g_client_rect.right;
    int height = g_client_rect.bottom;

    for (int y = 0; y < height; y += 3) {
        MoveToEx(g_hdc_mem, 0, y, NULL);
        LineTo(g_hdc_mem, width, y);
    }

    SelectObject(g_hdc_mem, old_pen);
    DeleteObject(scanline_pen);
}

/* ==================== GRID QUERIES ==================== */

int ui_get_cols(void) { return g_cols; }
int ui_get_rows(void) { return g_rows; }

/*
 * ui_on_resize
 *   Called when the window receives WM_SIZE. Immediately recalculates
 *   the character grid dimensions based on the new client area size.
 *   This ensures the grid is correct before the next WM_PAINT.
 */
void ui_on_resize(HWND hwnd)
{
    if (!hwnd) return;
    g_hwnd = hwnd;

    RECT rect;
    GetClientRect(hwnd, &rect);
    int width  = rect.right - rect.left;
    int height = rect.bottom - rect.top;

    /* Enforce minimum client size to prevent layout breakage */
    if (width < 400) width = 400;
    if (height < 300) height = 300;

    if (g_char_width > 0 && g_char_height > 0) {
        g_cols = (width - (2 * g_margin_x)) / g_char_width;
        g_rows = (height - (2 * g_margin_y)) / g_char_height;
        if (g_cols < 40) g_cols = 40;
        if (g_rows < 20) g_rows = 20;
    }
}

/* ==================== TEXT DRAWING ==================== */

/*
 * ui_draw_text
 *   Renders a text string at character-grid coordinate (row, col) with custom color.
 */
void ui_draw_text(int row, int col, const char *text, COLORREF color)
{
    if (!g_hdc_mem || !text) return;

    //Bug patch: unsupported characters

    // Convert UTF-8 string to wide character array (UTF-16)
    wchar_t wbuf[256];
    int wlen = MultiByteToWideChar(CP_UTF8, 0, text, -1, wbuf, 256);
    if (wlen <= 1) return;

    int x = g_margin_x + col * g_char_width;
    int y = g_margin_y + row * g_char_height;

    SetTextColor(g_hdc_mem, color);
    // Use TextOutW for wide strings and character support
    //TextOutW(g_hdc_mem, x, y, text, (int)strlen(text));
    TextOutW(g_hdc_mem, x, y, wbuf, wlen - 1);
}

/*
 * ui_draw_text_centered
 *   Renders a text string centered along the horizontal width of the grid at specified row.
 */
void ui_draw_text_centered(int row, const char *text, COLORREF color)
{
    if (!g_hdc_mem || !text) return;

    //Unsupported character bug patch

    //int len = (int)strlen(text);
    // wcslen counts UTF-16 code units (visual characters for standard BMP symbols)

    wchar_t wbuf[256];
    int wlen = MultiByteToWideChar(CP_UTF8, 0, text, -1, wbuf, 256) - 1;
    if (wlen < 0) wlen = 0;

    //int len = (int)wcslen(text);
    int col = (g_cols - wlen) / 2;
    if (col < 0) col = 0;

    ui_draw_text(row, col, text, color);
}

/*
 * ui_draw_text_in_region
 *   Renders a text string horizontally centered inside a sub-region (e.g. within a panel).
 */
void ui_draw_text_in_region(int row, int region_col, int region_width,
                            const char *text, COLORREF color)
{
    if (!g_hdc_mem || !text) return;

    int len = (int)strlen(text);
    int col = region_col + (region_width - len) / 2;
    if (col < region_col) col = region_col;

    ui_draw_text(row, col, text, color);
}

/* ==================== PANEL & SHAPE DRAWING ==================== */

/*
 * ui_draw_panel
 *   Draws a retro styled panel with background fill, border, and an optional header label.
 */
void ui_draw_panel(int row, int col, int width, int height, const char *title)
{
    if (!g_hdc_mem) return;

    RECT r;
    r.left   = g_margin_x + col * g_char_width;
    r.top    = g_margin_y + row * g_char_height;
    r.right  = r.left + width * g_char_width;
    r.bottom = r.top + height * g_char_height;

    /* Fill panel interior */
    HBRUSH bg_brush = CreateSolidBrush(CLR_PANEL_BG);
    FillRect(g_hdc_mem, &r, bg_brush);
    DeleteObject(bg_brush);

    /* Draw border lines */
    HPEN border_pen = CreatePen(PS_SOLID, 2, CLR_BORDER);
    HPEN old_pen = (HPEN)SelectObject(g_hdc_mem, border_pen);
    HBRUSH null_brush = (HBRUSH)GetStockObject(NULL_BRUSH);
    HBRUSH old_brush = (HBRUSH)SelectObject(g_hdc_mem, null_brush);

    Rectangle(g_hdc_mem, r.left, r.top, r.right, r.bottom);

    SelectObject(g_hdc_mem, old_pen);
    SelectObject(g_hdc_mem, old_brush);
    DeleteObject(border_pen);

    /* If title provided, create an inset title badge on top border */
    if (title && strlen(title) > 0) {
        char title_buf[128];
        snprintf(title_buf, sizeof(title_buf), " %s ", title);
        int title_len = (int)strlen(title_buf);
        int title_col = col + (width - title_len) / 2;
        if (title_col < col + 1) title_col = col + 1;

        int title_x = g_margin_x + title_col * g_char_width;
        int title_y = r.top - (g_char_height / 3);

        RECT badge_r;
        badge_r.left   = title_x;
        badge_r.top    = r.top - 2;
        badge_r.right  = title_x + title_len * g_char_width;
        badge_r.bottom = r.top + g_char_height / 2;

        HBRUSH title_bg = CreateSolidBrush(CLR_PANEL_BG);
        FillRect(g_hdc_mem, &badge_r, title_bg);
        DeleteObject(title_bg);

        SetTextColor(g_hdc_mem, CLR_ACCENT);
        TextOutA(g_hdc_mem, title_x, title_y, title_buf, title_len);
    }
}

/*
 * ui_draw_hline
 *   Draws a 1px horizontal rule inside the grid cell row.
 */
void ui_draw_hline(int row, int col, int width, COLORREF color)
{
    if (!g_hdc_mem) return;

    int x1 = g_margin_x + col * g_char_width;
    int x2 = x1 + width * g_char_width;
    int y  = g_margin_y + row * g_char_height + (g_char_height / 2);

    HPEN pen = CreatePen(PS_SOLID, 1, color);
    HPEN old_pen = (HPEN)SelectObject(g_hdc_mem, pen);

    MoveToEx(g_hdc_mem, x1, y, NULL);
    LineTo(g_hdc_mem, x2, y);

    SelectObject(g_hdc_mem, old_pen);
    DeleteObject(pen);
}

/*
 * ui_draw_menu_item
 *   Draws an interactive menu item row with highlight bar when selected.
 */
void ui_draw_menu_item(int row, int col, int width, const char *text, bool selected)
{
    if (!g_hdc_mem || !text) return;

    int x = g_margin_x + col * g_char_width;
    int y = g_margin_y + row * g_char_height;

    if (selected) {
        /* Draw selected highlight bar */
        RECT r;
        r.left   = x;
        r.top    = y;
        r.right  = x + width * g_char_width;
        r.bottom = y + g_char_height;

        HBRUSH hl_brush = CreateSolidBrush(CLR_HIGHLIGHT_BG);
        FillRect(g_hdc_mem, &r, hl_brush);
        DeleteObject(hl_brush);

        char buf[128];
        snprintf(buf, sizeof(buf), "> %s", text);
        SetTextColor(g_hdc_mem, CLR_HIGHLIGHT_FG);
        TextOutA(g_hdc_mem, x + 4, y, buf, (int)strlen(buf));
    } else {
        char buf[128];
        snprintf(buf, sizeof(buf), "  %s", text);
        SetTextColor(g_hdc_mem, CLR_TEXT);
        TextOutA(g_hdc_mem, x + 4, y, buf, (int)strlen(buf));
    }
}

/*
 * ui_fill_rect
 *   Paints a filled rectangle in grid coordinates with the specified color.
 */
void ui_fill_rect(int row, int col, int width, int height, COLORREF color)
{
    if (!g_hdc_mem) return;

    RECT r;
    r.left   = g_margin_x + col * g_char_width;
    r.top    = g_margin_y + row * g_char_height;
    r.right  = r.left + width * g_char_width;
    r.bottom = r.top + height * g_char_height;

    HBRUSH brush = CreateSolidBrush(color);
    FillRect(g_hdc_mem, &r, brush);
    DeleteObject(brush);
}

/* ==================== INPUT TRANSLATION ==================== */

/*
 * ui_translate_key
 *   Translates raw Win32 Virtual-Key codes into game KeyCode enums.
 */
KeyCode ui_translate_key(WPARAM wParam)
{
    switch (wParam) {
        case VK_UP:     return KEY_UP;
        case VK_DOWN:   return KEY_DOWN;
        case VK_LEFT:   return KEY_LEFT;
        case VK_RIGHT:  return KEY_RIGHT;
        case VK_RETURN: return KEY_ENTER;
        case VK_ESCAPE: return KEY_ESCAPE;
        case VK_TAB:    return KEY_TAB;
        case '1':       return KEY_1;
        case '2':       return KEY_2;
        case '3':       return KEY_3;
        case '4':       return KEY_4;
        case '5':       return KEY_5;
        case 'Y':       return KEY_Y;
        case 'N':       return KEY_N;
        default:        return KEY_NONE;
    }
}

/* ==================== UTILITY ==================== */

void ui_sleep_ms(int ms)
{
    Sleep(ms);
}

/* ==================== SCREEN TRANSITION STATE ==================== */

typedef enum {
    TRANS_IDLE = 0,
    TRANS_CLOSING,  /* Screen wipes shut */
    TRANS_OPENING   /* Screen wipes open into new screen */
} TransitionState;

static TransitionState g_trans_state        = TRANS_IDLE;
static ScreenID        g_trans_target       = SCREEN_BOOT;
static uint64_t        g_trans_start_time   = 0;
static const uint32_t  TRANS_HALF_DURATION  = 160; /* ms per phase (320ms total) */

/*
 * ui_get_ticks
 *   Returns high-resolution millisecond timestamp since system boot.
 */
uint64_t ui_get_ticks(void)
{
    return GetTickCount64();
}

/*
 * ui_color_lerp
 *   Linearly interpolates between two colors: t = 0.0 -> c1, t = 1.0 -> c2.
 */
COLORREF ui_color_lerp(COLORREF c1, COLORREF c2, float t)
{
    if (t <= 0.0f) return c1;
    if (t >= 1.0f) return c2;
    int r = (int)(GetRValue(c1) + (GetRValue(c2) - GetRValue(c1)) * t);
    int g = (int)(GetGValue(c1) + (GetGValue(c2) - GetGValue(c1)) * t);
    int b = (int)(GetBValue(c1) + (GetBValue(c2) - GetBValue(c1)) * t);
    return RGB(r, g, b);
}

/*
 * ui_start_transition
 *   Initiates a retro arcade screen transition to target_screen.
 */
void ui_start_transition(ScreenID target_screen)
{
    if (g_trans_state != TRANS_IDLE) return;
    g_trans_target     = target_screen;
    g_trans_state      = TRANS_CLOSING;
    g_trans_start_time = GetTickCount64();
}

/*
 * ui_is_transitioning
 *   Returns true while an animated transition is in progress.
 */
bool ui_is_transitioning(void)
{
    return (g_trans_state != TRANS_IDLE);
}

/*
 * ui_update_transition
 *   Advances transition phase. At midpoint, changes the active game screen.
 */
void ui_update_transition(GameState *game)
{
    if (g_trans_state == TRANS_IDLE) return;

    uint64_t now = GetTickCount64();
    uint32_t elapsed = (uint32_t)(now - g_trans_start_time);

    if (g_trans_state == TRANS_CLOSING) {
        if (elapsed >= TRANS_HALF_DURATION) {
            /* Switch the underlying backend screen at midpoint */
            Game_ChangeScreen(game, g_trans_target);
            g_trans_state = TRANS_OPENING;
            g_trans_start_time = now;
        }
    } else if (g_trans_state == TRANS_OPENING) {
        if (elapsed >= TRANS_HALF_DURATION) {
            g_trans_state = TRANS_IDLE;
        }
    }
}

/*
 * ui_draw_transition
 *   Paints the retro CRT shutter & phosphor beam transition overlay
 *   directly onto the double-buffered memory DC.
 */
void ui_draw_transition(void)
{
    if (!g_hdc_mem || g_trans_state == TRANS_IDLE) return;

    int width  = g_client_rect.right - g_client_rect.left;
    int height = g_client_rect.bottom - g_client_rect.top;
    if (width <= 0 || height <= 0) return;

    uint64_t now = GetTickCount64();
    uint32_t elapsed = (uint32_t)(now - g_trans_start_time);
    float t = (float)elapsed / (float)TRANS_HALF_DURATION;
    if (t > 1.0f) t = 1.0f;

    /* Venetian CRT raster shutter strips (each 14px high) */
    const int BAND_H = 14;
    int cover = 0;

    if (g_trans_state == TRANS_CLOSING) {
        cover = (int)(BAND_H * t);
    } else {
        cover = (int)(BAND_H * (1.0f - t));
    }
    if (cover < 0) cover = 0;
    if (cover > BAND_H) cover = BAND_H;

    HBRUSH black_brush = CreateSolidBrush(RGB(10, 10, 14));
    HPEN phosphor_pen = CreatePen(PS_SOLID, 1, RGB(0, 220, 255));
    HPEN old_pen = (HPEN)SelectObject(g_hdc_mem, phosphor_pen);

    for (int y = 0; y < height; y += BAND_H) {
        if (cover > 0) {
            RECT r = { 0, y, width, y + cover };
            FillRect(g_hdc_mem, &r, black_brush);

            /* Luminous CRT phosphor trace along leading edge */
            if (cover < BAND_H) {
                MoveToEx(g_hdc_mem, 0, y + cover, NULL);
                LineTo(g_hdc_mem, width, y + cover);
            }
        }
    }

    /* Central CRT beam flash near the transition midpoint */
    float beam_intensity = 0.0f;
    if (g_trans_state == TRANS_CLOSING && t > 0.7f) {
        beam_intensity = (t - 0.7f) / 0.3f;
    } else if (g_trans_state == TRANS_OPENING && t < 0.3f) {
        beam_intensity = (1.0f - (t / 0.3f));
    }

    if (beam_intensity > 0.05f) {
        int mid_y = height / 2;
        int beam_h = (int)(4 + 20 * (1.0f - beam_intensity));
        RECT beam_rect = { 0, mid_y - beam_h / 2, width, mid_y + beam_h / 2 };
        int glow = (int)(255 * beam_intensity);
        HBRUSH beam_brush = CreateSolidBrush(RGB(glow, glow, glow));
        FillRect(g_hdc_mem, &beam_rect, beam_brush);
        DeleteObject(beam_brush);
    }

    SelectObject(g_hdc_mem, old_pen);
    DeleteObject(phosphor_pen);
    DeleteObject(black_brush);
}