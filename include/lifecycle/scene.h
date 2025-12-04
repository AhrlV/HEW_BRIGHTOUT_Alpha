/*====================================================================


	Sceneクラス [scene.h]
	ゲーム内の全 GameObject / Component を管理し、
	型別検索や動的追加登録を提供する。
	タグと名前によるGameObjectの検索機能をサポートする。

	Author : Ryosuke Kageyama
	Date   : 2025/11/26

====================================================================*/

#ifndef LIFECYCLE_SCENE_H
#define LIFECYCLE_SCENE_H

#include <vector>
#include <memory>
#include <unordered_map>
#include <typeindex>
#include <cstdint>
#include <string>

// 前方宣言
class GameObject;
class Component;

/*====================================================================
	Sceneクラス
	ゲーム内の全GameObjectとComponentを管理する。
	タグと名前によるGameObjectの検索機能をサポートする。
====================================================================*/
class Scene 
{
private:
	// 所有するGameObjectのvector
	std::vector<std::unique_ptr<GameObject>> m_GameObjects;

	// ID管理
	uint64_t m_NextGameObjectId;
	uint64_t m_NextComponentId;
	std::unordered_map<uint64_t, GameObject*> m_GameObjectMap;
	std::unordered_map<uint64_t, Component*> m_ComponentMap;

	// 型別のコンポーネント配列
	std::unordered_map<std::type_index, std::vector<Component*>> m_ComponentTypeMap;

	/*====================================================================
		タグと名前によるGameObjectの検索マップ
	====================================================================*/
	
	// タグとGameObjectのマルチマップ（同じタグを持つGameObjectが複数存在可能）
	std::unordered_multimap<std::wstring, GameObject*> m_TagMap;
	
	// 名前とGameObjectのマップ（名前は一意であることを想定）
	std::unordered_map<std::wstring, GameObject*> m_NameMap;

	// 削除登録される内部メソッド
	void UnregisterGameObject(GameObject* go);

	// 破壊フラグが立っているオブジェクトを削除する
	void CleanupDestroyedObjects();

public:
	/*====================================================================
		コンストラクタとデストラクタ
	====================================================================*/
	
	// コンストラクタ
	Scene();
	
	// デストラクタ
	virtual ~Scene();

	/*====================================================================
		ライフサイクルメソッド
	====================================================================*/
	
	// リソース読み込み
	virtual void ResourceLoad() {};
	
	// 初期化
	virtual void Initialize() {};
	
	// 終了処理
	// シーン切り替え時に呼び出され、RenderSystemやResourceManagerなどの中身を破棄する
	virtual void Finalize();

	/*====================================================================
		GameObjectとComponentの管理
	====================================================================*/
	
	// GameObject登録（Sceneが所有権を取得）
	void RegisterGameObject(GameObject* go);

	// Component登録・解除
	void RegisterComponent(Component* comp, GameObject* owner);
	void UnregisterComponent(Component* comp);

	// 破壊処理の実行
	void ProcessCleanup();

	// 描画処理
	void Render();

	/*====================================================================
		GameObjectの検索（ID/タグ/名前）
	====================================================================*/
	
	// IDによる検索
	GameObject* GetGameObjectById(uint64_t id) const;
	Component* GetComponentById(uint64_t id) const;

	/*====================================================================
		タグによるGameObjectの検索
		
		引数:
		  tag - 検索するタグ文字列
		戻り値: 見つかったGameObject（見つからない場合はnullptr）
	====================================================================*/
	GameObject* FindGameObjectWithTag(const std::wstring& tag) const;
	
	/*====================================================================
		タグによるすべてのGameObjectの検索
		
		引数:
		  tag - 検索するタグ文字列
		戻り値: 見つかったGameObjectのリスト
	====================================================================*/
	std::vector<GameObject*> FindGameObjectsWithTag(const std::wstring& tag) const;
	
	/*====================================================================
		名前によるGameObjectの検索
		
		引数:
		  name - 検索する名前
		戻り値: 見つかったGameObject（見つからない場合はnullptr）
	====================================================================*/
	GameObject* FindGameObjectByName(const std::wstring& name) const;

	/*====================================================================
		タグと名前のマップ管理（内部用）
	====================================================================*/
	
	/*====================================================================
		タグマップにGameObjectを登録する
		
		引数:
		  go - 登録するGameObject
		  tag - 登録するタグ
	====================================================================*/
	void RegisterGameObjectTag(GameObject* go, const std::wstring& tag);
	
	/*====================================================================
		タグマップからGameObjectを解除する
		
		引数:
		  go - 解除するGameObject
		  oldTag - 解除する古いタグ
	====================================================================*/
	void UnregisterGameObjectTag(GameObject* go, const std::wstring& oldTag);
	
	/*====================================================================
		名前マップにGameObjectを登録する
		
		引数:
		  go - 登録するGameObject
		  name - 登録する名前
	====================================================================*/
	void RegisterGameObjectName(GameObject* go, const std::wstring& name);
	
	/*====================================================================
		名前マップからGameObjectを解除する
		
		引数:
		  go - 解除するGameObject
		  oldName - 解除する古い名前
	====================================================================*/
	void UnregisterGameObjectName(GameObject* go, const std::wstring& oldName);

	/*====================================================================
		Componentの型別取得
	====================================================================*/
	
	// 全Component取得（GameLoopから使用）
	const std::unordered_map<uint64_t, Component*>& GetAllComponents() const
	{
		return m_ComponentMap;
	}

	// 型別Component取得テンプレート
	template<typename T>
	std::vector<T*> GetComponentsByType() const
	{
		std::vector<T*> out;
		auto it = m_ComponentTypeMap.find(std::type_index(typeid(T)));
		if(it!=m_ComponentTypeMap.end())
		{
			for(Component* c : it->second)
			{
				if(auto casted = dynamic_cast<T*>(c))
				{
					out.push_back(casted);
				}
			}
		}
		return out;
	}

	friend class GameObject;
	friend class GameLoop;
};

#endif // LIFECYCLE_SCENE_H
