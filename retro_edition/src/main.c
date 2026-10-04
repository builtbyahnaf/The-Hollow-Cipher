/*
 * File: main.c
 * Project: The Hollow Cipher
 *
 * Description:
 *   Win32 GUI application entry point and event dispatch pump.
 *   Initializes GameState, registers window class, creates the main desktop window,
 *   runs the standard Windows message loop, and dispatches WM_PAINT and WM_KEYDOWN
 *   events to the currently active screen module.
 *
 * Architecture note:
 *   Frontend integration layer:
 *     - Owns the GameState instance and passes it to Game_Init() and Game_Shutdown().
 *     - Forwards user inputs to Screen_HandleKey().
 *     - Renders visual frames via Screen_Draw() inside WM_PAINT using ui_engine.
 *     - Decoupled from core game logic: screens can be swapped or tested independently.
 */

#include <windows.h>
#include <stdbool.h>
#include "game.h"
#include "ui_engine.h"
#include "screens.h"

/* ==================== GLOBAL APPLICATION STATE ==================== */

static GameState g_game;

/* ==================== WINDOW EVENT DISPATCH (WNDPROC) ==================== */

/*
 * MainWndProc
 *   Central Windows Procedure. Routes OS messages to appropriate frontend handlers.
 */
static LRESULT CALLBACK MainWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg) {
        case WM_CREATE:
            /* Initialize GDI font and grid metrics */
            ui_init(hwnd);
            /* Timer for CRT scanline refresh and blinking prompts (~60 FPS) */
            SetTimer(hwnd, 1, 16, NULL);
            return 0;

        case WM_TIMER:
            /* Invalidate window client area to drive animations */
            InvalidateRect(hwnd, NULL, FALSE);
            return 0;

        case WM_SIZE:
            /* Handle window resize - recalculate grid immediately */
            ui_on_resize(hwnd);
            return 0;

        case WM_GETMINMAXINFO:
            {
                MINMAXINFO *mmi = (MINMAXINFO *)lParam;
                /* Minimum client area: 400x300 + window frame */
                RECT rc = { 0, 0, 400, 300 };
                DWORD dwStyle = WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX | WS_THICKFRAME | WS_MAXIMIZEBOX;
                AdjustWindowRect(&rc, dwStyle, FALSE);
                mmi->ptMinTrackSize.x = rc.right - rc.left;
                mmi->ptMinTrackSize.y = rc.bottom - rc.top;
            }
            return 0;

        case WM_ERASEBKGND:
            /* Suppress default background erasing to prevent flicker (double-buffered in WM_PAINT) */
            return 1;

        case WM_PAINT:
            /* Advance screen transition animation state */
            ui_update_transition(&g_game);

            ui_begin_paint(hwnd);
            ui_fill_background();

            /* Route draw command to currently active screen */
            switch (g_game.currentScreen) {
                case SCREEN_BOOT:
                    Boot_Draw(&g_game);
                    break;
                case SCREEN_MAIN_MENU:
                    Home_Draw(&g_game);
                    break;
                case SCREEN_OPTIONS:
                    Options_Draw(&g_game);
                    break;
                case SCREEN_ABOUT:
                    About_Draw(&g_game);
                    break;
                case SCREEN_LEVEL_SELECT:
                    LevelSelect_Draw(&g_game);
                    break;
                case SCREEN_INTERROGATION:
                case SCREEN_FORENSICS:
                case SCREEN_EVIDENCE:
                case SCREEN_HISTORY:
                    StubScreen_Draw(&g_game);
                    break;
                case SCREEN_LEVEL_MAIN:
                    LevelMain_Draw(&g_game);
                    break;
                case SCREEN_EXIT:
                    PostMessage(hwnd, WM_CLOSE, 0, 0);
                    break;
                default:
                    break;
            }

            /* Draw CRT transition overlay on top of back-buffer */
            ui_draw_transition();

            ui_end_paint();
            return 0;

        case WM_KEYDOWN:
            {
                /* Ignore key inputs while screen transition is in flight */
                if (ui_is_transitioning()) {
                    return 0;
                }

                KeyCode key = ui_translate_key(wParam);
                if (key != KEY_NONE) {
                    /* Route input to currently active screen */
                    switch (g_game.currentScreen) {
                        case SCREEN_BOOT:
                            Boot_HandleKey(&g_game, key);
                            break;
                        case SCREEN_MAIN_MENU:
                            Home_HandleKey(&g_game, key);
                            break;
                        case SCREEN_OPTIONS:
                            Options_HandleKey(&g_game, key);
                            break;
                        case SCREEN_ABOUT:
                            About_HandleKey(&g_game, key);
                            break;
                        case SCREEN_LEVEL_SELECT:
                            LevelSelect_HandleKey(&g_game, key);
                            break;
                        case SCREEN_INTERROGATION:
                        case SCREEN_FORENSICS:
                        case SCREEN_EVIDENCE:
                        case SCREEN_HISTORY:
                            StubScreen_HandleKey(&g_game, key);
                            break;
                        case SCREEN_LEVEL_MAIN:
                            LevelMain_HandleKey(&g_game, key);
                            break;
                        default:
                            break;
                    }

                    /* If any screen requested application exit, close window */
                    if (!g_game.running || g_game.currentScreen == SCREEN_EXIT) {
                        PostMessage(hwnd, WM_CLOSE, 0, 0);
                    }

                    /* Immediately request window repaint on key response */
                    InvalidateRect(hwnd, NULL, FALSE);
                }
            }
            return 0;

        case WM_CHAR:
            {
                if (ui_is_transitioning()) {
                    return 0;
                }
                /* Handle character input for text fields */
                if (g_game.currentScreen == SCREEN_LEVEL_MAIN) {
                    LevelMain_HandleChar(&g_game, (char)wParam);
                    InvalidateRect(hwnd, NULL, FALSE);
                }
            }
            return 0;

        case WM_DESTROY:
            KillTimer(hwnd, 1);
            ui_shutdown();
            PostQuitMessage(0);
            return 0;

        default:
            return DefWindowProcA(hwnd, uMsg, wParam, lParam);
    }
}

/* ==================== APPLICATION ENTRY POINT ==================== */

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    (void)hPrevInstance;
    (void)lpCmdLine;

    /* 0. Enable Per-Monitor High-DPI awareness so Windows does not virtualize/scale window dimensions */
    HMODULE hUser32 = GetModuleHandleA("user32.dll");
    if (hUser32) {
        typedef BOOL (WINAPI *SetProcessDpiAwarenessContextProc)(HANDLE);
        SetProcessDpiAwarenessContextProc setDpiContext =
            (SetProcessDpiAwarenessContextProc)(void *)GetProcAddress(hUser32, "SetProcessDpiAwarenessContext");
        if (setDpiContext) {
            /* DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2 = ((HANDLE)-4) */
            setDpiContext((HANDLE)-4);
        } else {
            typedef BOOL (WINAPI *SetProcessDPIAwareProc)(void);
            SetProcessDPIAwareProc setDpiAware =
                (SetProcessDPIAwareProc)(void *)GetProcAddress(hUser32, "SetProcessDPIAware");
            if (setDpiAware) {
                setDpiAware();
            }
        }
    }

    /* 1. Initialize core game backend state */
    Game_Init(&g_game);

    /* 2. Register Windows Window Class */
    const char CLASS_NAME[] = "TheHollowCipherRetroWndClass";

    WNDCLASSEXA wcex;
    ZeroMemory(&wcex, sizeof(wcex));
    wcex.cbSize        = sizeof(WNDCLASSEXA);
    wcex.style         = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc   = MainWndProc;
    wcex.hInstance     = hInstance;
    wcex.hCursor       = LoadCursor(NULL, IDC_ARROW);
    wcex.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
    wcex.lpszClassName = CLASS_NAME;

    if (!RegisterClassExA(&wcex)) {
        MessageBoxA(NULL, "Failed to register window class.", "Error", MB_ICONERROR | MB_OK);
        return 1;
    }

    /* 3. Compute window outer dimension for desired client resolution (1474x829)
     * Target 16:9 widescreen layout for retro detective workstation */
    const int DESIRED_CLIENT_WIDTH  = 1474;
    const int DESIRED_CLIENT_HEIGHT = 829;

    RECT wr = { 0, 0, DESIRED_CLIENT_WIDTH, DESIRED_CLIENT_HEIGHT };
    /* Window style with resize support: WS_THICKFRAME + WS_MAXIMIZEBOX enables dragging resize */
    DWORD dwStyle = WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX | WS_THICKFRAME | WS_MAXIMIZEBOX;
    AdjustWindowRectEx(&wr, dwStyle, FALSE, 0);

    int win_width  = wr.right - wr.left;
    int win_height = wr.bottom - wr.top;

    /* Center on screen */
    int screen_w = GetSystemMetrics(SM_CXSCREEN);
    int screen_h = GetSystemMetrics(SM_CYSCREEN);
    int pos_x = (screen_w - win_width) / 2;
    int pos_y = (screen_h - win_height) / 2;
    if (pos_x < 0) pos_x = 0;
    if (pos_y < 0) pos_y = 0;

    /* 4. Create Main Application Window */
    HWND hwnd = CreateWindowExA(
        0,
        CLASS_NAME,
        "The Hollow Cipher - Retro Edition (KUET CSE 1-1)",
        dwStyle,
        pos_x, pos_y,
        win_width, win_height,
        NULL,
        NULL,
        hInstance,
        NULL
    );

    if (!hwnd) {
        MessageBoxA(NULL, "Failed to create application window.", "Error", MB_ICONERROR | MB_OK);
        return 1;
    }

    /* 5. Calibrate exact client size (compensating for DWM invisible resizing borders) */
    RECT rcClient, rcWindow;
    GetClientRect(hwnd, &rcClient);
    GetWindowRect(hwnd, &rcWindow);

    int diff_w = DESIRED_CLIENT_WIDTH  - (rcClient.right - rcClient.left);
    int diff_h = DESIRED_CLIENT_HEIGHT - (rcClient.bottom - rcClient.top);

    if (diff_w != 0 || diff_h != 0) {
        int final_w = (rcWindow.right - rcWindow.left) + diff_w;
        int final_h = (rcWindow.bottom - rcWindow.top) + diff_h;
        int final_x = (screen_w - final_w) / 2;
        int final_y = (screen_h - final_h) / 2;
        if (final_x < 0) final_x = 0;
        if (final_y < 0) final_y = 0;

        SetWindowPos(hwnd, NULL, final_x, final_y, final_w, final_h,
                     SWP_NOZORDER | SWP_NOACTIVATE | SWP_FRAMECHANGED);
    }

    /* Re-evaluate grid with calibrated dimensions */
    ui_on_resize(hwnd);

    ShowWindow(hwnd, (nCmdShow == SW_SHOWMAXIMIZED) ? SW_SHOWMAXIMIZED : SW_SHOWNORMAL);
    UpdateWindow(hwnd);

    /* 6. Standard Windows Message Loop */
    MSG msg;
    while (GetMessageA(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessageA(&msg);
    }

    /* 7. Graceful backend shutdown */
    Game_Shutdown(&g_game);

    return (int)msg.wParam;
}

/* Fallback entry point in case compiler targets console runtime */
int main(int argc, char **argv)
{
    (void)argc;
    (void)argv;
    return WinMain(GetModuleHandle(NULL), NULL, GetCommandLineA(), SW_SHOWDEFAULT);
}
