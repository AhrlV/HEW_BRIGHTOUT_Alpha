/*============================================================================================================

    ウィンドウ・テスト描画用メイン [main.cpp]
    GraphicsDevice / RenderTargetManager / Mesh / MeshRenderer / Material / Camera
    を用いて単純な板面の描画を描画するサンプル。

    Author : Ryosuke Kageyama
    Date   : 2025/11/25

=============================================================================================================*/
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <algorithm>

#include "scene/kageyama.h"

// Direct3D 関連
#include "direct3d.h"
#include "resource_management.h"

#include "lifecycle/scene_manager.h"
#include "lifecycle/gameloop.h"


/*============================================================================================================
    ウィンドウ定義
=============================================================================================================*/
static constexpr char WINDOW_CLASS[] = "GameWindow";
static constexpr char TITLE[] = "テスト描画";

/*============================================================================================================
    ウィンドウプロシージャ宣言
=============================================================================================================*/
LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

/*============================================================================================================
    WinMain
    ウィンドウ作成、D3D初期化、板面の描画のテストループを構築する。
=============================================================================================================*/
int APIENTRY WinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE /*hPrevInstance*/, _In_ LPSTR /*lpCmdLine*/, _In_ int nCmdShow)
{
    // COM 初期化
    if(FAILED(CoInitializeEx(nullptr, COINITBASE_MULTITHREADED)))
    {
        return 0;
    }

    // DPI 対応
    SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);


    // ウィンドウクラス登録
    WNDCLASSEX wcex{};
    wcex.cbSize = sizeof(WNDCLASSEX);
    wcex.lpfnWndProc = WndProc;
    wcex.hInstance = hInstance;
    wcex.hIcon = LoadIcon(hInstance, IDI_APPLICATION);
    wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wcex.lpszMenuName = nullptr;
    wcex.lpszClassName = WINDOW_CLASS;
    wcex.hIconSm = LoadIcon(wcex.hInstance, IDI_APPLICATION);
    RegisterClassEx(&wcex);

    // クライアント領域サイズ
    RECT window_rect = { 0, 0, 1920, 1080 };
    DWORD window_style = WS_OVERLAPPEDWINDOW ^ (WS_THICKFRAME | WS_MAXIMIZEBOX);
    AdjustWindowRect(&window_rect, window_style, FALSE);
    int window_width = window_rect.right - window_rect.left;
    int window_height = window_rect.bottom - window_rect.top;

    // 画面中央に配置
    int desktop_width = GetSystemMetrics(SM_CXSCREEN);
    int desktop_height = GetSystemMetrics(SM_CYSCREEN);
    int window_x = std::max((desktop_width - window_width) / 2, 0);
    int window_y = std::max((desktop_height - window_height) / 2, 0);

    // ウィンドウ作成
    HWND hWnd = CreateWindow(WINDOW_CLASS, TITLE, window_style, window_x, window_y, window_width, window_height, nullptr, nullptr, hInstance, nullptr);
    ShowWindow(hWnd, nCmdShow);
    UpdateWindow(hWnd);

    
    // Direct3D 初期化
    auto& device = GraphicsDevice::Instance();
    device.Initialize(hWnd);
    auto& rtm = RenderTargetManager::Instance();
    rtm.Initialize(device.GetDevice(), device.GetSwapChain());


    // SceneManagerシングルトンを取得してSceneを設定・初期化
    auto& sceneMgr = SceneManager::Instance();
    sceneMgr.ResourceLoadScene();
    sceneMgr.InitializeScene();


    // ゲームループの取得
    auto& gameLoop = GameLoop::Instance();


    // メインループ
    MSG msg;
    BOOL running = TRUE;
    while (running)
    {
        // Windowsメッセージ処理
        while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
        {
            if (msg.message == WM_QUIT)
            {
                running = FALSE;
                break;
            }
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }

        // ゲームループのティック処理
        // 物理更新、Awake、Start、Update、LateUpdate、破棄、描画を全て実行
        try
        {
            gameLoop.Tick();
        }
        catch (const std::runtime_error& e)
        {
            MessageBoxA(nullptr, e.what(), "エラー", MB_OK | MB_ICONERROR);
            break;
        }
    }

    return 0;
}

/*============================================================================================================
    WndProc
    入力処理と終了処理のみの簡易実装。
=============================================================================================================*/
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch(message)
    {
    case WM_KEYDOWN:
        if(wParam == VK_ESCAPE)
        {
            PostQuitMessage(0);
        }
        break;
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}
