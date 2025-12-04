
#ifndef CAPSULE_H
#define CAPSULE_H

#include "lifecycle.h"
#include "rendering.h"
#include "resource_management.h"

class Capsule : public GameObject
{
public:
	/*====================================================================
		コンストラクタ
		MeshRendererを追加し、Capsuleプリミティブメッシュを作成して設定する。
		
		引数:
		  radius - カプセルの半径（デフォルト: 0.5f）
		  height - カプセルの円柱部分の高さ（デフォルト: 1.0f）
		  slices - 円周方向の分割数（デフォルト: 32
		  stacks - 球の上下部分の分割数（デフォルト: 16）
	====================================================================*/
	Capsule(float radius = 0.5f, float height = 1.0f, int slices = 32, int stacks = 16)
	{
		// MeshRendererコンポーネントを追加
		auto meshRenderer = AddComponent<MeshRenderer>();

		// Capsuleプリミティブメッシュを作成して設定
		auto capsuleMesh = MeshFactory::CreateCapsule(radius, height, slices, stacks);
		meshRenderer->SetMesh(capsuleMesh);
		auto tf = GetComponent<TransForm>();
		tf->Scale() = {radius * 2.0f, 1.0f, radius * 2.0f};
	}
	virtual ~Capsule() = default;
};

#endif // CAPSULE_H
