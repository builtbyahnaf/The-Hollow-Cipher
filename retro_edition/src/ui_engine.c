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
 *     - Heap-managed UI engine context, dynamic text caching, and typed structs/enums.
 *
 *   Architecture note:
 *     Frontend rendering module. It depends on Windows GDI and ui_engine.h.
 *     Core game logic remains completely decoupled and oblivious to GDI.
 */

#include "ui_engine.h"
#include <wchar.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* ==================== PRIVATE ENUMS & STRUCTS ==================== */

typedef enum
{
    TRANS_IDLE = 0,
    TRANS_CLOSING,  /* Screen wipes shut */
    TRANS_OPENING   /* Screen wipes open into new screen */
} TransitionState;

typedef enum
{
    EFFECT_NONE = 0,
    EFFECT_SCANLINES,
    EFFECT_CRT_GLOW
} UIRenderEffect;

/* Monospace cell and screen grid geometry */
typedef struct
{
    int char_width;
    int char_height;
    int cols;
    int rows;
    int margin_x;
    int margin_y;
} UIGridMetrics;

/* Double-buffering GDI device context state */
typedef struct
{
    PAINTSTRUCT ps;
    HDC         hdc_screen;
    HDC         hdc_mem;
    HBITMAP     hbm_mem;
    HBITMAP     hbm_old;
    HFONT       hfont_old;
    RECT        client_rect;
} UIRenderContext;

/* CRT shutter transition animation controller */
typedef struct
{
    TransitionState state;
    ScreenID        target_screen;
    uint64_t        start_time;
    uint32_t        half_duration_ms;
} UITransitionEngine;

/* Dynamically allocated heap buffer for UTF-8 -> UTF-16 wide character conversion */
typedef struct
{
    wchar_t *buffer;
    size_t   capacity;
} UIDynamicBuffer;

/* Master UI Engine State structure (dynamically allocated) */
typedef struct
{
    HWND               hwnd;
    HFONT              retro_font;
    UIGridMetrics      grid;
    UIRenderContext    render;
    UITransitionEngine transition;
    UIDynamicBuffer    text_cache;
    UIRenderEffect     active_effect;
} UIEngineState;

/* Global engine instance pointer */
static UIEngineState *g_ui = NULL;

/* Default timing constants */
static const uint32_t DEFAULT_TRANS_HALF_DURATION = 160; /* ms per phase (320ms total) */
static const size_t   DEFAULT_TEXT_BUFFER_CAP      = 1024;

/* ==================== PRIVATE HELPER FUNCTIONS ==================== */

/* Allocate and initialize the UIEngineState on heap if not yet created */
static bool ui_ensure_context(HWND hwnd)
{
    if (g_ui)
    {
        if (hwnd) g_ui->hwnd = hwnd;
        return true;
    }

    g_ui = (UIEngineState *)calloc(1, sizeof(UIEngineState));
    if (!g_ui) return false;

    g_ui->hwnd = hwnd;
    g_ui->grid.char_width  = 10;
    g_ui->grid.char_height = 20;
    g_ui->grid.cols        = 80;
    g_ui->grid.rows        = 30;
    g_ui->grid.margin_x    = 16;
    g_ui->grid.margin_y    = 16;

    g_ui->transition.state            = TRANS_IDLE;
    g_ui->transition.target_screen    = SCREEN_BOOT;
    g_ui->transition.start_time       = 0;
    g_ui->transition.half_duration_ms = DEFAULT_TRANS_HALF_DURATION;

    g_ui->active_effect = EFFECT_SCANLINES;

    /* Dynamic text buffer allocation */
    g_ui->text_cache.capacity = DEFAULT_TEXT_BUFFER_CAP;
    g_ui->text_cache.buffer   = (wchar_t *)malloc(g_ui->text_cache.capacity * sizeof(wchar_t));
    if (g_ui->text_cache.buffer)
    {
        g_ui->text_cache.buffer[0] = L'\0';
    }

    return true;
}

/* ==================== INITIALIZATION / SHUTDOWN ==================== */

/*
 * ui_init
 *   Initializes font resources and measures character dimensions.
 *   Creates a crisp monospace retro-terminal font.
 */
void ui_init(HWND hwnd)
{
    if (!ui_ensure_context(hwnd)) return;

    /* Create crisp retro monospace font */
    g_ui->retro_font = CreateFontA(
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
    if (temp_dc)
    {
        HFONT old_font = (HFONT)SelectObject(temp_dc, g_ui->retro_font);
        TEXTMETRICA tm;
        if (GetTextMetricsA(temp_dc, &tm))
        {
            g_ui->grid.char_height = tm.tmHeight;
            g_ui->grid.char_width  = tm.tmAveCharWidth;
        }
        SelectObject(temp_dc, old_font);
        ReleaseDC(hwnd, temp_dc);
    }

    /* Update client area and calculate grid bounds */
    GetClientRect(hwnd, &g_ui->render.client_rect);
    int client_w = g_ui->render.client_rect.right - g_ui->render.client_rect.left;
    int client_h = g_ui->render.client_rect.bottom - g_ui->render.client_rect.top;

    if (g_ui->grid.char_width > 0 && g_ui->grid.char_height > 0)
    {
        g_ui->grid.cols = (client_w - (2 * g_ui->grid.margin_x)) / g_ui->grid.char_width;
        g_ui->grid.rows = (client_h - (2 * g_ui->grid.margin_y)) / g_ui->grid.char_height;
        if (g_ui->grid.cols < 40) g_ui->grid.cols = 40;
        if (g_ui->grid.rows < 20) g_ui->grid.rows = 20;
    }
}

/*
 * ui_shutdown
 *   Releases font objects, dynamic heap buffers, and UI context.
 */
void ui_shutdown(void)
{
    if (!g_ui) return;

    if (g_ui->retro_font)
    {
        DeleteObject(g_ui->retro_font);
        g_ui->retro_font = NULL;
    }

    if (g_ui->text_cache.buffer)
    {
        free(g_ui->text_cache.buffer);
        g_ui->text_cache.buffer   = NULL;
        g_ui->text_cache.capacity = 0;
    }

    free(g_ui);
    g_ui = NULL;
}

/* ==================== PAINT CYCLE (DOUBLE BUFFERING) ==================== */

/*
 * ui_begin_paint
 *   Sets up off-screen compatible memory DC and bitmap for flicker-free rendering.
 */
void ui_begin_paint(HWND hwnd)
{
    if (!ui_ensure_context(hwnd)) return;

    g_ui->hwnd = hwnd;
    g_ui->render.hdc_screen = BeginPaint(hwnd, &g_ui->render.ps);

    GetClientRect(hwnd, &g_ui->render.client_rect);
    int width  = g_ui->render.client_rect.right - g_ui->render.client_rect.left;
    int height = g_ui->render.client_rect.bottom - g_ui->render.client_rect.top;

    if (width <= 0) width = 800;
    if (height <= 0) height = 600;

    /* Recompute column and row capacity based on current window size */
    if (g_ui->grid.char_width > 0 && g_ui->grid.char_height > 0)
    {
        g_ui->grid.cols = (width - (2 * g_ui->grid.margin_x)) / g_ui->grid.char_width;
        g_ui->grid.rows = (height - (2 * g_ui->grid.margin_y)) / g_ui->grid.char_height;
        if (g_ui->grid.cols < 40) g_ui->grid.cols = 40;
        if (g_ui->grid.rows < 20) g_ui->grid.rows = 20;
    }

    /* Create compatible DC and back-buffer bitmap */
    g_ui->render.hdc_mem = CreateCompatibleDC(g_ui->render.hdc_screen);
    g_ui->render.hbm_mem = CreateCompatibleBitmap(g_ui->render.hdc_screen, width, height);
    g_ui->render.hbm_old = (HBITMAP)SelectObject(g_ui->render.hdc_mem, g_ui->render.hbm_mem);

    /* Setup font and text attributes on the memory DC */
    g_ui->render.hfont_old = (HFONT)SelectObject(g_ui->render.hdc_mem, g_ui->retro_font);
    SetBkMode(g_ui->render.hdc_mem, TRANSPARENT);
}

/*
 * ui_end_paint
 *   Blits the off-screen buffer to the screen DC and disposes back-buffer handles.
 */
void ui_end_paint(void)
{
    if (!g_ui) return;

    if (g_ui->render.hdc_screen && g_ui->render.hdc_mem)
    {
        int width  = g_ui->render.client_rect.right - g_ui->render.client_rect.left;
        int height = g_ui->render.client_rect.bottom - g_ui->render.client_rect.top;

        /* Single BitBlt to display surface */
        BitBlt(g_ui->render.hdc_screen, 0, 0, width, height, g_ui->render.hdc_mem, 0, 0, SRCCOPY);

        /* Cleanup memory DC */
        SelectObject(g_ui->render.hdc_mem, g_ui->render.hfont_old);
        SelectObject(g_ui->render.hdc_mem, g_ui->render.hbm_old);
        DeleteObject(g_ui->render.hbm_mem);
        DeleteDC(g_ui->render.hdc_mem);
    }

    EndPaint(g_ui->hwnd, &g_ui->render.ps);

    g_ui->render.hdc_screen = NULL;
    g_ui->render.hdc_mem    = NULL;
    g_ui->render.hbm_mem    = NULL;
}

/* ==================== BACKGROUND & SCANLINES ==================== */

/*
 * ui_fill_background
 *   Paints dark background and applies CRT monitor scanline raster effect.
 */
void ui_fill_background(void)
{
    if (!g_ui || !g_ui->render.hdc_mem) return;

    /* Solid dark background fill */
    HBRUSH bg_brush = CreateSolidBrush(CLR_BG);
    FillRect(g_ui->render.hdc_mem, &g_ui->render.client_rect, bg_brush);
    DeleteObject(bg_brush);

    /* Subtle CRT scanlines: draw every 3rd pixel line */
    if (g_ui->active_effect == EFFECT_SCANLINES)
    {
        HPEN scanline_pen = CreatePen(PS_SOLID, 1, RGB(10, 10, 14));
        HPEN old_pen = (HPEN)SelectObject(g_ui->render.hdc_mem, scanline_pen);

        int width = g_ui->render.client_rect.right;
        int height = g_ui->render.client_rect.bottom;

        for (int y = 0; y < height; y += 3)
        {
            MoveToEx(g_ui->render.hdc_mem, 0, y, NULL);
            LineTo(g_ui->render.hdc_mem, width, y);
        }

        SelectObject(g_ui->render.hdc_mem, old_pen);
        DeleteObject(scanline_pen);
    }
}

/* ==================== GRID QUERIES ==================== */

int ui_get_cols(void)
{
    return g_ui ? g_ui->grid.cols : 80;
}

int ui_get_rows(void)
{
    return g_ui ? g_ui->grid.rows : 30;
}

/*
 * ui_on_resize
 *   Recalculates character grid dimensions when window size changes.
 */
void ui_on_resize(HWND hwnd)
{
    if (!hwnd || !ui_ensure_context(hwnd)) return;

    RECT rect;
    GetClientRect(hwnd, &rect);
    int width  = rect.right - rect.left;
    int height = rect.bottom - rect.top;

    if (width < 400) width = 400;
    if (height < 300) height = 300;

    if (g_ui->grid.char_width > 0 && g_ui->grid.char_height > 0)
    {
        g_ui->grid.cols = (width - (2 * g_ui->grid.margin_x)) / g_ui->grid.char_width;
        g_ui->grid.rows = (height - (2 * g_ui->grid.margin_y)) / g_ui->grid.char_height;
        if (g_ui->grid.cols < 40) g_ui->grid.cols = 40;
        if (g_ui->grid.rows < 20) g_ui->grid.rows = 20;
    }
}

/* ==================== TEXT DRAWING ==================== */

/*
 * ui_draw_text
 *   Renders UTF-8 text string at grid position (row, col) with custom color.
 *   Uses dynamically allocated/expanded heap buffer if text exceeds default cache.
 */
void ui_draw_text(int row, int col, const char *text, COLORREF color)
{
    if (!g_ui || !g_ui->render.hdc_mem || !text) return;

    int required_len = MultiByteToWideChar(CP_UTF8, 0, text, -1, NULL, 0);
    if (required_len <= 1) return;

    wchar_t *wbuf = NULL;
    bool free_needed = false;

    if ((size_t)required_len <= g_ui->text_cache.capacity && g_ui->text_cache.buffer)
    {
        wbuf = g_ui->text_cache.buffer;
    }
    else
    {
        /* Dynamic heap allocation for oversized strings */
        wbuf = (wchar_t *)malloc((size_t)required_len * sizeof(wchar_t));
        if (!wbuf) return;
        free_needed = true;
    }

    int converted_len = MultiByteToWideChar(CP_UTF8, 0, text, -1, wbuf, required_len);
    if (converted_len > 1)
    {
        int x = g_ui->grid.margin_x + col * g_ui->grid.char_width;
        int y = g_ui->grid.margin_y + row * g_ui->grid.char_height;

        SetTextColor(g_ui->render.hdc_mem, color);
        TextOutW(g_ui->render.hdc_mem, x, y, wbuf, converted_len - 1);
    }

    if (free_needed)
    {
        free(wbuf);
    }
}

/*
 * ui_draw_text_centered
 *   Renders a text string centered horizontally across the window grid.
 */
void ui_draw_text_centered(int row, const char *text, COLORREF color)
{
    if (!g_ui || !g_ui->render.hdc_mem || !text) return;

    int req = MultiByteToWideChar(CP_UTF8, 0, text, -1, NULL, 0);
    int char_count = (req > 1) ? (req - 1) : (int)strlen(text);

    int col = (g_ui->grid.cols - char_count) / 2;
    if (col < 0) col = 0;

    ui_draw_text(row, col, text, color);
}

/*
 * ui_draw_text_in_region
 *   Renders a text string horizontally centered inside a specified column sub-region.
 */
void ui_draw_text_in_region(int row, int region_col, int region_width,
                            const char *text, COLORREF color)
{
    if (!g_ui || !g_ui->render.hdc_mem || !text) return;

    int req = MultiByteToWideChar(CP_UTF8, 0, text, -1, NULL, 0);
    int len = (req > 1) ? (req - 1) : (int)strlen(text);

    int col = region_col + (region_width - len) / 2;
    if (col < region_col) col = region_col;

    ui_draw_text(row, col, text, color);
}

/* ==================== PANEL & SHAPE DRAWING ==================== */

/*
 * ui_draw_panel
 *   Draws a bordered retro panel with background fill and inset header title.
 */
void ui_draw_panel(int row, int col, int width, int height, const char *title)
{
    if (!g_ui || !g_ui->render.hdc_mem) return;

    RECT r;
    r.left   = g_ui->grid.margin_x + col * g_ui->grid.char_width;
    r.top    = g_ui->grid.margin_y + row * g_ui->grid.char_height;
    r.right  = r.left + width * g_ui->grid.char_width;
    r.bottom = r.top + height * g_ui->grid.char_height;

    /* Fill panel interior */
    HBRUSH bg_brush = CreateSolidBrush(CLR_PANEL_BG);
    FillRect(g_ui->render.hdc_mem, &r, bg_brush);
    DeleteObject(bg_brush);

    /* Draw border lines */
    HPEN border_pen = CreatePen(PS_SOLID, 2, CLR_BORDER);
    HPEN old_pen = (HPEN)SelectObject(g_ui->render.hdc_mem, border_pen);
    HBRUSH null_brush = (HBRUSH)GetStockObject(NULL_BRUSH);
    HBRUSH old_brush = (HBRUSH)SelectObject(g_ui->render.hdc_mem, null_brush);

    Rectangle(g_ui->render.hdc_mem, r.left, r.top, r.right, r.bottom);

    SelectObject(g_ui->render.hdc_mem, old_pen);
    SelectObject(g_ui->render.hdc_mem, old_brush);
    DeleteObject(border_pen);

    /* Draw inset title banner if provided */
    if (title && strlen(title) > 0)
    {
        char title_buf[256];
        snprintf(title_buf, sizeof(title_buf), " %s ", title);
        int title_len = (int)strlen(title_buf);
        int title_col = col + (width - title_len) / 2;
        if (title_col < col + 1) title_col = col + 1;

        int title_x = g_ui->grid.margin_x + title_col * g_ui->grid.char_width;
        int title_y = r.top - (g_ui->grid.char_height / 3);

        RECT badge_r;
        badge_r.left   = title_x;
        badge_r.top    = r.top - 2;
        badge_r.right  = title_x + title_len * g_ui->grid.char_width;
        badge_r.bottom = r.top + g_ui->grid.char_height / 2;

        HBRUSH title_bg = CreateSolidBrush(CLR_PANEL_BG);
        FillRect(g_ui->render.hdc_mem, &badge_r, title_bg);
        DeleteObject(title_bg);

        /* Draw title with wide text support */
        wchar_t wtitle[256];
        int wlen = MultiByteToWideChar(CP_UTF8, 0, title_buf, -1, wtitle, 256);
        if (wlen > 1)
        {
            SetTextColor(g_ui->render.hdc_mem, CLR_ACCENT);
            TextOutW(g_ui->render.hdc_mem, title_x, title_y, wtitle, wlen - 1);
        }
    }
}

void ui_draw_panel_rect(const UIGridRect *rect, const char *title)
{
    if (!rect) return;
    ui_draw_panel(rect->row, rect->col, rect->width, rect->height, title);
}

/*
 * ui_draw_hline
 *   Draws a 1px horizontal separator rule across the specified grid row.
 */
void ui_draw_hline(int row, int col, int width, COLORREF color)
{
    if (!g_ui || !g_ui->render.hdc_mem) return;

    int x1 = g_ui->grid.margin_x + col * g_ui->grid.char_width;
    int x2 = x1 + width * g_ui->grid.char_width;
    int y  = g_ui->grid.margin_y + row * g_ui->grid.char_height + (g_ui->grid.char_height / 2);

    HPEN pen = CreatePen(PS_SOLID, 1, color);
    HPEN old_pen = (HPEN)SelectObject(g_ui->render.hdc_mem, pen);

    MoveToEx(g_ui->render.hdc_mem, x1, y, NULL);
    LineTo(g_ui->render.hdc_mem, x2, y);

    SelectObject(g_ui->render.hdc_mem, old_pen);
    DeleteObject(pen);
}

/*
 * ui_draw_menu_item
 *   Draws a selectable menu item row with highlight background when selected.
 */
void ui_draw_menu_item(int row, int col, int width, const char *text, bool selected)
{
    if (!g_ui || !g_ui->render.hdc_mem || !text) return;

    int x = g_ui->grid.margin_x + col * g_ui->grid.char_width;
    int y = g_ui->grid.margin_y + row * g_ui->grid.char_height;

    char buf[256];
    if (selected)
    {
        RECT r;
        r.left   = x;
        r.top    = y;
        r.right  = x + width * g_ui->grid.char_width;
        r.bottom = y + g_ui->grid.char_height;

        HBRUSH hl_brush = CreateSolidBrush(CLR_HIGHLIGHT_BG);
        FillRect(g_ui->render.hdc_mem, &r, hl_brush);
        DeleteObject(hl_brush);

        snprintf(buf, sizeof(buf), "> %s", text);
        ui_draw_text(row, col, buf, CLR_HIGHLIGHT_FG);
    }
    else
    {
        snprintf(buf, sizeof(buf), "  %s", text);
        ui_draw_text(row, col, buf, CLR_TEXT);
    }
}

/*
 * ui_fill_rect
 *   Paints a solid filled rectangle in grid coordinates.
 */
void ui_fill_rect(int row, int col, int width, int height, COLORREF color)
{
    if (!g_ui || !g_ui->render.hdc_mem) return;

    RECT r;
    r.left   = g_ui->grid.margin_x + col * g_ui->grid.char_width;
    r.top    = g_ui->grid.margin_y + row * g_ui->grid.char_height;
    r.right  = r.left + width * g_ui->grid.char_width;
    r.bottom = r.top + height * g_ui->grid.char_height;

    HBRUSH brush = CreateSolidBrush(color);
    FillRect(g_ui->render.hdc_mem, &r, brush);
    DeleteObject(brush);
}

void ui_fill_grid_rect(const UIGridRect *rect, COLORREF color)
{
    if (!rect) return;
    ui_fill_rect(rect->row, rect->col, rect->width, rect->height, color);
}

/* ==================== INPUT TRANSLATION ==================== */

KeyCode ui_translate_key(WPARAM wParam)
{
    switch (wParam)
    {
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

uint64_t ui_get_ticks(void)
{
    return GetTickCount64();
}

COLORREF ui_color_lerp(COLORREF c1, COLORREF c2, float t)
{
    if (t <= 0.0f) return c1;
    if (t >= 1.0f) return c2;
    int r = (int)(GetRValue(c1) + (GetRValue(c2) - GetRValue(c1)) * t);
    int g = (int)(GetGValue(c1) + (GetGValue(c2) - GetGValue(c1)) * t);
    int b = (int)(GetBValue(c1) + (GetBValue(c2) - GetBValue(c1)) * t);
    return RGB(r, g, b);
}

/* ==================== SCREEN TRANSITION SYSTEM ==================== */

void ui_start_transition(ScreenID target_screen)
{
    if (!g_ui || g_ui->transition.state != TRANS_IDLE) return;
    g_ui->transition.target_screen = target_screen;
    g_ui->transition.state         = TRANS_CLOSING;
    g_ui->transition.start_time    = GetTickCount64();
}

bool ui_is_transitioning(void)
{
    return (g_ui && g_ui->transition.state != TRANS_IDLE);
}

void ui_update_transition(GameState *game)
{
    if (!g_ui || g_ui->transition.state == TRANS_IDLE) return;

    uint64_t now     = GetTickCount64();
    uint32_t elapsed = (uint32_t)(now - g_ui->transition.start_time);

    if (g_ui->transition.state == TRANS_CLOSING)
    {
        if (elapsed >= g_ui->transition.half_duration_ms)
        {
            /* Transition halfway: swap active backend screen */
            Game_ChangeScreen(game, g_ui->transition.target_screen);
            g_ui->transition.state      = TRANS_OPENING;
            g_ui->transition.start_time = now;
        }
    }
    else if (g_ui->transition.state == TRANS_OPENING)
    {
        if (elapsed >= g_ui->transition.half_duration_ms)
        {
            g_ui->transition.state = TRANS_IDLE;
        }
    }
}

void ui_draw_transition(void)
{
    if (!g_ui || !g_ui->render.hdc_mem || g_ui->transition.state == TRANS_IDLE) return;

    int width  = g_ui->render.client_rect.right - g_ui->render.client_rect.left;
    int height = g_ui->render.client_rect.bottom - g_ui->render.client_rect.top;
    if (width <= 0 || height <= 0) return;

    uint64_t now     = GetTickCount64();
    uint32_t elapsed = (uint32_t)(now - g_ui->transition.start_time);
    float t = (float)elapsed / (float)g_ui->transition.half_duration_ms;
    if (t > 1.0f) t = 1.0f;

    const int BAND_H = 14;
    int cover = 0;

    if (g_ui->transition.state == TRANS_CLOSING)
    {
        cover = (int)(BAND_H * t);
    }
    else
    {
        cover = (int)(BAND_H * (1.0f - t));
    }
    if (cover < 0) cover = 0;
    if (cover > BAND_H) cover = BAND_H;

    HBRUSH black_brush  = CreateSolidBrush(RGB(10, 10, 14));
    HPEN   phosphor_pen = CreatePen(PS_SOLID, 1, RGB(0, 220, 255));
    HPEN   old_pen      = (HPEN)SelectObject(g_ui->render.hdc_mem, phosphor_pen);

    for (int y = 0; y < height; y += BAND_H)
    {
        if (cover > 0)
        {
            RECT r = { 0, y, width, y + cover };
            FillRect(g_ui->render.hdc_mem, &r, black_brush);

            if (cover < BAND_H)
            {
                MoveToEx(g_ui->render.hdc_mem, 0, y + cover, NULL);
                LineTo(g_ui->render.hdc_mem, width, y + cover);
            }
        }
    }

    /* Beam glow flash at transition apex */
    float beam_intensity = 0.0f;
    if (g_ui->transition.state == TRANS_CLOSING && t > 0.7f)
    {
        beam_intensity = (t - 0.7f) / 0.3f;
    }
    else if (g_ui->transition.state == TRANS_OPENING && t < 0.3f)
    {
        beam_intensity = (1.0f - (t / 0.3f));
    }

    if (beam_intensity > 0.05f)
    {
        int mid_y  = height / 2;
        int beam_h = (int)(4 + 20 * (1.0f - beam_intensity));
        RECT beam_rect = { 0, mid_y - beam_h / 2, width, mid_y + beam_h / 2 };
        int glow = (int)(255 * beam_intensity);
        HBRUSH beam_brush = CreateSolidBrush(RGB(glow, glow, glow));
        FillRect(g_ui->render.hdc_mem, &beam_rect, beam_brush);
        DeleteObject(beam_brush);
    }

    SelectObject(g_ui->render.hdc_mem, old_pen);
    DeleteObject(phosphor_pen);
    DeleteObject(black_brush);
}