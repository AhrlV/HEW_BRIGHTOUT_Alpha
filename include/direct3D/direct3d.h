/*============================================================================================================

 direct3d.h

=============================================================================================================*/

#ifndef DIRECT3D_H

#define DIRECT3D_H

#include <Windows.h>
#include <d3d11.h>


//セーフリリースマクロ
#define SAFE_RELEASE(o) if (o) { (o)->Release(); o = NULL; }


bool Direct3D_Init(HWND hWnd);
void Direct3D_Uninit();

ID3D11Device* Direct3D_GetDevice();
ID3D11DeviceContext* Direct3D_GetDeviceContext();


void Direct3D_Clear(); //バックバッファのクリア
void Direct3D_Present(); //バックバッファの表示

unsigned int Direct3D_GetBackBufferWidth();
unsigned int Direct3D_GetBackBufferHeight();

void SetViewPort(int n);


enum AlphaBlendMode
{
	BLEND_TRANSPARENT,
	BLEND_ADD,
	BLEND_MAX
};

void Direct3D_SetAlphaBlend(AlphaBlendMode blend);


void Direct3D_SetDepthTest(bool bEnable);



#endif
