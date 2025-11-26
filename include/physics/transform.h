/*====================================================================

	Transform / Math primitives [transform.h]

	Author : Ryosuke Kageyama
	Date   : 2025/11/19
====================================================================*/


#ifndef TRANSFORM_H
#define TRANSFORM_H

#include <DirectXMath.h>
#include "lifecycle/component.h"

class TransForm : public Component
{
public:
	DirectX::XMFLOAT3 Position{0,0,0};
	DirectX::XMFLOAT3 RotationEuler{0,0,0}; // radians
	DirectX::XMFLOAT3 Scale{1,1,1};

	DirectX::XMMATRIX GetWorldMatrix() const
	{
		DirectX::XMMATRIX S = DirectX::XMMatrixScaling(Scale.x,Scale.y,Scale.z);
		DirectX::XMMATRIX R = DirectX::XMMatrixRotationRollPitchYaw(RotationEuler.x,RotationEuler.y,RotationEuler.z);
		DirectX::XMMATRIX T = DirectX::XMMatrixTranslation(Position.x,Position.y,Position.z);
		return S*R*T;
	}
};

#endif // TRANSFORM_H
