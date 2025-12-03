/*====================================================================

	SceneManager クラス実装 [scene_manager.cpp]
	Scene / GameObject / Component のグローバル管理

	Author : Ryosuke Kageyama
	Date   : 2025/11/26

====================================================================*/

#include "lifecycle/scene_manager.h"
#include "lifecycle/scene.h"
#include "lifecycle/gameobject.h"
#include <stdexcept>

/*====================================================================
	アクティブなSceneのリソースを読み込む
	現在アクティブなSceneのResourceLoadメソッドを呼び出す。
====================================================================*/
void SceneManager::ResourceLoadScene()
{
	if(m_ActiveScene)
	{
		m_ActiveScene->ResourceLoad();
	}
}

/*====================================================================
	アクティブなSceneを初期化する
	現在アクティブなSceneのInitializeメソッドを呼び出す。
====================================================================*/
void SceneManager::InitializeScene()
{
	if (m_ActiveScene)
	{
		m_ActiveScene->Initialize();
	}
}

/*====================================================================
	アクティブなSceneを終了する
	現在アクティブなSceneのFinalizeメソッドを呼び出す。
====================================================================*/
void SceneManager::FinalizeScene()
{
	if (m_ActiveScene)
	{
		m_ActiveScene->Finalize();
	}
}

/*====================================================================
	アクティブなSceneを取得する
	
	戻り値: 現在アクティブなSceneのポインタ
====================================================================*/
Scene* SceneManager::GetActiveScene()
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
GameObject* SceneManager::RegisterGameObject(GameObject* go)
{
	if(!m_ActiveScene)
	{
		throw std::runtime_error("アクティブSceneが存在しません");
	}

	m_ActiveScene->RegisterGameObject(go);
	return go;
}
