/*====================================================================

	Prefabクラス実装 [prefab.cpp]
	GameObjectのテンプレートとして使用される静的リソース。

	Author : Ryosuke Kageyama
	Date   : 2025/11/26

====================================================================*/

#include "resourcemanagement/prefab.h"
#include "lifecycle/gameobject.h"
#include "lifecycle/gameloop.h"
#include "lifecycle/scene.h"
#include "rendering/mesh_renderer.h"
#include <stdexcept>
#include <unordered_map>

/*====================================================================
	コンストラクタ
	Prefabオブジェクトを生成し、ClassIDをModelに設定する。
====================================================================*/
Prefab::Prefab()
	: m_PrefabName(L"")
	, m_RootGameObject(nullptr)
{
	// ClassIDを静的リソース「Model」に設定
	SetClassID(ResourceClassID::Model);
}

/*====================================================================
	デストラクタ
====================================================================*/
Prefab::~Prefab()
{
}

/*====================================================================
	Initialize - Prefabを初期化する
	ルートGameObjectとすべての子GameObject、名前を設定する。
====================================================================*/
void Prefab::Initialize(
	std::shared_ptr<GameObject> rootGameObject,
	std::vector<std::shared_ptr<GameObject>> allGameObjects,
	const std::wstring& name
)
{
	if (!rootGameObject)
	{
		throw std::runtime_error("Prefab::Initialize: rootGameObject is null");
	}

	m_RootGameObject = rootGameObject;
	m_AllGameObjects = std::move(allGameObjects);
	m_PrefabName = name;
	
	// Objectの名前を設定
	SetName(name);
}

/*====================================================================
	GetRootGameObject - ルートGameObjectを取得する
====================================================================*/
std::shared_ptr<GameObject> Prefab::GetRootGameObject() const
{
	return m_RootGameObject;
}

/*====================================================================
	GetAllGameObjects - すべてのGameObjectを取得する
====================================================================*/
const std::vector<std::shared_ptr<GameObject>>& Prefab::GetAllGameObjects() const
{
	return m_AllGameObjects;
}

/*====================================================================
	GetPrefabName - プレハブ名を取得する
====================================================================*/
const std::wstring& Prefab::GetPrefabName() const
{
	return m_PrefabName;
}

/*====================================================================
	Instantiate - PrefabからGameObjectを生成する
	すべてのGameObjectを複製し、現在のアクティブシーンに登録する。
	親子関係とコンポーネント（MeshRenderer等）も完全にコピーする。
	生成されたルートGameObjectの生ポインタを返す。
====================================================================*/
GameObject* Prefab::Instantiate(Prefab* original)
{
	if (!original)
	{
		throw std::runtime_error("Prefab::Instantiate: original is null");
	}

	if (!original->m_RootGameObject)
	{
		throw std::runtime_error("Prefab::Instantiate: root GameObject is not initialized");
	}

	// 現在のアクティブシーンを取得
	Scene* currentScene = SceneManager::Instance().GetActiveScene();
	if (!currentScene)
	{
		throw std::runtime_error("Prefab::Instantiate: No active scene");
	}

	// 元のGameObjectから新しいGameObjectへのマッピング
	std::unordered_map<GameObject*, GameObject*> objectMap;
	
	// すべてのGameObjectを複製し、Sceneに登録
	GameObject* clonedRoot = nullptr;
	
	for (const auto& srcGameObject : original->m_AllGameObjects)
	{
		// GameObjectを複製（newで作成、Sceneに自動登録される）
		GameObject* cloned = new GameObject();
		cloned->SetName(srcGameObject->GetName());
		
		// マッピングに追加
		objectMap[srcGameObject.get()] = cloned;
		
		// ルートGameObjectを保存
		if (srcGameObject == original->m_RootGameObject)
		{
			clonedRoot = cloned;
		}
	}
	
	// 親子関係を設定
	for (const auto& srcGameObject : original->m_AllGameObjects)
	{
		GameObject* cloned = objectMap[srcGameObject.get()];
		auto srcTransform = srcGameObject->GetComponent<TransForm>();
		auto dstTransform = cloned->GetComponent<TransForm>();
		
		if (!srcTransform || !dstTransform)
		{
			continue;
		}
		
		// 位置・回転・スケールをコピー
		dstTransform->Position() = srcTransform->Position();
		dstTransform->Rotation() = srcTransform->Rotation();
		dstTransform->Scale() = srcTransform->Scale();
		
		// 親子関係を設定
		auto srcParent = srcTransform->GetParent();
		if (srcParent)
		{
			// 親のGameObjectを検索
			GameObject* srcParentGameObject = srcParent->GetGameObject();
			auto it = objectMap.find(srcParentGameObject);
			if (it != objectMap.end())
			{
				GameObject* dstParentGameObject = it->second;
				auto dstParentTransform = dstParentGameObject->GetComponent<TransForm>();
				if (dstParentTransform)
				{
					dstTransform->SetParent(dstParentTransform);
				}
			}
		}
	}
	
	// コンポーネントをコピー（MeshRenderer等）
	for (const auto& srcGameObject : original->m_AllGameObjects)
	{
		GameObject* cloned = objectMap[srcGameObject.get()];
		
		// MeshRendererをコピー
		auto srcMeshRenderer = srcGameObject->GetComponent<MeshRenderer>();
		if (srcMeshRenderer)
		{
			auto dstMeshRenderer = cloned->AddComponent<MeshRenderer>();
			dstMeshRenderer->SetMesh(srcMeshRenderer->GetMesh());
			dstMeshRenderer->SetMaterial(srcMeshRenderer->GetSharedMaterial());
		}
	}
	
	if (!clonedRoot)
	{
		throw std::runtime_error("Prefab::Instantiate: Failed to clone root GameObject");
	}

	return clonedRoot;
}

/*====================================================================
	Destroy - Prefabを削除する
	削除フラグを立てる。
====================================================================*/
void Prefab::Destroy(Prefab* obj)
{
	if (!obj)
	{
		throw std::runtime_error("Prefab::Destroy: obj is null");
	}

	// 削除フラグを立てる
	obj->m_Destroy = true;
}
