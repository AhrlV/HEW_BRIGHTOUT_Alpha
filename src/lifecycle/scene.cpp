/*====================================================================


    Scene 実装 [scene.cpp]
    GameObject / Component の登録とライフサイクル処理を管理
	
	Author : Ryosuke Kageyama
	Date   : 2025/11/26

====================================================================*/


#include <algorithm>
#include "lifecycle/scene.h"
#include "lifecycle/gameobject.h"
#include "lifecycle/component.h"
#include "lifecycle/camera.h"
#include "direct3D/mesh_renderer.h"

/*====================================================================
	コンストラクタ
	Sceneを初期化する。
====================================================================*/
Scene::Scene()
	: m_NextGameObjectId(1)
	, m_NextComponentId(1)
{
}

/*====================================================================
	デストラクタ
	Sceneを破棄する。
====================================================================*/
Scene::~Scene() = default;

/*====================================================================
	GameObjectを登録する
	Sceneが所有権を取得し、IDを付与する。
	新規作成リストに追加して、次のProcessAwakeで初期化される。
	
	引数:
	  go - 登録するGameObject（new で作成されたもの）
====================================================================*/
void Scene::RegisterGameObject(GameObject* go)
{
	if(!go)
	{
		return;
	}

	// 所有権を取得
	m_GameObjects.emplace_back(std::unique_ptr<GameObject>(go));

	// IDとSceneポインタを設定
	go->m_Id = m_NextGameObjectId++;
	go->m_Scene = this;
	m_GameObjectMap[go->m_Id] = go;

	// 新規作成リストに追加（ProcessAwakeで処理される）
	m_NewlyCreated.push_back(go);
}

/*====================================================================
	GameObjectの登録を解除する
	管理マップから削除し、所属するコンポーネントも登録解除する。
	
	引数:
	  go - 登録解除するGameObject
====================================================================*/
void Scene::UnregisterGameObject(GameObject* go)
{
	if(!go)
	{
		return;
	}

	// マップから削除
	m_GameObjectMap.erase(go->m_Id);
	go->m_Scene = nullptr;

	// 所属する全コンポーネントを登録解除
	for(auto& comp : go->m_Components)
	{
		UnregisterComponent(comp.get());
	}
}

/*====================================================================
	Componentを登録する
	IDを付与し、型別マップに追加する。
	
	引数:
	  comp - 登録するComponent
	  owner - Componentの所有者
====================================================================*/
void Scene::RegisterComponent(Component* comp, GameObject* owner)
{
	if(!comp || !owner)
	{
		return;
	}

	// 既に登録済みの場合はスキップ
	if(comp->m_Id != 0)
	{
		return;
	}

	// IDを付与
	comp->m_Id = m_NextComponentId++;

	// マップに登録
	m_ComponentMap[comp->m_Id] = comp;
	m_ComponentTypeMap[std::type_index(typeid(*comp))].push_back(comp);
}

/*====================================================================
	Componentの登録を解除する
	管理マップから削除する。
	
	引数:
	  comp - 登録解除するComponent
====================================================================*/
void Scene::UnregisterComponent(Component* comp)
{
	if(!comp)
	{
		return;
	}

	// マップから削除
	m_ComponentMap.erase(comp->m_Id);

	// 型別マップから削除
	auto tindex = std::type_index(typeid(*comp));
	auto it = m_ComponentTypeMap.find(tindex);
	if(it != m_ComponentTypeMap.end())
	{
		auto& vec = it->second;
		vec.erase(std::remove(vec.begin(), vec.end(), comp), vec.end());
		if(vec.empty())
		{
			m_ComponentTypeMap.erase(tindex);
		}
	}

	// IDをリセット
	comp->m_Id = 0;
}

/*====================================================================
	破棄フラグが立ったオブジェクトを削除する
	Destroyフラグが立っているGameObjectを実際に削除する。
====================================================================*/
void Scene::CleanupDestroyedObjects()
{
	// 破棄予約されたGameObjectを削除
	auto it = std::remove_if(m_GameObjects.begin(), m_GameObjects.end(),
		[this](const std::unique_ptr<GameObject>& go)
		{
			if (go->IsDestroyed())
			{
				// 削除前に登録解除
				UnregisterGameObject(go.get());
				return true;
			}
			return false;
		});

	m_GameObjects.erase(it, m_GameObjects.end());
}

/*====================================================================
	GameObjectを破棄する
	Destroyフラグを立てる。実際の削除はCleanupDestroyedObjects()で行われる。
	
	引数:
	  target - 破棄するGameObject
====================================================================*/
void Scene::DestroyGameObject(GameObject* target)
{
	if (!target)
	{
		return;
	}

	target->Destroy();
}

/*====================================================================
	Sceneを初期化する
	派生クラスでオーバーライドして使用する。
====================================================================*/
void Scene::Initialize() {}

/*====================================================================
	新規GameObjectのAwakeを処理する
	新しく作成されたGameObjectとそのコンポーネントを初期化する。
====================================================================*/
void Scene::ProcessAwake()
{
	for (auto* go : m_NewlyCreated)
	{
		// 追加されたコンポーネントをSceneに登録
		for(auto* comp : go->m_AddedComponents)
		{
			RegisterComponent(comp, go);
		}

		// Awakeを呼び出す
		// 注: AwakeNewComponents()内でm_AddedComponentsがクリアされる
		go->AwakeNewComponents();
	}
	m_NewlyCreated.clear();
}

/*====================================================================
	StartをGameObjectに対して処理する
	まだStartが呼ばれていないコンポーネントのStartメソッドを呼び出す。
====================================================================*/
void Scene::ProcessStart()
{
	for (auto& gameobj : m_GameObjects)
	{
		// 動的に追加されたコンポーネントをSceneに登録
		for(auto* comp : gameobj->m_AddedComponents)
		{
			RegisterComponent(comp, gameobj.get());
		}

		// Startを呼び出す
		// 注: StartNewComponents()内でm_AddedComponentsがクリアされる
		gameobj->StartNewComponents();
	}
}

/*====================================================================
	フレーム更新を実行する
	全てのGameObjectのUpdateメソッドを呼び出す。
====================================================================*/
void Scene::Update()
{
	// 動的に追加されたコンポーネントをSceneに登録
	for(auto& gameobj : m_GameObjects)
	{
		for(auto* comp : gameobj->m_AddedComponents)
		{
			RegisterComponent(comp, gameobj.get());
		}
		gameobj->m_AddedComponents.clear();
	}

	// Update呼び出し
	for (auto& gameobj : m_GameObjects)
	{
		gameobj->Update();
	}

	// 更新の最後に破棄フラグが立ったオブジェクトを削除
	CleanupDestroyedObjects();
}

/*====================================================================
	後段更新を実行する
	全てのGameObjectのLateUpdateメソッドを呼び出す。
====================================================================*/
void Scene::LateUpdate()
{
	for (auto& gameobj : m_GameObjects)
	{
		gameobj->LateUpdate();
	}

	// 更新の最後に破棄フラグが立ったオブジェクトを削除
	CleanupDestroyedObjects();
}

/*====================================================================
	固定更新を実行する
	全てのGameObjectのFixedUpdateメソッドを呼び出す。
====================================================================*/
void Scene::FixedUpdate()
{
	for (auto& gameobj : m_GameObjects)
	{
		gameobj->FixedUpdate();
	}

	// 更新の最後に破棄フラグが立ったオブジェクトを削除
	CleanupDestroyedObjects();
}

/*====================================================================
	描画を実行する
	全てのカメラから全てのMeshRendererを描画する。
====================================================================*/
void Scene::Render()
{
	auto cameras = GetComponentsByType<Camera>();
	auto renderers = GetComponentsByType<MeshRenderer>();

	for (auto* cam : cameras)
	{
		for (auto* mr : renderers)
		{
			mr->Render(*cam);
		}
	}
}

/*====================================================================
	IDでGameObjectを取得する
	
	引数:
	  id - 検索するID
	戻り値: 見つかったGameObject（存在しない場合はnullptr）
====================================================================*/
GameObject* Scene::GetGameObjectById(uint64_t id) const
{
	auto it = m_GameObjectMap.find(id);
	return it == m_GameObjectMap.end() ? nullptr : it->second;
}

/*====================================================================
	IDでComponentを取得する
	
	引数:
	  id - 検索するID
	戻り値: 見つかったComponent（存在しない場合はnullptr）
====================================================================*/
Component* Scene::GetComponentById(uint64_t id) const
{
	auto it = m_ComponentMap.find(id);
	return it == m_ComponentMap.end() ? nullptr : it->second;
}
