/*====================================================================

	GameObjectクラス [gameobject.h]
	ゲーム内のエンティティ。Componentのコンテナとして機能。
	Objectクラスを継承し、IDとアクティブフラグを持つ。
	自身が属するSceneへの参照を保持する。

	Author : Ryosuke Kageyama
	Date   : 2025/11/26

====================================================================*/

#ifndef GAMEOBJECT_H
#define GAMEOBJECT_H

#include "lifecycle/scene.h"
#include "lifecycle/gameloop.h"
#include "lifecycle/object.h"
#include "component.h"
#include <vector>
#include <memory>
#include <type_traits>

// 前方宣言
class Scene;

/*====================================================================
	GameObjectクラス
	ゲーム内のエンティティ。
	複数のComponentを持ち、シーンの階層構造の一部となる。
====================================================================*/
class GameObject : public Object
{
private:
	// 自身が属しているScene
	Scene* m_Scene;

	// 所有するコンポーネント
	std::vector<std::unique_ptr<Component>> m_Components;

	// 追加されたばかりの未登録コンポーネント
	std::vector<Component*> m_AddedComponents;

	// 自身のアクティブフラグ
	bool m_ActiveSelf;


public:

	// コンストラクタ
	GameObject();
	// デストラクタ
	virtual ~GameObject() = default;

	// 所属するSceneを取得
	Scene* GetScene() const;

	// アクティブ状態の設定・取得
	void SetActiveSelf(bool active);
	bool IsActiveSelf() const;
	bool IsActiveInHierarchy() const;

	// Component追加メソッド（可変引数テンプレート）
	template <typename T, typename... Args>
	requires std::is_base_of<Component, T>::value
	T* AddComponent(Args&&... args);

	// Component取得メソッド（単体）
	template <typename T>
	[[nodiscard]] T* GetComponent() const;

	// Component取得メソッド（複数）
	template <typename T>
	[[nodiscard]] std::vector<T*> GetComponents() const;

	friend class Scene;
	friend class GameLoop;
};

/*====================================================================
	Componentを追加する
	新しいComponentを作成し、GameObjectに追加する。
	即座にAwakeを呼び出し、GameLoopの未Startリストに登録する。
	
	テンプレート引数:
	  T - 追加するComponentの型
	  Args - Componentのコンストラクタ引数
	引数:
	  args - Componentのコンストラクタに渡す引数
	戻り値: 追加されたComponentのポインタ
====================================================================*/
template <typename T, typename... Args>
requires std::is_base_of<Component, T>::value
T* GameObject::AddComponent(Args&&... args)
{
	// Componentを作成
	auto comp = std::make_unique<T>(std::forward<Args>(args)...);
	T* raw = comp.get();

	// オーナーを設定
	raw->m_Owner = this;

	// コンポーネントリストに追加
	m_Components.emplace_back(std::move(comp));

	// 新規追加リストに追加（Scene登録待ち）
	m_AddedComponents.push_back(raw);

	// Sceneに登録済みの場合は、Sceneのコンポーネントマップに登録
	if(m_Scene)
	{
		m_Scene->RegisterComponent(raw, this);
	}

	// 即座にAwakeを呼び出す
	if (!raw->m_IsAwakeCalled)
	{
		raw->Awake();
		raw->m_IsAwakeCalled = true;
		
		// GameLoopの未Startリストに登録
		GameLoop::Instance().RegisterAwakeComponent(raw);
	}

	return raw;
}

/*====================================================================
	指定した型のComponentを取得する（単体）
	最初に見つかったComponentを返す。
	
	テンプレート引数:
	  T - 取得するComponentの型
	戻り値: 見つかったComponent（存在しない場合はnullptr）
====================================================================*/
template <typename T>
[[nodiscard]] T* GameObject::GetComponent() const
{
	for (auto& c : m_Components)
	{
		if (auto casted = dynamic_cast<T*>(c.get()))
		{
			return casted;
		}
	}
	return nullptr;
}

/*====================================================================
	指定した型のComponentを取得する（複数）
	該当する全てのComponentを返す。
	
	テンプレート引数:
	  T - 取得するComponentの型
	戻り値: 見つかったComponentのリスト
====================================================================*/
template <typename T>
[[nodiscard]] std::vector<T*> GameObject::GetComponents() const
{
	std::vector<T*> results;
	for (auto& c : m_Components)
	{
		if (auto casted = dynamic_cast<T*>(c.get()))
		{
			results.push_back(casted);
		}
	}
	return results;
}

#endif // GAMEOBJECT_H
