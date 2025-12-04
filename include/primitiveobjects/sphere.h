#ifndef SPHERE_H
#define SPHERE_H

#include "lifecycle.h"
#include "rendering.h"
#include "resource_management.h"

class Sphere : public GameObject
{
public:
	/*====================================================================
		コンストラクタ
		MeshRendererを追加し、Sphereプリミティブメッシュを作成して設定する。
		
		引数:
		  radius - 球の半径（デフォルト: 0.5f）
		  slices - 経度方向の分割数（デフォルト: 32）
		  stacks - 緯度方向の分割数（デフォルト: 32）
	====================================================================*/
	Sphere(float radius = 0.5f, int slices = 32, int stacks = 32)
	{
		// MeshRendererコンポーネントを追加
		auto meshRenderer = AddComponent<MeshRenderer>();

		// Sphereプリミティブメッシュを作成して設定
		auto sphereMesh = MeshFactory::CreateSphere(0.5f, slices, stacks);
		meshRenderer->SetMesh(sphereMesh);
		
		// TransFormは既にメンバとして存在するのでGetComponentで取得
		auto tf = GetComponent<TransForm>();
		tf->Scale() = Vector3(radius * 2.0f, radius * 2.0f, radius * 2.0f);
	}
	
	virtual ~Sphere() = default;
};

#endif
