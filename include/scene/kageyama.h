/*====================================================================

	Kageyamaテストシーン [kageyama.h]
	3Dレンダリングのテストシーン。
	立方体とカメラを配置して描画をテストする。

	Author : Ryosuke Kageyama
	Date   : 2025/11/26

====================================================================*/

#ifndef KAGEYAMA_H
#define KAGEYAMA_H

#include "lifecycle.h"
#include "direct3D/mesh_renderer.h"
#include "direct3D/resource_factory.h"

using namespace DirectX;

/*====================================================================
	Kageyamaクラス
	3Dレンダリングのテストシーン。
	立方体メッシュとカメラを生成して描画をテストする。
====================================================================*/
class Kageyama : public Scene
{
public:
	/*====================================================================
		シーンを初期化する
		シェーダー、メッシュ、マテリアルを作成し、
		GameObjectとComponentを配置する。
	====================================================================*/
	void Initialize() override
	{
		// 立方体GameObjectの作成
		GameObject* cube1 = new GameObject();
		auto mr1 = cube1->AddComponent<MeshRenderer>();
		mr1->SetMesh(MeshFactory::CreateCube(1.0f));
		mr1->GetMaterial()->Texture = TextureFactory::Create(L"grass.png");
		auto tf1 = cube1->GetComponent<TransForm>();
		tf1->Position.z += 2.0f;
		tf1->Position.y += 3.0f;

		GameObject* cube2 = new GameObject();
		auto mr2 = cube2->AddComponent<MeshRenderer>();
		mr2->SetMesh(MeshFactory::CreateCube(2.0f));
		auto tf2 = cube2->GetComponent<TransForm>();
		tf2->Position.x -= 2.0f;


		// カメラGameObjectの作成
		GameObject* camobj = new GameObject();
		camobj->AddComponent<Camera>();
		auto camtf = camobj->GetComponent<TransForm>();
		camtf->Position.z -= 5.0f;
	}
};

#endif // KAGEYAMA_H
