

#ifndef CAMERA_H
#define CAMERA_H

#include <DirectXMath.h>
#include "lifecycle/component.h"
#include "lifecycle/gameobject.h"
#include "physics/transform.h"

class Camera : public Component
{
public:
	DirectX::XMFLOAT3 Forward{0,0,1};
	DirectX::XMFLOAT3 Up{0,1,0};
	DirectX::XMFLOAT3 Right{1,0,0};
	float FovY = DirectX::XM_PI * 0.5f;
	float Aspect = 16.f/9.f;
	float NearZ = 0.1f;
	float FarZ = 1000.f;


	// 行列計算時に Transform の位置と回転を使用 (内部状態は更新しないため const)
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
	DirectX::XMMATRIX GetProjMatrix() const { return DirectX::XMMatrixPerspectiveFovLH(FovY, Aspect, NearZ, FarZ); }
};

#endif
