/*====================================================================

	Component実装 [component.cpp]
	Componentクラスの実装。

	Author : Ryosuke Kageyama
	Date   : 2025/11/26

====================================================================*/

#include "lifecycle/component.h"
#include "lifecycle/gameobject.h"
#include "lifecycle/scene.h"
#include <stdexcept>

/*====================================================================
	コンストラクタ
	メンバ変数を初期化する。
====================================================================*/
Component::Component()
	: Object()
	, m_Owner(nullptr)
	, m_IsAwakeCalled(false)
	, m_IsStartCalled(false)
{
	// ComponentのクラスIDを設定
	m_ClassID = ResourceClassID::Component;
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

/*====================================================================
	Instantiate - Componentを複製して生成する
	現在のアクティブシーンに登録し、生ポインタを返す。
	
	注意: この基底クラスのメソッドは基本的な複製のみを行う。
	派生クラスで固有のメンバ変数がある場合は、
	派生クラスでこのメソッドをオーバーライドする必要がある。
	
	引数:
	  original - 複製元のComponent
	戻り値: 複製されたComponentの生ポインタ
	例外: originalがnullptrの場合はruntime_errorをスロー
====================================================================*/
Component* Component::Instantiate(Component* original)
{
	if (!original)
	{
		throw std::runtime_error("Component::Instantiate: original is null");
	}

	// 新しいComponentを作成
	// 注意: これは基底クラスのComponentを作成するため、
	// 派生クラスで使用する場合は派生クラス側でオーバーライドが必要
	Component* cloned = new Component();
	
	// 基本的なメンバ変数をコピー（アドレスではなく値をコピー）
	cloned->m_Active = original->m_Active;
	cloned->m_Destroy = false;
	cloned->m_Owner = nullptr;  // オーナーは後で設定される
	cloned->m_IsAwakeCalled = false;
	cloned->m_IsStartCalled = false;
	cloned->m_Name = original->m_Name;
	
	return cloned;
}

/*====================================================================
	Destroy - Componentを削除する
	削除フラグを立てる。
	
	引数:
	  obj - 削除するComponent
	例外: objがnullptrの場合はruntime_errorをスロー
====================================================================*/
void Component::Destroy(Component* obj)
{
	if (!obj)
	{
		throw std::runtime_error("Component::Destroy: obj is null");
	}

	// 削除フラグを立てる
	obj->m_Destroy = true;
}
