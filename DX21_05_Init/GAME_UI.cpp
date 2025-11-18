#include <windows.h>
#include <string>
#include <vector>
#include <functional>

// 窗口常量
const int WIDTH = 800;
const int HEIGHT = 600;
const wchar_t* WINDOW_CLASS = L"GameUIClass";
const wchar_t* WINDOW_TITLE = L"纯C++游戏UI示例";

// 颜色定义
const COLORREF COLOR_BG = RGB(30, 50, 80);       // 背景色（深蓝）
const COLORREF COLOR_BUTTON_NORMAL = RGB(100, 100, 100); // 按钮默认色
const COLORREF COLOR_BUTTON_HOVER = RGB(150, 150, 150);  // 按钮悬停色
const COLORREF COLOR_BUTTON_TEXT = RGB(255, 255, 255);   // 按钮文字色
const COLORREF COLOR_TITLE = RGB(255, 255, 0);           // 标题文字色

// UI元素基类
class UIElement {
protected:
    RECT rect;       // 位置和大小（left, top, right, bottom）
    bool visible;    // 是否可见
    HWND hWnd;       // 关联的窗口句柄

public:
    UIElement(HWND parentWnd, int x, int y, int w, int h)
        : hWnd(parentWnd), visible(true) {
        rect.left = x;
        rect.top = y;
        rect.right = x + w;
        rect.bottom = y + h;
    }

    virtual ~UIElement() = default;

    // 绘制元素
    virtual void Draw(HDC hdc) = 0;
    // 处理鼠标点击
    virtual void OnClick(int x, int y) {}
    // 处理鼠标移动（用于悬停效果）
    virtual void OnMouseMove(int x, int y) {}

    // 检查点是否在元素内
    bool IsInside(int x, int y) const {
        return (x >= rect.left && x <= rect.right &&
            y >= rect.top && y <= rect.bottom);
    }

    void SetVisible(bool v) { visible = v; }
    bool IsVisible() const { return visible; }
};

// 按钮类
class Button : public UIElement {
private:
    std::wstring text;
    bool hovered;                     // 是否悬停
    std::function<void()> callback;   // 点击回调

public:
    Button(HWND parentWnd, int x, int y, int w, int h, const std::wstring& txt)
        : UIElement(parentWnd, x, y, w, h), text(txt), hovered(false) {
    }

    // 设置点击回调
    void SetCallback(std::function<void()> cb) {
        callback = cb;
    }

    // 处理鼠标移动（更新悬停状态）
    void OnMouseMove(int x, int y) override {
        bool newHover = IsInside(x, y);
        if (newHover != hovered) {
            hovered = newHover;
            // 重绘按钮（刷新悬停效果）
            InvalidateRect(hWnd, &rect, FALSE);
        }
    }

    // 处理点击
    void OnClick(int x, int y) override {
        if (IsInside(x, y) && callback) {
            callback();
        }
    }

    // 绘制按钮
    void Draw(HDC hdc) override {
        if (!visible) return;

        // 1. 绘制按钮背景（根据悬停状态切换颜色）
        HBRUSH brush = CreateSolidBrush(hovered ? COLOR_BUTTON_HOVER : COLOR_BUTTON_NORMAL);
        FillRect(hdc, &rect, brush);
        DeleteObject(brush);

        // 2. 绘制按钮边框
        HPEN pen = CreatePen(PS_SOLID, 2, RGB(0, 0, 0));
        HPEN oldPen = (HPEN)SelectObject(hdc, pen);
        Rectangle(hdc, rect.left, rect.top, rect.right, rect.bottom);
        SelectObject(hdc, oldPen);
        DeleteObject(pen);

        // 3. 绘制按钮文字（居中）
        SetTextColor(hdc, COLOR_BUTTON_TEXT);
        SetBkMode(hdc, TRANSPARENT);  // 文字背景透明
        DrawText(hdc, text.c_str(), -1, &rect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
    }
};

// 文本元素类
class TextElement : public UIElement {
private:
    std::wstring text;
    COLORREF color;

public:
    TextElement(HWND parentWnd, int x, int y, int w, int h, const std::wstring& txt, COLORREF clr)
        : UIElement(parentWnd, x, y, w, h), text(txt), color(clr) {
    }

    void SetText(const std::wstring& txt) {
        text = txt;
        InvalidateRect(hWnd, &rect, FALSE); // 重绘
    }

    void Draw(HDC hdc) override {
        if (!visible) return;

        SetTextColor(hdc, color);
        SetBkMode(hdc, TRANSPARENT);
        // 使用基础文本格式，避免依赖DT_WORDELLIPSIS
        DrawText(hdc, text.c_str(), -1, &rect, DT_LEFT | DT_TOP | DT_SINGLELINE);
    }
};

// UI管理器（管理所有UI元素）
class UIManager {
private:
    std::vector<UIElement*> elements;

public:
    ~UIManager() {
        for (auto elem : elements) delete elem;
    }

    void Add(UIElement* elem) {
        elements.push_back(elem);
    }

    // 绘制所有可见元素
    void DrawAll(HDC hdc) {
        for (auto elem : elements) {
            if (elem->IsVisible()) {
                elem->Draw(hdc);
            }
        }
    }

    // 分发鼠标移动事件
    void OnMouseMove(int x, int y) {
        for (auto elem : elements) {
            elem->OnMouseMove(x, y);
        }
    }

    // 分发鼠标点击事件
    void OnMouseClick(int x, int y) {
        for (auto elem : elements) {
            elem->OnClick(x, y);
        }
    }
};

// 全局对象
UIManager* uiManager = nullptr;
bool isRunning = true;
TextElement* statusText = nullptr;

// 按钮点击回调
void OnStartGame() {
    if (statusText) {
        statusText->SetText(L"游戏已开始！按ESC退出");
    }
}

void OnQuitGame() {
    isRunning = false;
    PostQuitMessage(0);
}

// 窗口过程（处理系统消息）
LRESULT CALLBACK WindowProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
    case WM_PAINT: {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hWnd, &ps);

        // 绘制背景
        HBRUSH bgBrush = CreateSolidBrush(COLOR_BG);
        FillRect(hdc, &ps.rcPaint, bgBrush);
        DeleteObject(bgBrush);

        // 绘制所有UI元素
        if (uiManager) uiManager->DrawAll(hdc);

        EndPaint(hWnd, &ps);
        return 0;
    }

    case WM_MOUSEMOVE: {
        int x = LOWORD(lParam);
        int y = HIWORD(lParam);
        if (uiManager) uiManager->OnMouseMove(x, y);
        return 0;
    }

    case WM_LBUTTONDOWN: {
        int x = LOWORD(lParam);
        int y = HIWORD(lParam);
        if (uiManager) uiManager->OnMouseClick(x, y);
        return 0;
    }

    case WM_KEYDOWN: {
        if (wParam == VK_ESCAPE) { // ESC键退出
            isRunning = false;
            PostQuitMessage(0);
        }
        return 0;
    }

    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    }

    return DefWindowProc(hWnd, msg, wParam, lParam);
}

// 注册窗口类
void RegisterWindowClass(HINSTANCE hInstance) {
    WNDCLASSEX wc = { 0 };
    wc.cbSize = sizeof(WNDCLASSEX);
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = WINDOW_CLASS;
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wc.style = CS_HREDRAW | CS_VREDRAW; // 窗口大小变化时重绘
    RegisterClassEx(&wc);
}

// 创建窗口
HWND CreateGameWindow(HINSTANCE hInstance) {
    return CreateWindowEx(
        0,
        WINDOW_CLASS,
        WINDOW_TITLE,
        WS_OVERLAPPEDWINDOW & ~WS_THICKFRAME & ~WS_MAXIMIZEBOX, // 禁止调整大小和最大化
        CW_USEDEFAULT, CW_USEDEFAULT, WIDTH, HEIGHT,
        NULL, NULL, hInstance, NULL
    );
}

// 初始化UI元素
void InitUI(HWND hWnd) {
    uiManager = new UIManager();

    // 添加标题文本
    uiManager->Add(new TextElement(hWnd, 250, 50, 300, 50, L"纯C++游戏UI演示", COLOR_TITLE));

    // 添加开始按钮
    Button* startBtn = new Button(hWnd, 300, 200, 200, 50, L"开始游戏");
    startBtn->SetCallback(OnStartGame);
    uiManager->Add(startBtn);

    // 添加退出按钮
    Button* quitBtn = new Button(hWnd, 300, 300, 200, 50, L"退出游戏");
    quitBtn->SetCallback(OnQuitGame);
    uiManager->Add(quitBtn);

    // 添加状态文本
    statusText = new TextElement(hWnd, 250, 500, 300, 30, L"请点击开始游戏", RGB(0, 255, 0));
    uiManager->Add(statusText);
}

// 主函数
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    RegisterWindowClass(hInstance);
    HWND hWnd = CreateGameWindow(hInstance);
    if (!hWnd) return 0;

    InitUI(hWnd);
    ShowWindow(hWnd, nCmdShow);

    // 消息循环
    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0) > 0) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    // 清理资源
    delete uiManager;
    return (int)msg.wParam;
}