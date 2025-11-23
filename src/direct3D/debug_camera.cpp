/*=======================================================================================================


     デバッグカメラ[debug_camera.cpp]

                                                                              Author :  Ryosuke Kageyama
                                                                              Date   :  2025/10/28
========================================================================================================*/

#include "direct3D/debug_camera.h"
#include "direct3D/key_logger.h"
#include "direct3D/direct3d.h"
#include "direct3D/shader3d.h"

using namespace DirectX;

DebugCamera::DebugCamera(const DirectX::XMFLOAT3& position, const DirectX::XMFLOAT3& target)
    : m_postision(position), m_up({ 0.0,1.0,0.0 })
{
    // 前方向はどっち
    XMVECTOR xtarget = XMLoadFloat3(&target);
    XMVECTOR xposition = XMLoadFloat3(&position);
    XMVECTOR xfront = XMVector3Normalize(xtarget - xposition);

    XMStoreFloat3(&m_front, xfront);

    // 適当に右方向ベクトルを作ってみる
    XMVECTOR xright = XMVector3Cross({ 0.0, 1.0, 0.0 }, xfront);
    xright = XMVector3Normalize(xright);
    XMStoreFloat3(&m_right, xright);
}

void DebugCamera::HandleInput()
{
    constexpr float MOVE_SPEED     = 0.2f;
    constexpr float ROTATION_SPEED = XMConvertToRadians(1.0f);
    
    XMVECTOR xfront = XMLoadFloat3(&m_front);
    XMVECTOR xright = XMLoadFloat3(&m_right);
    XMVECTOR xup =    XMLoadFloat3(&m_up);

    // 右回転
    if (KeyLogger_isPressed(KK_RIGHT))
    {
        XMMATRIX rotation = XMMatrixRotationAxis({0.0f, 1.0f, 0.0f}, ROTATION_SPEED);
        xfront = XMVector3TransformNormal(xfront, rotation);
        xright = XMVector3Cross(xup, xfront);

        xfront = XMVector3Normalize(xfront);
        xright = XMVector3Normalize(xright);
    }

    // 左回転
    if (KeyLogger_isPressed(KK_LEFT))
    {
        XMMATRIX rotation = XMMatrixRotationAxis({0.0f, 1.0f, 0.0f}, -ROTATION_SPEED);
        xfront = XMVector3TransformNormal(xfront, rotation);
        xright = XMVector3Cross(xup, xfront);

        xfront = XMVector3Normalize(xfront);
        xright = XMVector3Normalize(xright);
    }

    // 上回転
    if (KeyLogger_isPressed(KK_UP))
    {
        XMMATRIX rotation = XMMatrixRotationAxis(xright, -ROTATION_SPEED);
        xfront = XMVector3TransformNormal(xfront, rotation);
        xup = XMVector3Cross(xfront, xright);

        xfront = XMVector3Normalize(xfront);
        xup = XMVector3Normalize(xup);
    }

    // 下回転
    if (KeyLogger_isPressed(KK_DOWN))
    {
        XMMATRIX rotation = XMMatrixRotationAxis(xright, ROTATION_SPEED);
        xfront = XMVector3TransformNormal(xfront, rotation);
        xup = XMVector3Cross(xfront, xright);

        xfront = XMVector3Normalize(xfront);
        xup = XMVector3Normalize(xup);
    }

    // きりもみ右回転
    if (KeyLogger_isPressed(KK_E))
    {
        XMMATRIX rotation = XMMatrixRotationAxis(xfront, -ROTATION_SPEED);
        xright = XMVector3TransformNormal(xright, rotation);
        xup = XMVector3Cross(xfront, xright);

        xright = XMVector3Normalize(xright);
        xup = XMVector3Normalize(xup);
    }

    // きりもみ左回転
    if (KeyLogger_isPressed(KK_Q))
    {
        XMMATRIX rotation = XMMatrixRotationAxis(xfront, ROTATION_SPEED);
        xright = XMVector3TransformNormal(xright, rotation);
        xup = XMVector3Cross(xfront, xright);

        xright = XMVector3Normalize(xright);
        xup = XMVector3Normalize(xup);
    }



    XMStoreFloat3(&m_front, xfront);
    XMStoreFloat3(&m_right, xright);
    XMStoreFloat3(&m_up, xup);

    static XMVECTOR xposition = XMLoadFloat3(&m_postision);

    // 前
    if (KeyLogger_isPressed(KK_W))
    {
        xposition += XMVector3Normalize(XMVectorSetY(xfront, 0.0)) * MOVE_SPEED;
    }

    // 後
    if (KeyLogger_isPressed(KK_S))
    {
        XMVECTOR xback = xfront * (-1.0f);
        xposition += XMVector3Normalize(XMVectorSetY(xback, 0.0)) * MOVE_SPEED;
    }

    // 左
    if (KeyLogger_isPressed(KK_A))
    {
        XMVECTOR xleft = xright * (-1.0f);
        xposition = XMLoadFloat3(&m_postision) + xleft * MOVE_SPEED;   
    }

    // 右
    if (KeyLogger_isPressed(KK_D))
    {
        xposition = XMLoadFloat3(&m_postision) + xright * MOVE_SPEED;
    }

    // 上
    if (KeyLogger_isPressed(KK_SPACE))
    {
        const XMVECTOR cxup{ 0.0, 1.0, 0.0 };
        xposition = XMLoadFloat3(&m_postision) + cxup * MOVE_SPEED;
    }

    // 下
    if (KeyLogger_isPressed(KK_LEFTSHIFT))
    {
        const XMVECTOR cxdown{ 0.0, -1.0, 0.0 };
        xposition = XMLoadFloat3(&m_postision) + cxdown * MOVE_SPEED;
    }

    XMStoreFloat3(&m_postision, xposition);
}

void DebugCamera::SetMatrix() const
{
    // ビュー変換行列の設定
    XMVECTOR eye     = XMLoadFloat3(&m_postision);
    XMVECTOR target  = XMLoadFloat3(&m_postision) + XMLoadFloat3(&m_front);
    XMVECTOR up      = XMLoadFloat3(&m_up);
    XMMATRIX mtxView = XMMatrixLookAtLH(eye, target, up);
    Shader3d_SetViewMatrix(mtxView);



    // 頂点シェーダーにプロジェクション変換行列を設定
    // 視錐台
    float FovAngleY = 1.0f;
    float NearZ = 0.1f;
    float FarZ = 1000.0f;

    float width  = static_cast<float>(Direct3D_GetBackBufferWidth());
    float height = static_cast<float>(Direct3D_GetBackBufferHeight());

    XMMATRIX mtxProj = XMMatrixPerspectiveFovLH(FovAngleY, width / height, NearZ, FarZ);
    Shader3d_SetProjectionMatrix(mtxProj);
}
