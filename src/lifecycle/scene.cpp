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
#include "rendering/render_system.h"
#include "resourcemanagement/resource_manager.h"
#include "lifecycle/gameloop.h"

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
	Sceneを破棄する前に、全てのGameObjectとコンポーネントを
	安全にクリアする。
====================================================================*/
Scene::~Scene()
{
	// GameObjectを破棄する前に、各GameObjectに対してコンポーネントの登録解除を行う
	for (auto& go_ptr : m_GameObjects)
	{
		if (go_ptr)
		{
			// GameObjectが持つコンポーネントをクリア
			// これによりコンポーネントのデストラクタが呼ばれる
			go_ptr->m_Components.clear();
			
			// TransFormもクリア
			go_ptr->m_Transform.reset();
		}
	}

	// GameObjectsをクリア（unique_ptrのデストラクタが自動的に呼ばれる）
	m_GameObjects.clear();
	
	// マップもクリア
	m_GameObjectMap.clear();
	m_ComponentMap.clear();
	m_ComponentTypeMap.clear();
	m_TagMap.clear();
	m_NameMap.clear();
}

/*====================================================================
	GameObjectを登録する
	Sceneが所有権を取得し、IDを付与する。
	新規作成リストに追加して、後でProcessAwakeで初期化する。
	タグと名前のマップにも登録する。
	
	引数:
	  go - 登録するGameObject（new で作成された物）
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
	
	// タグマップに登録（デフォルトタグ"Untagged"）
	if (!go->m_Tag.empty())
	{
		RegisterGameObjectTag(go, go->m_Tag);
	}
	
	// 名前マップに登録
	if (!go->m_Name.empty())
	{
		RegisterGameObjectName(go, go->m_Name);
	}
}

/*====================================================================
	GameObjectの登録を解除する
	管理マップから削除し、所属する全Componentを登録解除する。
	タグと名前のマップからも削除する。
	
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
	
	// タグマップから削除
	if (!go->m_Tag.empty())
	{
		UnregisterGameObjectTag(go, go->m_Tag);
	}
	
	// 名前マップから削除
	if (!go->m_Name.empty())
	{
		UnregisterGameObjectName(go, go->m_Name);
	}
	
	go->m_Scene = nullptr;

	// 所属する全Componentを登録解除
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
	Sceneを終了する
	全てのGameObject/Componentを削除し、レンダリングシステムをクリーンアップする。
	タグと名前のマップもクリアする。
====================================================================*/
void Scene::Finalize()
{
	m_GameObjects.clear();
	
	m_GameObjectMap.clear();
	m_ComponentMap.clear();
	m_ComponentTypeMap.clear();
	
	// タグと名前のマップをクリア
	m_TagMap.clear();
	m_NameMap.clear();
	
	m_NextGameObjectId = 1;
	m_NextComponentId = 1;
	
	RenderSystem::Instance().Cleanup();

	ResourceManager::Instance().Clear();

	// GameLoopの未Startリストをクリア
	GameLoop::Instance().ClearPendingStart();
}

/*====================================================================
	破棄処理を実行する
	破棄フラグが立ったGameObjectを削除する。
====================================================================*/
void Scene::ProcessCleanup()
{
	CleanupDestroyedObjects();
}

/*====================================================================
	描画を実行する
	全てのカメラから全てのMeshRendererを描画する。
	RenderSystemに処理を委譲する。
====================================================================*/
void Scene::Render()
{
	// RenderSystemにレンダリングを委譲
	RenderSystem::Instance().Render(this);
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

/*====================================================================
	タグによるGameObjectの検索（単一）
	最初に見つかったGameObjectを返す。
	
	引数:
	  tag - 検索するタグ文字列
	戻り値: 見つかったGameObject（見つからない場合はnullptr）
====================================================================*/
GameObject* Scene::FindGameObjectWithTag(const std::wstring& tag) const
{
	auto it = m_TagMap.find(tag);
	if (it != m_TagMap.end())
	{
		return it->second;
	}
	return nullptr;
}

/*====================================================================
	タグによるすべてのGameObjectの検索
	同じタグを持つすべてのGameObjectを返す。
	
	引数:
	  tag - 検索するタグ文字列
	戻り値: 見つかったGameObjectのリスト
====================================================================*/
std::vector<GameObject*> Scene::FindGameObjectsWithTag(const std::wstring& tag) const
{
	std::vector<GameObject*> results;
	
	auto range = m_TagMap.equal_range(tag);
	for (auto it = range.first; it != range.second; ++it)
	{
		results.push_back(it->second);
	}
	
	return results;
}

/*====================================================================
	名前によるGameObjectの検索
	指定された名前を持つGameObjectを返す。
	
	引数:
	  name - 検索する名前
	戻り値: 見つかったGameObject（見つからない場合はnullptr）
====================================================================*/
GameObject* Scene::FindGameObjectByName(const std::wstring& name) const
{
	auto it = m_NameMap.find(name);
	if (it != m_NameMap.end())
	{
		return it->second;
	}
	return nullptr;
}

/*====================================================================
	タグマップにGameObjectを登録する
	
	引数:
	  go - 登録するGameObject
	  tag - 登録するタグ
====================================================================*/
void Scene::RegisterGameObjectTag(GameObject* go, const std::wstring& tag)
{
	if (!go || tag.empty())
	{
		return;
	}
	
	m_TagMap.insert(std::make_pair(tag, go));
}

/*====================================================================
	タグマップからGameObjectを解除する
	
	引数:
	  go - 解除するGameObject
	  oldTag - 解除する古いタグ
====================================================================*/
void Scene::UnregisterGameObjectTag(GameObject* go, const std::wstring& oldTag)
{
	if (!go || oldTag.empty())
	{
		return;
	}
	
	// マルチマップから特定のGameObjectのエントリを削除
	auto range = m_TagMap.equal_range(oldTag);
	for (auto it = range.first; it != range.second; )
	{
		if (it->second == go)
		{
			it = m_TagMap.erase(it);
		}
		else
		{
			++it;
		}
	}
}

/*====================================================================
	名前マップにGameObjectを登録する
	
	引数:
	  go - 登録するGameObject
	  name - 登録する名前
====================================================================*/
void Scene::RegisterGameObjectName(GameObject* go, const std::wstring& name)
{
	if (!go || name.empty())
	{
		return;
	}

	auto it = m_NameMap.find(name);
	if (it != m_NameMap.end())
	{
		throw std::runtime_error("Scene::RegisterGameObjectName: Duplicate GameObject name: " + std::string(name.begin(), name.end()));
	}

	m_NameMap[name] = go;
}

/*====================================================================
	名前マップからGameObjectを解除する
	
	引数:
	  go - 解除するGameObject
	  oldName - 解除する古い名前
====================================================================*/
void Scene::UnregisterGameObjectName(GameObject* go, const std::wstring& oldName)
{
	if (!go || oldName.empty())
	{
		return;
	}
	
	auto it = m_NameMap.find(oldName);
	if (it != m_NameMap.end() && it->second == go)
	{
		m_NameMap.erase(it);
	}
}
