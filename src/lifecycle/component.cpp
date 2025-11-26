/*====================================================================

	Component実装 [component.cpp]
	Componentクラスの実装。

	Author : Ryosuke Kageyama
	Date   : 2025/11/26

====================================================================*/

#include "lifecycle/component.h"
#include "lifecycle/gameobject.h"
#include "lifecycle/scene.h"

/*====================================================================
	コンストラクタ
	メンバ変数を初期化する。
====================================================================*/
Component::Component()
	: Object()
	, m_Owner(nullptr)
{
}

/*====================================================================
	デストラクタ
	リソースを解放する。
====================================================================*/
Component::~Component()
{
}

/*====================================================================
	所属するGameObjectを取得する
	戻り値: このComponentが所属するGameObjectのポインタ
====================================================================*/
GameObject* Component::GetGameObject() const
{
	return m_Owner;
}

/*====================================================================
	所属するGameObjectを取得する（const版）
	戻り値: このComponentが所属するGameObjectのconstポインタ
====================================================================*/
const GameObject* Component::GetGameObjectConst() const
{
	return m_Owner;
}

/*====================================================================
	所属するGameObjectを変更する
====================================================================*/
void Component::SetGameObject(GameObject* owner)
{
	m_Owner = owner;
}

/*====================================================================
	所属するSceneを取得する
	GameObjectを経由してSceneを取得する。
	戻り値: このComponentが所属するSceneのポインタ
====================================================================*/
Scene* Component::GetScene() const
{
	if (m_Owner)
	{
		return m_Owner->GetScene();
	}
	return nullptr;
}
