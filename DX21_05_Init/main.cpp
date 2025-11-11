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
    // ウィンドウクラス情報をまとめる
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

    // 获取屏幕分辨率
    int screenWidth = GetSystemMetrics(SM_CXSCREEN);
    int screenHeight = GetSystemMetrics(SM_CYSCREEN);

    // 创建无边框窗口覆盖整个屏幕
    HWND hWnd;

    hWnd = CreateWindowEx(0,
        CLASS_NAME,
        WINDOW_NAME,
        WS_POPUP | WS_VISIBLE,  // 无边框样式并立即可见
        0, 0,                   // 位置在(0,0)
        screenWidth,            // 宽度覆盖整个屏幕
        screenHeight,           // 高度覆盖整个屏幕
        NULL,
        NULL,
        hInstance,
        NULL);
    // ゲームループに入る前にDirectXの初期化をする
    RendererInit(hWnd);
    InitGameWorld();

    MSG msg;

    // ゲームループ
    while (1)
    {
        // 新たにメッセージがあれば
        if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
        {
            // ウィンドウプロシージャにメッセージを送る
            DispatchMessage(&msg);

            // 「WM_QUIT」メッセージを受け取ったらループを抜ける
            if (msg.message == WM_QUIT) {
                break;
            }
        }
        else
        {
            GameLoop();  // 处理键盘输入
        }
    }

    // DirectXの解放処理
    RendererUninit();

    UnregisterClass(CLASS_NAME, hInstance);

    return (int)msg.wParam;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
    case WM_DESTROY:    // ウィンドウ破棄のメッセージ
        PostQuitMessage(0); // 「WM_QUIT」メッセージを送る → アプリ終了
        break;

    case WM_CLOSE:      // 「x」ボタンが押されたら
    {
        int res = MessageBoxA(NULL, "終了しますか？", "確認", MB_OKCANCEL);
        if (res == IDOK) {
            DestroyWindow(hWnd);  // 「WM_DESTROY」メッセージを送る
        }
    }
    break;

    case WM_KEYDOWN: // キー入力があったメッセージ
        if (LOWORD(wParam) == VK_ESCAPE)
        { // 入力されたキーがESCAPEなら
            PostMessage(hWnd, WM_CLOSE, wParam, lParam); // 「WM_CLOSE」を送る
        }
        break;

    default:
        // 受け取ったメッセージに対してデフォルトの処理を実行
        return DefWindowProc(hWnd, uMsg, wParam, lParam);
        break;
    }

    return 0;
}