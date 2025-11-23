#ifndef COMPONENT_H
#define COMPONENT_H
/*====================================================================


	Component [component.h]

									Author : Ryosuke Kageyama
									Date   : 2025/11/18
====================================================================*/

class GameObject;
class PhysicsSystem; // 物理呼び出し制限用

class Component
{
protected:
	// このComponentを所有するGameObjectへのポインタ
	GameObject* m_Owner = nullptr;

	// このComponent自体の有効/無効（GameObjectとは独立したフラグ）
	bool m_Active = true;

public:
	Component() = default;
	virtual ~Component() = default;

	// Unityのようなライフサイクルメソッド
	virtual void Awake() {}
	virtual void Start() {}
	virtual void Update() {}
	virtual void LateUpdate() {}
	// 固定更新と描画
	virtual void FixedUpdate() {}
	virtual void Render() {}

public:
	// 有効/無効の設定・取得（Component 自身のフラグ）
	inline void SetActive(bool active) { m_Active = active; }
	inline bool IsActive() const { return m_Active; }

	// 所有するGameObjectを取得するメソッド
	const GameObject* GetGameObject() const { return m_Owner; }

	// GameObject/PhysicsSystemが内部にアクセスできるように
	friend class GameObject;
};

#endif
