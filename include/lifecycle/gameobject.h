/*====================================================================

	GameObjectクラス [gameobject.h]
	ゲーム内のエンティティ。Componentのコンテナとして機能。
	Objectクラスを継承し、IDとアクティブフラグを持つ。
	生成時に自動的にSceneへの参照を保持する。
	TransFormコンポーネントを必須メンバとして持つ。

	Author : Ryosuke Kageyama
	Date   : 2025/11/26

====================================================================*/

#ifndef GAMEOBJECT_H
#define GAMEOBJECT_H

#include "lifecycle/scene.h"
#include "lifecycle/gameloop.h"
#include "lifecycle/object.h"
#include "component.h"
#include "physics/transform.h"
#include "lifecycle/scene_manager.h"
#include <vector>
#include <memory>
#include <type_traits>

// 前方宣言
class Scene;
class TransForm;

/*====================================================================
	GameObjectクラス
	ゲーム内のエンティティ。
	複数のComponentを保持し、シーンの階層構造の一部となる。
	TransFormを必須メンバとして保持する。
====================================================================*/
class GameObject : public Object
{
private:
	// 所属しているScene
	Scene* m_Scene;

	// 必須のTransFormコンポーネント
	std::unique_ptr<TransForm> m_Transform;

	// 追加のコンポーネント（TransForm以外）
	std::vector<std::unique_ptr<Component>> m_Components;

	// 自身のアクティブフラグ
	bool m_ActiveSelf;

public:
	/*====================================================================
		コンストラクタとデストラクタ
	====================================================================*/
	
	// 通常のコンストラクタ（Sceneに自動登録される）
	GameObject();
	
	/*====================================================================
		Prefab用のコンストラクタ
		Prefabのテンプレートとして使用するGameObjectを作成する際に使用。
		skipSceneRegistration = trueの場合、Sceneへの自動登録をスキップする。
		
		引数:
		  skipSceneRegistration - trueの場合、Scene登録をスキップ
	====================================================================*/
	explicit GameObject(bool skipSceneRegistration);
	
	// デストラクタ
	virtual ~GameObject() = default;

	/*====================================================================
		基本的なプロパティ
	====================================================================*/
	
	// 所属するSceneを取得
	Scene* GetScene() const;

	// アクティブ状態の設定・取得
	void SetActiveSelf(bool active);
	bool IsActiveSelf() const;
	bool IsActiveInHierarchy() const;

	/*====================================================================
		親子関係の管理（TransFormベース）
	====================================================================*/
	
	/*====================================================================
		親GameObjectを設定する
		TransFormの親子関係を設定する。
		
		引数:
		  parent - 親となるGameObject（nullptrの場合は親を解除）
	====================================================================*/
	void SetParent(GameObject* parent);
	
	/*====================================================================
		親GameObjectを取得する
		TransFormの親から親GameObjectを取得する。
		
		戻り値: 親GameObject（親がいない場合はnullptr）
	====================================================================*/
	GameObject* GetParent() const;
	
	/*====================================================================
		子GameObjectを取得する（複数）
		TransFormの子から子GameObjectのリストを取得する。
		
		戻り値: 子GameObjectのリスト
	====================================================================*/
	std::vector<GameObject*> GetChildren() const;
	
	/*====================================================================
		子GameObjectの数を取得する
		TransFormの子の数を取得する。
		
		戻り値: 子GameObjectの数
	====================================================================*/
	size_t GetChildCount() const;
	
	/*====================================================================
		指定インデックスの子GameObjectを取得する
		TransFormの子から指定インデックスの子GameObjectを取得する。
		
		引数:
		  index - 子のインデックス（0から始まる）
		戻り値: 子GameObject（範囲外の場合はnullptr）
	====================================================================*/
	GameObject* GetChild(size_t index) const;

	/*====================================================================
		Component管理
	====================================================================*/
	
	// Component追加メソッド（可変長テンプレート）
	// TransFormは追加できない
	template <typename T, typename... Args>
	requires std::is_base_of<Component, T>::value && (!std::is_same<T, TransForm>::value)
	T* AddComponent(Args&&... args);

	// Component取得メソッド（単一）
	// TransFormは特別扱い
	template <typename T>
	[[nodiscard]] T* GetComponent() const;

	// Component取得メソッド（複数）
	template <typename T>
	[[nodiscard]] std::vector<T*> GetComponents() const;

	/*====================================================================
		静的メソッド
	====================================================================*/
	
	/*====================================================================
		Instantiate - GameObjectを複製して生成する
		現在のアクティブシーンに登録し、生ポインタを返す。
		
		引数:
		  original - 複製元のGameObject
		戻り値: 生成されたGameObjectの生ポインタ
		例外: originalがnullptrの場合はruntime_errorをスロー
	====================================================================*/
	static GameObject* Instantiate(GameObject* original);

	/*====================================================================
		Destroy - GameObjectを削除する
		削除フラグを立てる。
		
		引数:
		  obj - 削除するGameObject
		例外: objがnullptrの場合はruntime_errorをスロー
	====================================================================*/
	static void Destroy(GameObject* obj);

	friend class Scene;
	friend class GameLoop;
};

/*====================================================================
	Componentを追加する
	新しいComponentを作成し、GameObjectに追加する。
	作成後、Awakeを呼び出し、GameLoopの未Start用リストに登録する。
	制約: TransFormは追加できない（コンパイルエラー）
	
	テンプレート引数:
	  T - 追加するComponentの型
	  Args - Componentのコンストラクタ引数
	引数:
	  args - Componentのコンストラクタに渡す引数
	戻り値: 追加されたComponentのポインタ
====================================================================*/
template <typename T, typename... Args>
requires std::is_base_of<Component, T>::value && (!std::is_same<T, TransForm>::value)
T* GameObject::AddComponent(Args&&... args)
{
	// Componentを作成
	auto comp = std::make_unique<T>(std::forward<Args>(args)...);
	T* raw = comp.get();

	// オーナーを設定
	raw->m_Owner = this;

	// コンポーネントリストに追加
	m_Components.emplace_back(std::move(comp));


	// Sceneに登録済みの場合は、Sceneのコンポーネントマップに登録
	if (m_Scene)
	{
		m_Scene->RegisterComponent(raw, this);
	}

	// 作成後、Awakeを呼び出す
	if (!raw->m_IsAwakeCalled)
	{
		raw->Awake();
		raw->m_IsAwakeCalled = true;
		
		// GameLoopの未Start用リストに登録
		GameLoop::Instance().RegisterAwakeComponent(raw);
	}

	return raw;
}

/*====================================================================
	指定した型のComponentを取得する（単一）
	最初に見つかったComponentを返す。
	TransFormの場合は専用のメンバ変数から返す。
	
	テンプレート引数:
	  T - 取得するComponentの型
	戻り値: 見つかったComponent（存在しない場合はnullptr）
====================================================================*/
template <typename T>
[[nodiscard]] T* GameObject::GetComponent() const
{
	// TransFormの場合は専用のメンバ変数から返す
	if constexpr (std::is_same<T, TransForm>::value)
	{
		return m_Transform.get();
	}
	else
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
}

/*====================================================================
	指定した型のComponentを取得する（複数）
	該当する全てのComponentを返す。
	TransFormの場合は専用のメンバ変数から返す。
	
	テンプレート引数:
	  T - 取得するComponentの型
	戻り値: 見つかったComponentのリスト
====================================================================*/
template <typename T>
[[nodiscard]] std::vector<T*> GameObject::GetComponents() const
{
	std::vector<T*> results;
	
	// TransFormの場合は専用のメンバ変数から返す
	if constexpr (std::is_same<T, TransForm>::value)
	{
		if (m_Transform)
		{
			results.push_back(m_Transform.get());
		}
	}
	else
	{
		for (auto& c : m_Components)
		{
			if (auto casted = dynamic_cast<T*>(c.get()))
			{
				results.push_back(casted);
			}
		}
	}
	
	return results;
}

#endif // GAMEOBJECT_H
