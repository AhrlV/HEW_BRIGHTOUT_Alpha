#ifndef CAMERA_OBJ_H
#define CAMERA_OBJ_H

#include "lifecycle.h"
#include "rendering.h"

class CameraObj : public GameObject
{
public:
	/*====================================================================
		コンストラクタ
		Cameraを追加して設定する。

		引数:
		  size - 立方体のサイズ（デフォルト: 1.0f）
	====================================================================*/
	CameraObj(float Fov = DirectX::XM_PI * 0.5f, float  Aspect = 16.0f / 9.0f, float NearZ = 0.1f, float FarZ = 1000.f)
	{
		// MeshRendererコンポーネントを追加
		auto camera = AddComponent<Camera>();
		camera->FovY = Fov;
		camera->Aspect = Aspect;
		camera->NearZ = NearZ;
		camera->FarZ = FarZ;
	}

	virtual ~CameraObj() = default;
};

#endif