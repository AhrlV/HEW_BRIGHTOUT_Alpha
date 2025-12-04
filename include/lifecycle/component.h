/*====================================================================

	Component [component.h]
	GameObjectに付与される機能の基底クラス。
	Objectクラスを継承し、アクティブフラグとIDを持つ。

	Author : Ryosuke Kageyama
	Date   : 2025/11/26

====================================================================*/

#ifndef COMPONENT_H
#define COMPONENT_H

#include "lifecycle/object.h"


class GameObject;
class PhysicsSystem;
class Scene;

/*====================================================================
	Componentクラス
	GameObjectに付与される機能の基底クラス。
	Objectを継承し、ライフサイクル用メソッドを提供する。
====================================================================*/
class Component : public Object
{
protected:
	// このComponentが所属するGameObjectへのポインタ
	GameObject* m_Owner;

	// ライフサイクルフラグ
	bool m_IsAwakeCalled;
	bool m_IsStartCalled;

	// Unityのようなライフサイクル用メソッド
	virtual void Awake() {}
	virtual void Start() {}
	virtual void Update() {}
	virtual void LateUpdate() {}
	virtual void FixedUpdate() {}

	void SetGameObject(GameObject* owner);

	// 所属するSceneを取得するメソッド
	Scene* GetScene() const;

	// ライフサイクルフラグの取得
	bool IsAwakeCalled() const { return m_IsAwakeCalled; }
	bool IsStartCalled() const { return m_IsStartCalled; }

public:

	// コンストラクタ
	Component();
	// デストラクタ
	virtual ~Component();

	// 所属するGameObjectを取得するメソッド
	GameObject* GetGameObject() const;
	const GameObject* GetGameObjectConst() const;

	/*====================================================================
		Instantiate - Componentを複製して生成する
		現在のアクティブシーンに登録し、生ポインタを返す。
		
		引数:
		  original - 複製元のComponent
		戻り値: 複製されたComponentの生ポインタ
		例外: originalがnullptrの場合はruntime_errorをスロー
	====================================================================*/
	static Component* Instantiate(Component* original);

	/*====================================================================
		Destroy - Componentを削除する
		削除フラグを立てる。
		
		引数:
		  obj - 削除するComponent
		例外: objがnullptrの場合はruntime_errorをスロー
	====================================================================*/
	static void Destroy(Component* obj);

	// GameObject/GameLoopから内部にアクセスできるようにする
	friend class GameObject;
	friend class GameLoop;
};

#endif // COMPONENT_H
