/*====================================================================

	World ä÷êîåQ é¿ëï [world.cpp]

	Author : Ryosuke Kageyama
	Date   : 2025/11/19
====================================================================*/

#include "physics/physicssystem.h"
#include "lifecycle/world.h"
#include "lifecycle/scene.h"
#include "lifecycle/gameobject.h"
#include "lifecycle/time.h"


static std::unique_ptr<Scene> g_ActiveScene;

namespace World
{
	void SetScene(std::unique_ptr<Scene> scene)
	{
		g_ActiveScene = std::move(scene);
		if (g_ActiveScene) g_ActiveScene->Initialize();
	}

	const Scene* GetActiveScene() { return g_ActiveScene.get(); }

	void DestroyGameObject(GameObject* go) { if (g_ActiveScene) g_ActiveScene->DestroyGameObject(go); }

	GameObject* RegisterGameObject(GameObject* go)
	{
		return (g_ActiveScene && go) ? g_ActiveScene->AdoptGameObject(go) : nullptr;
	}

	void Tick()
	{
		Time::BeginFrame();
		if (!g_ActiveScene) return;

		while (Time::HasFixedStep()) {
			PhysicsSystem::Instance().PhsicsUpdate();
			g_ActiveScene->FixedUpdate();
			Time::ConsumeFixedStep();
		}

		g_ActiveScene->ProcessAwake();
		g_ActiveScene->ProcessStart();
		g_ActiveScene->Update();
		g_ActiveScene->LateUpdate();
		g_ActiveScene->Render();
	}
}

