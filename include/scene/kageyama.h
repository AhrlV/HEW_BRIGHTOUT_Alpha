#ifndef KAGEYAMA_H
#define KAGEYAMA_H

#include "lifecycle.h"
#include "rendering.h"
#include "primitive_objects.h"
#include "resource_management.h"


class Kageyama : public Scene
{
private:
	Textureptr player_tex;
	Modelptr kirby_prefab;
	Modelptr house_prefab;

public:

	/*====================================================================
		ResourceLoad - リソースの読み込み
		テクスチャとモデルを読み込む。
	====================================================================*/
	void ResourceLoad() override
	{
		player_tex = TextureFactory::Create(L"dirt.png");
		kirby_prefab = ModelFactory::LoadFromFBX(L"kirby.fbx", 10.0f);
		house_prefab = ModelFactory::LoadFromFBX(L"house.fbx", 0.5f);
	}

	/*====================================================================
		Initialize - シーンの初期化
		GameObjectを作成し、配置する。
	====================================================================*/
	void Initialize() override
	{	
		Cube* ground = new Cube();
		auto ground_mr = ground->GetComponent<MeshRenderer>();
		ground_mr->material.SetTexture(player_tex);
		auto tf1 = ground->GetComponent<TransForm>();
		tf1->Scale() = Vector3(10.0f, 1.0f, 10.0f);
		tf1->Position().y -= 3.0f;
		

		auto house = Prefab::Instantiate(house_prefab.get());
		auto house_tf = house->GetComponent<TransForm>();
		house_tf->Rotation().SetEulerY(-90.0f);


		auto kirby = Prefab::Instantiate(kirby_prefab.get());
		auto tf = kirby->GetComponent<TransForm>();
		tf->Position().x = 3.0f;

		// カメラを作成
		CameraObj* camobj = new CameraObj();
		auto camtf = camobj->GetComponent<TransForm>();
		camtf->Position().z -= 5.0f;
		camobj->SetName(L"MainCamera");
		camobj->SetTag(L"camera");
	}
};

#endif // KAGEYAMA_H
