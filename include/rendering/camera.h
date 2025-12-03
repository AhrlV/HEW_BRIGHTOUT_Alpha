/*============================================================================================================

    カメラコンポーネント [camera.h]
    シーン内のカメラを表すコンポーネント。
    ビュー行列と射影行列を計算し、頂点シェーダーのPerFrame定数バッファを更新する。

    Author : Ryosuke Kageyama
    Date   : 2025/11/26

=============================================================================================================*/
#ifndef CAMERA_H
#define CAMERA_H

#include <DirectXMath.h>
#include "lifecycle/component.h"
#include "lifecycle/gameobject.h"
#include "physics/transform.h"

// 前方宣言
class VertexShader;

/*============================================================================================================
    カメラコンポーネントクラス
    シーン内のカメラを表現し、ビュー行列と射影行列を計算する。
=============================================================================================================*/
class Camera : public Component
{
public:
	/*========================================================================================================
		カメラパラメータ
	========================================================================================================*/
	
	// カメラの前方向ベクトル
	DirectX::XMFLOAT3 Forward{0,0,1};
	
	// カメラの上方向ベクトル
	DirectX::XMFLOAT3 Up{0,1,0};
	
	// カメラの右方向ベクトル
	DirectX::XMFLOAT3 Right{1,0,0};
	
	// 視野角（Y軸方向、ラジアン）
	float FovY = DirectX::XM_PI * 0.5f;
	
	// アスペクト比（幅/高さ）
	float Aspect = 16.f/9.f;
	
	// 近クリップ面までの距離
	float NearZ = 0.1f;
	
	// 遠クリップ面までの距離
	float FarZ = 1000.f;

	/*========================================================================================================
		行列計算
	========================================================================================================*/
	
	// ビュー行列を取得する
	// TransFormコンポーネントの位置と回転を使用してビュー行列を計算する。
	// 戻り値: ビュー行列
	DirectX::XMMATRIX GetViewMatrix() const
	{
		using namespace DirectX;

		auto tf = m_Owner->GetComponent<TransForm>();

		XMFLOAT3 posF = tf->Position;
		XMFLOAT3 rotF = tf->RotationEuler;

		XMMATRIX R = XMMatrixRotationRollPitchYaw(rotF.x, rotF.y, rotF.z);
		XMVECTOR f = XMVector3Normalize(XMVector3TransformNormal(XMVectorSet(0,0,1,0), R));
		XMVECTOR u = XMVector3Normalize(XMVector3TransformNormal(XMVectorSet(0,1,0,0), R));
		XMVECTOR pos = XMLoadFloat3(&posF);
		return XMMatrixLookAtLH(pos, pos+f, u);
	}
	
	// 射影行列を取得する
	// カメラパラメータを使用して射影行列を計算する。
	// 戻り値: 射影行列
	DirectX::XMMATRIX GetProjMatrix() const
	{
		return DirectX::XMMatrixPerspectiveFovLH(FovY, Aspect, NearZ, FarZ);
	}
};

#endif
