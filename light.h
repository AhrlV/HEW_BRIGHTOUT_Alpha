/*=======================================================================================================


     ÉâÉCÉgÇÃêßå‰[light.h]

                                                                              Author :  Ryosuke Kageyama
                                                                              Date   :  2025/11/06
========================================================================================================*/


#ifndef LIGHT_H
#define LIGHT_H

#include <DirectXMath.h>

void Light_Init();
void Light_UnInit();

void Light_SetAmbient(const DirectX::XMFLOAT4& color);

struct Diffuse
{
    DirectX::XMFLOAT4 color;
    DirectX::XMFLOAT4 direction;
};

void Light_SetDiffuse(const DirectX::XMFLOAT4& color, const DirectX::XMFLOAT3& direction);
void Light_SetDiffuse(const Diffuse& light);

#endif
