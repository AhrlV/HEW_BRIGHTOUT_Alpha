
#ifndef KAGEYAMA_H
#define KAGEYAMA_H

#include "lifecycle.h"
#include "rendering.h"
#include "resource_management.h"
#include <memory>


using namespace DirectX;

class Kageyama : public Scene
{
private:


public:

	void ResourceLoad() override
	{

	}

	void Initialize() override
	{
		GameObject* cube1 = new GameObject();
		auto mr1 = cube1->AddComponent<MeshRenderer>();
		mr1->SetMesh(MeshFactory::CreateCube());
		mr1->material.SetBaseColor({1.0f, 1.0f, 1.0f, 1.0f});
		mr1->material.SetTexture(TextureFactory::Create(L"dirt.png"));
		auto tf1 = cube1->GetComponent<TransForm>();
		tf1->Position.z += 2.0f;
		tf1->Position.y += 3.0f;

		GameObject* cube2 = new GameObject();
		auto mr2 = cube2->AddComponent<MeshRenderer>();
		mr2->SetMesh(MeshFactory::CreateCube(2.0f));
		auto tf2 = cube2->GetComponent<TransForm>();
		tf2->Position.x -= 2.0f;

		GameObject* camobj = new GameObject();
		camobj->AddComponent<Camera>();
		auto camtf = camobj->GetComponent<TransForm>();
		camtf->Position.z -= 5.0f;
	}
};

#endif
