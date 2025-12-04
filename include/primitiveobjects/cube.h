#ifndef CUBE_H
#define CUBE_H

#include "lifecycle.h"
#include "rendering.h"
#include "resource_management.h"

class Cube : public GameObject
{
public:
	/*====================================================================
		コンストラクタ
		MeshRendererを追加し、Cubeプリミティブメッシュを作成して設定する。
		
		引数:
		  size - 立方体のサイズ（デフォルト: 1.0f）
	====================================================================*/
	Cube(float size = 1.0f)
	{
		// MeshRendererコンポーネントを追加
		auto meshRenderer = AddComponent<MeshRenderer>();

		// Cubeプリミティブメッシュを作成して設定
		auto cubeMesh = MeshFactory::CreateCube();
		meshRenderer->SetMesh(cubeMesh);
		
		// TransFormは既にメンバとして存在するのでGetComponentで取得
		auto tf = GetComponent<TransForm>();
		tf->Scale() = Vector3(size, size, size);
	}
	
	virtual ~Cube() = default;
};

#endif
