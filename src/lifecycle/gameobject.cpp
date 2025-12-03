/*====================================================================


    GameObject 実装 [gameobject.cpp]
	GameObjectクラスの実装。

                                    Author : Ryosuke Kageyama
                                    Date   : 2025/11/26
====================================================================*/

#include "lifecycle/gameobject.h"
#include "physics/transform.h"
#include "lifecycle/scene_manager.h"

/*====================================================================
	コンストラクタ
	GameObjectを初期化し、必須のTransFormコンポーネントを追加する。
	
	処理順序:
	1. まずAddComponent<TransForm>()でTransFormを追加
	2. AddComponent内でAwakeが即座に呼ばれる
	3. SceneManager::Instance().RegisterGameObject()でSceneに登録
	4. Scene::RegisterGameObject()でm_Sceneが設定される
====================================================================*/
GameObject::GameObject()
	: Object()
	, m_Scene(nullptr)
	, m_ActiveSelf(true)
{
	// 必須TransFormを追加
	// この時点ではm_Sceneはnullだが、AddComponent内でAwakeが呼ばれる
	AddComponent<TransForm>();
	
	// Sceneに登録
	// これでm_Sceneが設定され、m_NewlyCreatedに追加される
	SceneManager::Instance().RegisterGameObject(this);
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
