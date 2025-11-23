
/*=======================================================================================================


     Cube‚Ì•`‰æ[cube.cpp]

                                                                              Author :  Ryosuke Kageyama
                                                                              Date   :  2025/10/16
========================================================================================================*/

#ifndef CUBE_H
#define CUBE_H

#include<d3d11.h>
#include <DirectXMath.h>

void Cube_Initialize(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
void Cube_Finalize();
void Cube_Draw(const DirectX::XMMATRIX& mtxWorld, int tex_id);


struct TransForm3D
{
    float m_posX;
    float m_posY;
    float m_posZ;

    float m_rotX;
    float m_rotY;
    float m_rotZ;

    float m_scaleX;
    float m_scaleY;
    float m_scaleZ;

    TransForm3D()
        : m_posX(0.0f), m_posY(0.0f), m_posZ(0.0f), m_rotX(0.0f), m_rotY(0.0f), m_rotZ(0.0f), m_scaleX(0.0f), m_scaleY(0.0f), m_scaleZ(0.0f)
    {}

};


#endif

