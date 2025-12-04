#ifndef CYLINDER_H
#define CYLINDER_H

#include "lifecycle.h"
#include "rendering.h"
#include "resource_management.h"

class Cylinder : public GameObject
{
public:
	/*====================================================================
		コンストラクタ
		MeshRendererを追加し、Cylinderプリミティブメッシュを作成して設定する。
		
		引数:
		  radius - 円柱の半径（デフォルト: 0.5f）
		  height - 円柱の高さ（デフォルト: 1.0f）
		  slices - 円周方向の分割数（デフォルト: 32）
	====================================================================*/
	Cylinder(float radius = 0.5f, float height = 1.0f, int slices = 32)
	{
		// MeshRendererコンポーネントを追加
		auto meshRenderer = AddComponent<MeshRenderer>();

		// Cylinderプリミティブメッシュを作成して設定
		auto cylinderMesh = MeshFactory::CreateCylinder(0.5f, height, slices);
		meshRenderer->SetMesh(cylinderMesh);
		
		// TransFormは既にメンバとして存在するのでGetComponentで取得
		auto tf = GetComponent<TransForm>();
		tf->Scale() = Vector3(radius * 2.0f, 1.0f, radius * 2.0f);
	}
	
	virtual ~Cylinder() = default;
};

#endif
