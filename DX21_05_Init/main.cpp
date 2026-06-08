#undef UNICODE
#define _CRT_SECURE_NO_WARNINGS
#include "Texture1.h"
#include "Render.h"
#include "Camera.h"
#include "Game.h"
#include <windowsx.h> 
#include <atltypes.h>
#include <time.h>
#include "SceneManager.h"

SceneManager sceneManager;

LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

int WINAPI WinMain(_In_ HINSTANCE hInstance, _In_opt_  HINSTANCE hPrevInstance, _In_ LPSTR lpCmdLine, _In_ int nCmdShow)
{
    // ウィンドウクラス情報を設定する
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
    srand(static_cast<unsigned int>(time(NULL)));

    // 画面解像度を取得する
    int screenWidth = GetSystemMetrics(SM_CXSCREEN);
    int screenHeight = GetSystemMetrics(SM_CYSCREEN);

    g_windowWidth = screenWidth;
    g_windowHeight = screenHeight;

    // 画面全体を覆うボーダーレスウィンドウを生成する
    HWND hWnd;

    hWnd = CreateWindowEx(0,
        CLASS_NAME,
        WINDOW_NAME,
        WS_POPUP | WS_VISIBLE,  // ボーダーレスで即時表示
        0, 0,                   // 位置は (0, 0)
        screenWidth,            // 幅は画面全体
        screenHeight,           // 高さは画面全体
        NULL,
        NULL,
        hInstance,
        NULL);

    // ゲームループに入る前に DirectX を初期化する
    RendererInit(hWnd);    // レンダラーを初期化する
    SetGameWindowHandle(hWnd);
    SetInGameCursorEnabled(true);

    InitGameWorld();
    sceneManager.Init(TITLE); // タイトル画面から開始する

    MSG msg;

    // ゲームループ
    while (1)
    {
        // 新しいメッセージがある場合
        if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
        {
            // ウィンドウプロシージャへメッセージを送る
            DispatchMessage(&msg);

            // WM_QUIT を受け取ったらループを抜ける
            if (msg.message == WM_QUIT) {
                break;
            }
        }
        else
        {
            sceneManager.GameLoop();  // 入力処理とゲーム更新を行う
        }
    }

    CleanUpGameWorld();
    // DirectX リソースを解放する
    RendererUninit();

    UnregisterClass(CLASS_NAME, hInstance);

    return (int)msg.wParam;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
    case WM_CREATE:
        break;

    case WM_SETFOCUS:
        break;

    case WM_KILLFOCUS:
        break;

    case WM_SETCURSOR:
        // クライアント領域でカーソルを強制的に NULL にしない。
        // カーソル表示はシーンに応じて ShowCursor(TRUE/FALSE) で制御する。
        break;

    case WM_SIZE: {
        // ウィンドウサイズ変更時にグローバル変数とカメラを更新する
        g_windowWidth = LOWORD(lParam);
        g_windowHeight = HIWORD(lParam);
        g_camera.SetWindowSize(g_windowWidth, g_windowHeight);

        break;
    }

    case WM_DESTROY:

        CleanUpGameWorld();
        SetInGameCursorEnabled(false);
        PostQuitMessage(0);
        break;

    case WM_CLOSE: {

        CleanUpGameWorld();
        SetInGameCursorEnabled(false);
        int res = MessageBoxA(NULL, "Are you sure you want to exit?", "Confirmation", MB_OKCANCEL);
        if (res == IDOK) {
            DestroyWindow(hWnd);
        }
        break;
    }

   /* case WM_KEYDOWN:
        if (LOWORD(wParam) == VK_ESCAPE) {
            PostMessage(hWnd, WM_CLOSE, wParam, lParam);
        }
        break;*/

    default:
        return DefWindowProc(hWnd, uMsg, wParam, lParam);
    }

    return 0;
}