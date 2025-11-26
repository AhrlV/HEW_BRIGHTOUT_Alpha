/*====================================================================


    GameObject 実装 [gameobject.cpp]
	GameObjectクラスの実装。

                                    Author : Ryosuke Kageyama
                                    Date   : 2025/11/26
====================================================================*/

#include "lifecycle/gameobject.h"
#include "physics/transform.h"
#include "lifecycle/world.h"

/*====================================================================
	コンストラクタ
	GameObjectを初期化し、必須のTransFormコンポーネントを追加する。
	
	処理順序:
	1. まずAddComponent<TransForm>()でTransFormを追加（m_Sceneはまだnull）
	2. World::Instance().RegisterGameObject()でSceneに登録
	3. Scene::RegisterGameObject()でm_Sceneが設定される
	4. その後、ProcessAwake()でTransFormのRegisterComponentとAwakeが呼ばれる
====================================================================*/
GameObject::GameObject()
	: Object()
	, m_Scene(nullptr)
	, m_ActiveSelf(true)
{
	// 必須TransFormを追加
	// この時点ではm_Sceneはnullだが、m_AddedComponentsに追加される
	AddComponent<TransForm>();
	
	// Sceneに登録
	// これでm_Sceneが設定され、m_NewlyCreatedに追加される
	World::Instance().RegisterGameObject(this);
}


/*====================================================================
	所属するSceneを取得する
	
	戻り値: このGameObjectが属するSceneのポインタ
====================================================================*/
Scene* GameObject::GetScene() const
{
	return m_Scene;
}

/*====================================================================
	自身のアクティブ状態を設定する
	
	引数:
	  active - 設定するアクティブ状態
====================================================================*/
void GameObject::SetActiveSelf(bool active)
{
	m_ActiveSelf = active;
	Object::SetActive(active);
}

/*====================================================================
	自身のアクティブ状態を取得する
	
	戻り値: 自身がアクティブな場合true
====================================================================*/
bool GameObject::IsActiveSelf() const
{
	return m_ActiveSelf;
}

/*====================================================================
	階層内でのアクティブ状態を取得する
	
	戻り値: 階層内でアクティブな場合true
	注: 現在は親階層を考慮していない
====================================================================*/
bool GameObject::IsActiveInHierarchy() const
{
	return m_ActiveSelf;
}

/*====================================================================
	新規追加されたComponentのAwakeを呼び出す
	追加されたばかりのコンポーネントのAwakeメソッドを実行する。
====================================================================*/
void GameObject::AwakeNewComponents()
{
	for (auto* c : m_AddedComponents)
	{
		c->Awake();
	}
	m_AddedComponents.clear();
}

/*====================================================================
	新規追加されたComponentのStartを呼び出す
	まだStartが呼ばれていないアクティブなコンポーネントのStartメソッドを実行する。
====================================================================*/
void GameObject::StartNewComponents()
{
	for (auto& c : m_Components)
	{
		if (m_StartedComponents.find(c.get()) == m_StartedComponents.end())
		{
			if (IsActiveInHierarchy() && c->IsActive())
			{
				c->Start();
				m_StartedComponents.insert(c.get());
			}
		}
	}
}

/*====================================================================
	毎フレーム更新
	アクティブな全てのコンポーネントのUpdateメソッドを呼び出す。
====================================================================*/
void GameObject::Update()
{
	if (!IsActiveInHierarchy())
	{
		return;
	}

	for (auto& c : m_Components)
	{
		if (c->IsActive())
		{
			c->Update();
		}
	}
}

/*====================================================================
	後段更新
	アクティブな全てのコンポーネントのLateUpdateメソッドを呼び出す。
====================================================================*/
void GameObject::LateUpdate()
{
	if (!IsActiveInHierarchy())
	{
		return;
	}

	for (auto& c : m_Components)
	{
		if (c->IsActive())
		{
			c->LateUpdate();
		}
	}
}

/*====================================================================
	固定更新
	アクティブな全てのコンポーネントのFixedUpdateメソッドを呼び出す。
====================================================================*/
void GameObject::FixedUpdate()
{
	if (!IsActiveInHierarchy())
	{
		return;
	}

	for (auto& c : m_Components)
	{
		if (c->IsActive())
		{
			c->FixedUpdate();
		}
	}
}
