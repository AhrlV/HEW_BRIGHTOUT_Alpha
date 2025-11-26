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
	Objectを継承し、ライフサイクルメソッドを提供する。
====================================================================*/
class Component : public Object
{
protected:
	// このComponentが所属するGameObjectへのポインタ
	GameObject* m_Owner;

public:
	// コンストラクタ
	Component();

	// デストラクタ
	virtual ~Component();

	// Unityのようなライフサイクルメソッド
	virtual void Awake() {}
	virtual void Start() {}
	virtual void Update() {}
	virtual void LateUpdate() {}
	virtual void FixedUpdate() {}

	// 所属するGameObjectを取得するメソッド
	GameObject* GetGameObject() const;
	const GameObject* GetGameObjectConst() const;

	void SetGameObject(GameObject* owner);

	// 所属するSceneを取得するメソッド
	Scene* GetScene() const;

	// GameObject/PhysicsSystem/Sceneから内部にアクセスできるようにする
	friend class GameObject;
	friend class Scene;
	friend class PhysicsSystem;
};

#endif // COMPONENT_H
