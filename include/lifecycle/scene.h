/*====================================================================


	Sceneクラス [scene.h]
	ゲーム内の全 GameObject / Component を管理し、
	型別検索や動的追加登録を提供する。

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

// 前方宣言
class GameObject;
class Component;

/*====================================================================
	Sceneクラス
	ゲーム内の全GameObjectとComponentを管理する。
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

	// 内部登録解除メソッド
	void UnregisterGameObject(GameObject* go);

	// 破棄フラグが立ったオブジェクトを削除する
	void CleanupDestroyedObjects();

public:
	// コンストラクタ・デストラクタ
	Scene();
	virtual ~Scene();



	// リソース読み込み
	virtual void ResourceLoad() {};
	// 初期化
	virtual void Initialize() {};
	


	// 終了処理
	// シーン切り替え時に呼び出され、RenderSystemやResourceManagerなどの中身を解放する
	virtual void Finalize();

	// GameObject登録（Sceneが所有権を取得）
	void RegisterGameObject(GameObject* go);

	// Component登録・解除
	void RegisterComponent(Component* comp, GameObject* owner);
	void UnregisterComponent(Component* comp);


	// 破棄処理を実行
	void ProcessCleanup();

	// 描画処理
	void Render();

	// 取得系
	GameObject* GetGameObjectById(uint64_t id) const;
	Component* GetComponentById(uint64_t id) const;

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
