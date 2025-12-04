/*====================================================================


    GameObject 実装 [gameobject.cpp]
	GameObjectクラスの実装。

                                    Author : Ryosuke Kageyama
                                    Date   : 2025/11/26
====================================================================*/

#include "lifecycle/gameobject.h"
#include "physics/transform.h"
#include "lifecycle/scene_manager.h"
#include <stdexcept>

/*====================================================================
	コンストラクタ
	GameObjectを生成し、必須のTransFormコンポーネントを作成する。
	
	処理の流れ:
	1. TransFormを直接メンバ変数として作成
	2. TransFormのAwakeを即座に呼び出す
	3. SceneManager::Instance().RegisterGameObject()でSceneに登録
	4. Scene::RegisterGameObject()でm_Sceneを設定する
====================================================================*/
GameObject::GameObject()
	: Object()
	, m_Scene(nullptr)
	, m_Transform(std::make_unique<TransForm>())
	, m_ActiveSelf(true)
	, m_Tag(L"Untagged")
{
	// GameObjectのクラスIDを設定
	m_ClassID = ResourceClassID::GameObject;

	// TransFormのオーナーを設定
	m_Transform->m_Owner = this;
	
	// TransFormのAwakeを即座に呼び出す
	if (!m_Transform->m_IsAwakeCalled)
	{
		m_Transform->Awake();
		m_Transform->m_IsAwakeCalled = true;
		
		// GameLoopの未Startプールに登録
		GameLoop::Instance().RegisterAwakeComponent(m_Transform.get());
	}
	
	// Sceneに登録
	// ここでm_Sceneが設定され、m_NewlyCreatedに追加される
	SceneManager::Instance().RegisterGameObject(this);
}

/*====================================================================
	Prefab用のコンストラクタ
	GameObjectを生成するが、Scene登録をスキップできる。
	Prefabのテンプレートとして使用するGameObjectを作成する際に使用。
	
	引数:
	  skipSceneRegistration - trueの場合、Scene登録をスキップ
	
	処理の流れ:
	1. TransFormを直接メンバ変数として作成
	2. TransFormのAwakeを即座に呼び出す
	3. skipSceneRegistration = falseの場合のみ、Sceneに登録
====================================================================*/
GameObject::GameObject(bool skipSceneRegistration)
	: Object()
	, m_Scene(nullptr)
	, m_Transform(std::make_unique<TransForm>())
	, m_ActiveSelf(true)
	, m_Tag(L"Untagged")
{
	// GameObjectのクラスIDを設定
	m_ClassID = ResourceClassID::GameObject;

	// TransFormのオーナーを設定
	m_Transform->m_Owner = this;
	
	// TransFormのAwakeを即座に呼び出す
	if (!m_Transform->m_IsAwakeCalled)
	{
		m_Transform->Awake();
		m_Transform->m_IsAwakeCalled = true;
		
		// GameLoopの未Startプールに登録
		GameLoop::Instance().RegisterAwakeComponent(m_Transform.get());
	}
	
	// Prefab用の場合はSceneへの登録をスキップ
	if (!skipSceneRegistration)
	{
		// Sceneに登録
		// ここでm_Sceneが設定され、m_NewlyCreatedに追加される
		SceneManager::Instance().RegisterGameObject(this);
	}
}

/*====================================================================
	所属するSceneを取得する
	
	戻り値: このGameObjectが所属するSceneのポインタ
====================================================================*/
Scene* GameObject::GetScene() const
{
	return m_Scene;
}

/*====================================================================
	自分のアクティブ状態を設定する
	
	引数:
	  active - 設定するアクティブ状態
====================================================================*/
void GameObject::SetActiveSelf(bool active)
{
	m_ActiveSelf = active;
	Object::SetActive(active);
}

/*====================================================================
	自分のアクティブ状態を取得する
	
	戻り値: 自分がアクティブな場合true
====================================================================*/
bool GameObject::IsActiveSelf() const
{
	return m_ActiveSelf;
}

/*====================================================================
	名前を設定する
	名前を設定し、Sceneの名前マップに自動的に登録する。
	
	引数:
	  name - 設定する名前
====================================================================*/
void GameObject::SetName(const std::wstring& name)
{
	// 古い名前での登録を解除
	if (m_Scene && !m_Name.empty())
	{
		m_Scene->UnregisterGameObjectName(this, m_Name);
	}
	
	// 新しい名前を設定
	m_Name = name;
	
	// 新しい名前で登録
	if (m_Scene && !m_Name.empty())
	{
		m_Scene->RegisterGameObjectName(this, m_Name);
	}
}

/*====================================================================
	階層でのアクティブ状態を取得する
	親階層を遡って、すべての親がアクティブかどうかを確認する。
	自分または親のいずれかが非アクティブの場合、falseを返す。
	
	戻り値: 階層でアクティブな場合true
====================================================================*/
bool GameObject::IsActiveInHierarchy() const
{
	// 自分が非アクティブの場合はfalse
	if (!m_ActiveSelf)
	{
		return false;
	}
	
	// TransFormが存在しない場合は自分のアクティブ状態のみを返す
	if (!m_Transform)
	{
		return m_ActiveSelf;
	}
	
	// 親を遡って確認
	TransForm* parentTransform = m_Transform->GetParent();
	while (parentTransform)
	{
		GameObject* parentGameObject = parentTransform->GetGameObject();
		
		// 親GameObjectが存在しない場合は続行
		if (!parentGameObject)
		{
			break;
		}
		
		// 親が非アクティブの場合はfalse
		if (!parentGameObject->IsActiveSelf())
		{
			return false;
		}
		
		// さらに親を遡る
		parentTransform = parentTransform->GetParent();
	}
	
	// すべての親がアクティブの場合はtrue
	return true;
}

/*====================================================================
	親GameObjectを設定する
	TransFormの親子関係を設定する。
	
	引数:
	  parent - 親となるGameObject（nullptrの場合は親を解除）
====================================================================*/
void GameObject::SetParent(GameObject* parent)
{
	if (!m_Transform)
	{
		return;
	}
	
	if (parent)
	{
		// 親のTransFormを取得
		TransForm* parentTransform = parent->GetComponent<TransForm>();
		if (parentTransform)
		{
			m_Transform->SetParent(parentTransform);
		}
	}
	else
	{
		// 親を解除
		m_Transform->SetParent(nullptr);
	}
}

/*====================================================================
	親GameObjectを取得する
	TransFormの親から親GameObjectを取得する。
	
	戻り値: 親GameObject（親がいない場合はnullptr）
====================================================================*/
GameObject* GameObject::GetParent() const
{
	if (!m_Transform)
	{
		return nullptr;
	}
	
	TransForm* parentTransform = m_Transform->GetParent();
	if (!parentTransform)
	{
		return nullptr;
	}
	
	return parentTransform->GetGameObject();
}

/*====================================================================
	子GameObjectを取得する（複数）
	TransFormの子から子GameObjectのリストを取得する。
	
	戻り値: 子GameObjectのリスト
====================================================================*/
std::vector<GameObject*> GameObject::GetChildren() const
{
	std::vector<GameObject*> children;
	
	if (!m_Transform)
	{
		return children;
	}
	
	const std::vector<TransForm*>& childTransforms = m_Transform->GetChildren();
	children.reserve(childTransforms.size());
	
	for (TransForm* childTransform : childTransforms)
	{
		if (childTransform)
		{
			GameObject* childGameObject = childTransform->GetGameObject();
			if (childGameObject)
			{
				children.push_back(childGameObject);
			}
		}
	}
	
	return children;
}

/*====================================================================
	子GameObjectの数を取得する
	TransFormの子の数を取得する。
	
	戻り値: 子GameObjectの数
====================================================================*/
size_t GameObject::GetChildCount() const
{
	if (!m_Transform)
	{
		return 0;
	}
	
	return m_Transform->GetChildCount();
}

/*====================================================================
	指定インデックスの子GameObjectを取得する
	TransFormの子から指定インデックスの子GameObjectを取得する。
	
	引数:
	  index - 子のインデックス（0から始まる）
	戻り値: 子GameObject（範囲外の場合はnullptr）
====================================================================*/
GameObject* GameObject::GetChild(size_t index) const
{
	if (!m_Transform)
	{
		return nullptr;
	}
	
	const std::vector<TransForm*>& childTransforms = m_Transform->GetChildren();
	
	if (index >= childTransforms.size())
	{
		return nullptr;
	}
	
	TransForm* childTransform = childTransforms[index];
	if (!childTransform)
	{
		return nullptr;
	}
	
	return childTransform->GetGameObject();
}

/*====================================================================
	タグを設定する
	GameObjectをグループ化するためのタグを設定する。
	Sceneのタグマップに自動的に登録される。
	
	引数:
	  tag - 設定するタグ文字列
====================================================================*/
void GameObject::SetTag(const std::wstring& tag)
{
	// 古いタグでの登録を解除
	if (m_Scene && !m_Tag.empty())
	{
		m_Scene->UnregisterGameObjectTag(this, m_Tag);
	}
	
	// 新しいタグを設定
	m_Tag = tag;
	
	// 新しいタグで登録
	if (m_Scene && !m_Tag.empty())
	{
		m_Scene->RegisterGameObjectTag(this, m_Tag);
	}
}

/*====================================================================
	タグを取得する
	
	戻り値: 現在設定されているタグ文字列
====================================================================*/
const std::wstring& GameObject::GetTag() const
{
	return m_Tag;
}

/*====================================================================
	指定されたタグを持つかチェックする
	
	引数:
	  tag - チェックするタグ文字列
	戻り値: 指定されたタグを持つ場合true
====================================================================*/
bool GameObject::CompareTag(const std::wstring& tag) const
{
	return m_Tag == tag;
}

/*====================================================================
	指定されたタグを持つGameObjectを検索する（単一）
	最初に見つかったGameObjectを返す。
	現在のアクティブシーンから検索する。
	
	引数:
	  tag - 検索するタグ文字列
	戻り値: 見つかったGameObject（見つからない場合はnullptr）
====================================================================*/
GameObject* GameObject::FindWithTag(const std::wstring& tag)
{
	Scene* activeScene = SceneManager::Instance().GetActiveScene();
	if (!activeScene)
	{
		return nullptr;
	}
	
	return activeScene->FindGameObjectWithTag(tag);
}

/*====================================================================
	指定されたタグを持つすべてのGameObjectを検索する
	現在のアクティブシーンから検索する。
	
	引数:
	  tag - 検索するタグ文字列
	戻り値: 見つかったGameObjectのリスト
====================================================================*/
std::vector<GameObject*> GameObject::FindGameObjectsWithTag(const std::wstring& tag)
{
	Scene* activeScene = SceneManager::Instance().GetActiveScene();
	if (!activeScene)
	{
		return std::vector<GameObject*>();
	}
	
	return activeScene->FindGameObjectsWithTag(tag);
}

/*====================================================================
	指定された名前を持つGameObjectを検索する
	現在のアクティブシーンから検索する。
	
	引数:
	  name - 検索する名前
	戻り値: 見つかったGameObject（見つからない場合はnullptr）
====================================================================*/
GameObject* GameObject::Find(const std::wstring& name)
{
	Scene* activeScene = SceneManager::Instance().GetActiveScene();
	if (!activeScene)
	{
		return nullptr;
	}
	
	return activeScene->FindGameObjectByName(name);
}
/*====================================================================
	Instantiate - GameObjectを複製して生成する
	現在のアクティブシーンに登録し、生ポインタを返す。
	
	処理の流れ:
	1. 新しいGameObjectを作成（コンストラクタで自動的にSceneに登録される）
	2. 基本的なメンバ変数をコピー
	3. TransFormの内容をコピー
	4. 全てのComponentをInstantiate()して複製
	5. 子GameObjectを再帰的にInstantiate()
	
	引数:
	  original - 複製元のGameObject
	戻り値: 複製されたGameObjectの生ポインタ
	例外: originalがnullptrの場合はruntime_errorをスロー
====================================================================*/
GameObject* GameObject::Instantiate(GameObject* original)
{
	if (!original)
	{
		throw std::runtime_error("GameObject::Instantiate: original is null");
	}

	// 新しいGameObjectを作成（コンストラクタで自動的にTransFormを作成し、Sceneに登録される）
	GameObject* cloned = new GameObject();
	
	// 基本的なメンバ変数をコピー（アドレスではなく値をコピー）
	cloned->m_Active = original->m_Active;
	cloned->m_Destroy = false;
	cloned->m_ActiveSelf = original->m_ActiveSelf;
	cloned->m_Name = original->m_Name;
	
	// TransFormの内容をコピー（階層構造は後で処理）
	if (original->m_Transform && cloned->m_Transform)
	{
		cloned->m_Transform->Position() = original->m_Transform->Position();
		cloned->m_Transform->Rotation() = original->m_Transform->Rotation();
		cloned->m_Transform->Scale() = original->m_Transform->Scale();
	}
	
	// 全てのComponentを複製
	for (const auto& comp : original->m_Components)
	{
		// Component::Instantiate()を呼び出して複製
		Component* clonedComp = Component::Instantiate(comp.get());
		
		if (clonedComp)
		{
			// オーナーを設定
			clonedComp->m_Owner = cloned;
			
			// コンポーネントリストに追加（unique_ptrでラップ）
			cloned->m_Components.emplace_back(std::unique_ptr<Component>(clonedComp));
			
			// Sceneに登録
			if (cloned->m_Scene)
			{
				cloned->m_Scene->RegisterComponent(clonedComp, cloned);
			}
			
			// ライフサイクルフラグをリセット（新しいコンポーネントなので再度Awake/Startが必要）
			clonedComp->m_IsAwakeCalled = false;
			clonedComp->m_IsStartCalled = false;
			
			// Awakeを呼び出し、GameLoopに登録
			if (!clonedComp->m_IsAwakeCalled)
			{
				clonedComp->Awake();
				clonedComp->m_IsAwakeCalled = true;
				GameLoop::Instance().RegisterAwakeComponent(clonedComp);
			}
		}
	}
	
	// 子GameObjectを再帰的に複製
	if (original->m_Transform)
	{
		const std::vector<TransForm*>& children = original->m_Transform->GetChildren();
		
		for (TransForm* childTransform : children)
		{
			GameObject* childGameObject = childTransform->GetGameObject();
			
			if (childGameObject)
			{
				// 子GameObjectを複製
				GameObject* clonedChild = Instantiate(childGameObject);
				
				if (clonedChild && clonedChild->m_Transform)
				{
					// 親子関係を設定
					clonedChild->m_Transform->SetParent(cloned->m_Transform.get());
				}
			}
		}
	}
	
	return cloned;
}

/*====================================================================
	Destroy - GameObjectを削除する
	削除フラグを立てる。
	
	引数:
	  obj - 削除するGameObject
	例外: objがnullptrの場合はruntime_errorをスロー
====================================================================*/
void GameObject::Destroy(GameObject* obj)
{
	if (!obj)
	{
		throw std::runtime_error("GameObject::Destroy: obj is null");
	}

	for (auto child : obj->GetChildren())
	{
		GameObject::Destroy(child);
	}

	// 削除フラグを立てる
	obj->m_Destroy = true;
}
