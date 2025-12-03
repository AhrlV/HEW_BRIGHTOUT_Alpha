/*====================================================================

	GameLoop実装 [gameloop.cpp]
	ゲームループの実装。

	Author : Ryosuke Kageyama
	Date   : 2025/11/26

====================================================================*/

#include "lifecycle/gameloop.h"
#include "lifecycle/component.h"
#include "lifecycle/scene.h"
#include "lifecycle/scene_manager.h"
#include "lifecycle/gameobject.h"
#include "lifecycle/time.h"
#include "physics/physicssystem.h"
#include "direct3D/direct3d_device.h"
#include "direct3D/direct3d_RTV.h"
#include <stdexcept>

/*====================================================================
	ComponentのAwake後に呼び出される
	未Startリストに追加する。
	
	引数:
	  comp - Awakeが呼ばれたComponent
====================================================================*/
void GameLoop::RegisterAwakeComponent(Component* comp)
{
	if (!comp)
	{
		return;
	}

	m_PendingStartComponents.push_back(comp);
}

/*====================================================================
	物理更新処理
	PhysicsSystemの更新を行う。
	
	引数:
	  scene - 現在のScene
====================================================================*/
void GameLoop::ProcessPhysics(Scene* scene)
{
	if (!scene)
	{
		return;
	}

	PhysicsSystem::Instance().PhsicsUpdate();
}

/*====================================================================
	未StartなComponentのStartを呼び出す
	GameObjectとComponentが両方有効化されている場合のみStartを実行する。
	
	引数:
	  scene - 現在のScene
====================================================================*/
void GameLoop::ProcessStart(Scene* scene)
{
	if (!scene)
	{
		return;
	}

	// 未StartなComponentを処理
	auto it = m_PendingStartComponents.begin();
	while (it != m_PendingStartComponents.end())
	{
		Component* comp = *it;

		// 既にStartが呼ばれているか確認
		if (comp->m_IsStartCalled)
		{
			it = m_PendingStartComponents.erase(it);
			continue;
		}

		// GameObjectとComponentが両方有効な場合のみStartを呼ぶ
		GameObject* owner = comp->GetGameObject();
		if (owner && owner->IsActiveInHierarchy() && comp->IsActive())
		{
			comp->Start();
			comp->m_IsStartCalled = true;
			it = m_PendingStartComponents.erase(it);
		}
		else
		{
			++it;
		}
	}
}

/*====================================================================
	全ComponentのUpdateを呼び出す
	
	引数:
	  scene - 現在のScene
====================================================================*/
void GameLoop::ProcessUpdate(Scene* scene)
{
	if (!scene)
	{
		return;
	}

	// SceneからComponentマップを取得して更新
	const auto& componentMap = scene->GetAllComponents();
	for (const auto& pair : componentMap)
	{
		Component* comp = pair.second;
		if (!comp)
		{
			continue;
		}

		GameObject* owner = comp->GetGameObject();
		if (owner && owner->IsActiveInHierarchy() && comp->IsActive())
		{
			comp->Update();
		}
	}
}

/*====================================================================
	全ComponentのLateUpdateを呼び出す
	
	引数:
	  scene - 現在のScene
====================================================================*/
void GameLoop::ProcessLateUpdate(Scene* scene)
{
	if (!scene)
	{
		return;
	}

	// SceneからComponentマップを取得して更新
	const auto& componentMap = scene->GetAllComponents();
	for (const auto& pair : componentMap)
	{
		Component* comp = pair.second;
		if (!comp)
		{
			continue;
		}

		GameObject* owner = comp->GetGameObject();
		if (owner && owner->IsActiveInHierarchy() && comp->IsActive())
		{
			comp->LateUpdate();
		}
	}
}

/*====================================================================
	全ComponentのFixedUpdateを呼び出す
	
	引数:
	  scene - 現在のScene
====================================================================*/
void GameLoop::ProcessFixedUpdate(Scene* scene)
{
	if (!scene)
	{
		return;
	}

	// SceneからComponentマップを取得して更新
	const auto& componentMap = scene->GetAllComponents();
	for (const auto& pair : componentMap)
	{
		Component* comp = pair.second;
		if (!comp)
		{
			continue;
		}

		GameObject* owner = comp->GetGameObject();
		if (owner && owner->IsActiveInHierarchy() && comp->IsActive())
		{
			comp->FixedUpdate();
		}
	}
}

/*====================================================================
	破棄処理を実行する
	
	引数:
	  scene - 現在のScene
====================================================================*/
void GameLoop::ProcessCleanup(Scene* scene)
{
	if (!scene)
	{
		return;
	}

	scene->ProcessCleanup();
}

/*====================================================================
	描画処理を実行する
	
	引数:
	  scene - 現在のScene
====================================================================*/
void GameLoop::ProcessRender(Scene* scene)
{
	if (!scene)
	{
		return;
	}

	// 画面クリア
	auto& device = GraphicsDevice::Instance();
	auto& rtm = RenderTargetManager::Instance();
	rtm.BindAndClear(device.GetContext(), 0.1f, 0.1f, 0.1f, 1.0f, TRUE);
	rtm.SetViewport(device.GetContext(), 0);

	// 描画処理
	scene->Render();

	// 画面更新
	device.Present();
}

/*====================================================================
	ゲームループのメインティック処理
	物理更新、Awake、Start、Update、LateUpdate、破棄、描画を実行する。
	
	例外:
	  アクティブなSceneが存在しない場合はruntime_errorをスロー
====================================================================*/
void GameLoop::Tick()
{
	// フレーム開始
	Time::BeginFrame();

	// アクティブなSceneを取得
	Scene* scene = SceneManager::Instance().GetActiveScene();
	if (!scene)
	{
		throw std::runtime_error("アクティブSceneが存在しません");
	}

	// 固定更新ループ
	while (Time::HasFixedStep())
	{
		ProcessPhysics(scene);
		ProcessFixedUpdate(scene);
		Time::ConsumeFixedStep();
	}


	// Start処理
	ProcessStart(scene);

	// Update処理
	ProcessUpdate(scene);

	// LateUpdate処理
	ProcessLateUpdate(scene);

	// 破棄処理
	ProcessCleanup(scene);

	// 描画処理
	ProcessRender(scene);
}

/*====================================================================
	未Startリストをクリアする
	Scene切り替え時などに呼び出す。
====================================================================*/
void GameLoop::ClearPendingStart()
{
	m_PendingStartComponents.clear();
}
