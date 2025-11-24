#ifndef WORLD_H
#define WORLD_H
/*====================================================================

	World 関数群 (SceneとGameObject生成/破壊管理) [world.h]

	Author : Ryosuke Kageyama
	Date   : 2025/11/19
====================================================================*/

#include <memory>
#include <type_traits>

// 前方宣言
class Scene;
class GameObject;

namespace World
{
	// 既存シーンを登録
	void SetScene(std::unique_ptr<Scene> scene);

	// シーン切替（生成して登録）
	template <typename T>
	void ChangeScene()
	{
		static_assert(std::is_base_of<Scene, T>::value, "Sceneを継承した型を指定してください");
		auto scn = std::make_unique<T>();
		SetScene(std::move(scn));
	}

	// アクティブシーン取得
	const Scene* GetActiveScene();

	// GameObject登録/破壊API
	GameObject* RegisterGameObject(GameObject* go);
	void DestroyGameObject(GameObject* go);

	// ライフサイクル更新
	void Tick();
}

#endif
