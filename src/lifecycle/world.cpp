/*====================================================================

	World クラス実装 [world.cpp]
	Scene / GameObject / Component のグローバル管理

	Author : Ryosuke Kageyama
	Date   : 2025/11/26

====================================================================*/

#include "physics/physicssystem.h"
#include "lifecycle/world.h"
#include "lifecycle/scene.h"
#include "lifecycle/gameobject.h"
#include "lifecycle/time.h"
#include "direct3D/direct3d_device.h"
#include "direct3D/direct3d_RTV.h"
#include <stdexcept>

/*====================================================================
	アクティブなSceneを初期化する
	現在アクティブなSceneのInitializeメソッドを呼び出す。
====================================================================*/
void World::InitializeScene()
{
	if (m_ActiveScene)
	{
		m_ActiveScene->Initialize();
	}
}

/*====================================================================
	アクティブなSceneを取得する
	
	戻り値: 現在アクティブなSceneのポインタ
====================================================================*/
Scene* World::GetActiveScene()
{
	return m_ActiveScene.get();
}

/*====================================================================
	GameObjectを登録する
	アクティブなSceneにGameObjectを登録する。
	
	引数:
	  go - 登録するGameObject
	戻り値: 登録されたGameObjectのポインタ
	例外: アクティブなSceneが存在しない場合はruntime_errorをスロー
====================================================================*/
GameObject* World::RegisterGameObject(GameObject* go)
{
	if(!m_ActiveScene)
	{
		throw std::runtime_error("アクティブSceneが存在しません");
	}

	m_ActiveScene->RegisterGameObject(go);
	return go;
}

/*====================================================================
	ゲームループのティック処理
	物理更新、フレーム更新、描画を実行する。
====================================================================*/
void World::Tick()
{
	auto& device = GraphicsDevice::Instance();
	auto& rtm = RenderTargetManager::Instance();

	// 画面クリア
	rtm.BindAndClear(device.GetContext(), 0.1f, 0.1f, 0.1f, 1.0f, TRUE);
	rtm.SetViewport(device.GetContext(), 0);

	// フレーム開始
	Time::BeginFrame();

	if (!m_ActiveScene)
	{
		throw std::runtime_error("アクティブSceneが存在しません");
	}

	// 固定更新ループ
	while (Time::HasFixedStep())
	{
		PhysicsSystem::Instance().PhsicsUpdate();
		m_ActiveScene->FixedUpdate();
		Time::ConsumeFixedStep();
	}

	// ライフサイクル処理
	m_ActiveScene->ProcessAwake();
	m_ActiveScene->ProcessStart();
	m_ActiveScene->Update();
	m_ActiveScene->LateUpdate();
	m_ActiveScene->Render();

	// 画面更新
	device.Present();
}

