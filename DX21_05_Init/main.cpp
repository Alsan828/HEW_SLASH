#undef UNICODE
#define _CRT_SECURE_NO_WARNINGS
#include "Texture1.h"
#include "Render.h"
#include "Game.h"
#include <windowsx.h> 
#include <atltypes.h>
#include <time.h>

LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

int WINAPI WinMain(_In_ HINSTANCE hInstance, _In_opt_  HINSTANCE hPrevInstance, _In_ LPSTR lpCmdLine, _In_ int nCmdShow)
{
    // Set up window class information
    WNDCLASSEX wc;
    wc.cbSize = sizeof(WNDCLASSEX);
    wc.style = CS_CLASSDC;
    wc.lpfnWndProc = WndProc;
    wc.cbClsExtra = 0;
    wc.cbWndExtra = 0;
    wc.hInstance = hInstance;
    wc.hIcon = NULL;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wc.lpszMenuName = NULL;
    wc.lpszClassName = CLASS_NAME;
    wc.hIconSm = NULL;

    RegisterClassEx(&wc);
    srand(time(NULL));

    // Get screen resolution
    int screenWidth = GetSystemMetrics(SM_CXSCREEN);
    int screenHeight = GetSystemMetrics(SM_CYSCREEN);

    // Create borderless window covering entire screen
    HWND hWnd;

    hWnd = CreateWindowEx(0,
        CLASS_NAME,
        WINDOW_NAME,
        WS_POPUP | WS_VISIBLE,  // Borderless style and immediately visible
        0, 0,                   // Position at (0,0)
        screenWidth,            // Width covers entire screen
        screenHeight,           // Height covers entire screen
        NULL,
        NULL,
        hInstance,
        NULL);

    // Initialize DirectX before entering game loop
    RendererInit(hWnd);
    InitGameWorld();

    MSG msg;

    // Game loop
    while (1)
    {
        // If there are new messages
        if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
        {
            // Send message to window procedure
            DispatchMessage(&msg);

            // Break loop if WM_QUIT message is received
            if (msg.message == WM_QUIT) {
                break;
            }
        }
        else
        {
            GameLoop();  // Handle keyboard input and update game
        }
    }

    // Clean up DirectX resources
    RendererUninit();

    UnregisterClass(CLASS_NAME, hInstance);

    return (int)msg.wParam;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
    case WM_DESTROY:    // Window destruction message
        PostQuitMessage(0); // Send WM_QUIT message → Application termination
        break;

    case WM_CLOSE:      // When 'x' button is pressed
    {
        int res = MessageBoxA(NULL, "Are you sure you want to exit?", "Confirmation", MB_OKCANCEL);
        if (res == IDOK) {
            DestroyWindow(hWnd);  // Send WM_DESTROY message
        }
    }
    break;

    case WM_KEYDOWN: // Key input message
        if (LOWORD(wParam) == VK_ESCAPE)
        { // If ESCAPE key is pressed
            PostMessage(hWnd, WM_CLOSE, wParam, lParam); // Send WM_CLOSE message
        }
        break;

    default:
        // Execute default processing for received message
        return DefWindowProc(hWnd, uMsg, wParam, lParam);
        break;
    }

    return 0;
}