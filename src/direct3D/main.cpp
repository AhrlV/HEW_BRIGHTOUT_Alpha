/*============================================================================================================

　　ウィンドウ表示 [main.cpp]
                                                                                Author : Ryosuke Kageyama
                                                                                Date   : 2025/5/7

=============================================================================================================*/
#include <SDKDDKVer.h>
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <algorithm>

#include "direct3D/direct3d.h"
#include "direct3D/shader.h"
#include "direct3D/texture.h"
#include "direct3D/sprite.h"

#include "direct3D/shader3d.h"
#include "direct3D/game.h"

#include "direct3D/system_timer.h"
#include "direct3D/debug_text.h"
#include <sstream>

#include "direct3D/key_logger.h"
#include "direct3D/mouse.h"

#include "lifecycle.h"


using namespace DirectX;


/*============================================================================================================
　　ウィンドウ情報
=============================================================================================================*/
static constexpr char WINDOW_CLASS[] = "GameWindow"; // メインウィンドウのクラス名
static constexpr char TITLE[] = "ゲームウィンドウ";  // タイトルバーのテキスト


/*============================================================================================================
　　ウィンドウプロシージャ　プロトタイプ宣言
=============================================================================================================*/
LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);



/*============================================================================================================
　　メイン
=============================================================================================================*/
int APIENTRY WinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE /*hPrevInstance*/,
    _In_ LPSTR /*lpCmdLine*/, _In_ int nCmdShow)
{
    int hoge = CoInitializeEx(nullptr, COINITBASE_MULTITHREADED);

    SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);


    hoge = NULL;

    //ウィンドウクラスの登録
    WNDCLASSEX wcex{};

    wcex.cbSize = sizeof(WNDCLASSEX);                         //構造体のサイズ
    wcex.lpfnWndProc = WndProc;                               //ウィンドウプロシージャのポインター
    wcex.hInstance = hInstance;
    wcex.hIcon = LoadIcon(hInstance, IDI_APPLICATION);
    wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wcex.lpszMenuName = nullptr; //メニューは作らない
    wcex.lpszClassName = WINDOW_CLASS;
    wcex.hIconSm = LoadIcon(wcex.hInstance, IDI_APPLICATION);

    RegisterClassEx(&wcex);

    //　クライアント領域のサイズを持った矩形（左からleft, top, right, bottom, ）
    RECT window_rect = { 0, 0, 1600, 900 };

    //　ウィンドウのスタイル
    DWORD window_style = WS_OVERLAPPEDWINDOW ^ (WS_THICKFRAME | WS_MAXIMIZEBOX);

    //　指定したクライアント領域を確保するために新たな矩形座標を計算
    AdjustWindowRect(&window_rect, window_style, FALSE);

    //ウィンドウの幅と高さを算出
    int window_width = window_rect.right - window_rect.left;
    int window_height = window_rect.bottom - window_rect.top;

    //プライマリモニターの画面解像取得
    int desktop_width = GetSystemMetrics(SM_CXSCREEN);
    int desktop_height = GetSystemMetrics(SM_CYSCREEN);



    // デスクトップの真ん中にウィンドウが生成されるように座標を計算
    // ※ただし万が一、デスクトップよりウィンドウが大きい場合は左上に表示
    int window_x = (std::max)((desktop_width - window_width) / 2, 0); // 修正: std::max を括弧で囲む
    int window_y = (std::max)((desktop_height - window_height) / 2, 0); // 修正: std::max を括弧で囲む

    //メインウィンドウの作成
    HWND hWnd = CreateWindow(
        WINDOW_CLASS,          //クラス名
        TITLE,                 //タイトル
        window_style, //ビット演算NAND
        window_x,         //適当にウィンドウの位置を決める
        window_y,
        window_width,         //大きさ
        window_height,
        nullptr,
        nullptr,
        hInstance,
        nullptr);

    ShowWindow(hWnd, nCmdShow);
    UpdateWindow(hWnd);



    /*各種初期化*/
    SystemTimer_Initialize();
    KeyLogger_Initialize();
    Mouse_Initialize(hWnd);

    if (!Direct3D_Init(hWnd)) //Direct3Dの初期化
    {
        MessageBox(nullptr, "Direct3Dの初期化に失敗しました", "エラー", MB_OK);
    }
    else
    {
        if (!Shader_Initialize(Direct3D_GetDevice(), Direct3D_GetDeviceContext()))
        {
            MessageBox(nullptr, "シェーダーの初期化に失敗しました", "エラー", MB_OK);
        }
        else
        {
            if (!Sprite_Initialize(Direct3D_GetDevice(), Direct3D_GetDeviceContext()))
            {
				MessageBox(nullptr, "ポリゴンの初期化に失敗しました", "エラー", MB_OK);
            }
        }
        Texture_Initialize(Direct3D_GetDevice(), Direct3D_GetDeviceContext());
    }

    Shader3d_Initialize(Direct3D_GetDevice(), Direct3D_GetDeviceContext());

    hal::DebugText dt(Direct3D_GetDevice(), Direct3D_GetDeviceContext(),
        L"resources/texture/consolab_ascii_512.png",
        Direct3D_GetBackBufferWidth(), Direct3D_GetBackBufferHeight(),
        0.0f, 0.0f,
        0, 0,
        0.0f, 0.0f
    );



    Mouse_SetVisible(false);
    Game_Init();


    //時間計測用
    double exec_last_time = 0.0;
    double fps_last_time = 0.0;
    double current_time = 0.0;
    ULONG frame_count = 0;

    double fps = 0;
    
    exec_last_time = fps_last_time = SystemTimer_GetTime();

    //メッセージループ
    MSG msg;

    do
    {
        
        if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) //ウィンドウメッセージが来ていたら
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
        else //ゲームの処理
        {

            current_time = SystemTimer_GetTime();
            double elapsed_time = current_time - fps_last_time;


            if (elapsed_time >= 1.0) {

                fps = frame_count / elapsed_time;
                fps_last_time = current_time;
                frame_count = 0;
            }

            
            if ((current_time - exec_last_time) >= (1.0f / 60.0f)) {

                KeyLogger_Update();


                Direct3D_Clear();
                SetViewPort(0);

                Tick();

                Game_Update();
                Game_Draw();


#if defined(DEBUG) || defined(_DEBUG)
                std::stringstream ss;
                ss << "fps:" << fps << std::endl;
                dt.SetText(ss.str().c_str());

                dt.Draw();
                dt.Clear();
#endif


                Direct3D_Present();

                exec_last_time = current_time;
                frame_count++;
            }
            else {
                Sleep(0);
            }
        }

    } while (msg.message != WM_QUIT);


    Sprite_Finalize();
    Texture_Finalize();
    Shader_Finalize();
    Shader3d_Finalize();
    Game_UnInit();
    Direct3D_Uninit(); //Direct3Dの終了処理
    Mouse_Finalize();
}


/*============================================================================================================
　　ウィンドウプロシージャ
=============================================================================================================*/
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_ACTIVATEAPP:
        Mouse_ProcessMessage(message, wParam, lParam);
        Keyboard_ProcessMessage(message, wParam, lParam);
        break;
    case WM_INPUT:
    case WM_MOUSEMOVE:
    case WM_LBUTTONDOWN:
    case WM_LBUTTONUP:
    case WM_RBUTTONDOWN:
    case WM_RBUTTONUP:
    case WM_MBUTTONDOWN:
    case WM_MBUTTONUP:
    case WM_MOUSEWHEEL:
    case WM_XBUTTONDOWN:
    case WM_XBUTTONUP:
    case WM_MOUSEHOVER:
        Mouse_ProcessMessage(message, wParam, lParam);
        break;
    case WM_KEYDOWN:
    case WM_SYSKEYDOWN:
        if (wParam == VK_ESCAPE) {
            SendMessage(hWnd, WM_CLOSE, 0, 0); //WM_CLOSEメッセージの送信
        }
    case WM_KEYUP:
    case WM_SYSKEYUP:
        Keyboard_ProcessMessage(message, wParam, lParam);
        break;

    case WM_CLOSE: //　ウィンドウを閉じるメッセージ
        if (MessageBox(hWnd, "本当に終了してよろしいですか？",
            "確認", MB_OKCANCEL | MB_DEFBUTTON2) == IDOK) {
            DestroyWindow(hWnd); //指定のウィンドウにWM_DESTROYメッセージを送る
        }
        break; //DefWindowProc関数にメッセージを流さず終了することによって何もなかったことにする

    case WM_DESTROY: //　ウィンドウの破棄メッセージ
        PostQuitMessage(0); // WM_QUITメッセージの送信
        break;
    default:
        //　通常のメッセージ処理はこの関数に任せる
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}
