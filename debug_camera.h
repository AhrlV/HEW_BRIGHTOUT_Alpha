/*=======================================================================================================


     デバッグ用カメラ [debug_camera.h]

                                                                              Author :  Ryosuke Kageyama
                                                                              Date   :  2025/10/28
========================================================================================================*/


#ifndef DEBUG_CAMERA_H
#define DEBUG_CAMERA_H

#include <directXMath.h>
#include "camera.h"


class DebugCamera : public Camera
{
private:
    DirectX::XMFLOAT3 m_front{};
    DirectX::XMFLOAT3 m_right{};
    DirectX::XMFLOAT3 m_up{};
    DirectX::XMFLOAT3 m_postision{};

public:
    DebugCamera(const DirectX::XMFLOAT3& position, const DirectX::XMFLOAT3& target);

    void HandleInput() override; // キー入力待ち

    virtual void SetMatrix() const override;
};


#endif