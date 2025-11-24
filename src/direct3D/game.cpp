
/*=======================================================================================================


     GameÇÃèàóù[game.cpp]

                                                                              Author :  Ryosuke Kageyama
                                                                              Date   :  2025/10/23
========================================================================================================*/
#include "direct3D/game.h"
#include "direct3D/camera.h"
#include "direct3D/direct3d.h"
#include "direct3D/grid.h"
#include "direct3D/cube.h"
#include "direct3D/light.h"

#include "direct3D/debug_camera.h"
#include "direct3D/model.h"

using namespace DirectX;

static DebugCamera g_camera({ 0.0f, 1.0f, -5.0f }, { 0.0f, 0.0f, 0.0f });

static MODEL* g_pKirby = nullptr;

void Game_Init()
{
    Cube_Initialize(Direct3D_GetDevice(), Direct3D_GetDeviceContext());
    Grid_Init(Direct3D_GetDevice(), Direct3D_GetDeviceContext(), 10, 10, 1.0f, 1.0f);
    Light_Init();

    g_pKirby = ModelLoad("resources/model/kirby.fbx", 0.2f);
}

void Game_UnInit()
{
    ModelRelease(g_pKirby);

    Cube_Finalize();
    Grid_UnInit();
    Light_UnInit();
}

void Game_Update()
{
    g_camera.HandleInput();
}

void Game_Draw()
{
    g_camera.SetMatrix();

    Direct3D_SetDepthTest(true);

    Grid_Draw();

    Light_SetAmbient({0.2f, 0.1f, 0.1f, 1.0f});

    XMFLOAT3 direction;
    XMStoreFloat3(&direction, { 0.5f, -1.0f, 1.0f });
    Light_SetDiffuse({ 1.0f, 1.0f, 1.0f, 1.0f }, direction);

    ModelDraw(g_pKirby, XMMatrixIdentity());


    Direct3D_SetDepthTest(false);
}
