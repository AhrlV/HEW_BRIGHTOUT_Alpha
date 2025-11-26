/*====================================================================

	World 関数群 (Scene/GameObject作成/破棄管理) [world.h]
	シングルトン的にアクティブ Scene を保持し、
	GameObject/Component の登録 API を提供する。

	Author : Ryosuke Kageyama
	Date   : 2025/11/26

====================================================================*/

#ifndef WORLD_H
#define WORLD_H

#include <memory>
#include <type_traits>

class Scene;
class GameObject;
class Component;

/*====================================================================
	Worldクラス
	グローバルなScene管理とGameObjectの登録を行うシングルトンクラス。
====================================================================*/
class World
{
private:
	// アクティブなScene
	std::unique_ptr<Scene> m_ActiveScene;

	// プライベートコンストラクタ（シングルトン）
	World() = default;

	// デストラクタ
	~World() = default;

public:
	// コピー・ムーブを禁止
	World(const World&) = delete;
	World& operator=(const World&) = delete;
	World(World&&) = delete;
	World& operator=(World&&) = delete;

	/*====================================================================
		シングルトンインスタンスを取得する
		戻り値: Worldの唯一のインスタンスへの参照
	====================================================================*/
	static World& Instance()
	{
		static World instance;
		return instance;
	}

	/*====================================================================
		Sceneを設定する
		型Tの新しいSceneを生成してアクティブなSceneに設定する。
		初期化は行わない。
		
		テンプレート引数:
		  T - 設定するSceneの型
	====================================================================*/
	template <typename T>
	requires std::is_base_of<Scene, T>::value
	void SetScene()
	{
		m_ActiveScene = std::make_unique<T>();
	}

	/*====================================================================
		アクティブなSceneを初期化する
		現在アクティブなSceneのInitializeメソッドを呼び出す。
	====================================================================*/
	void InitializeScene();

	/*====================================================================
		Sceneを変更する
		型Tの新しいSceneを生成して設定し、初期化を行う。
		
		テンプレート引数:
		  T - 変更するSceneの型
	====================================================================*/
	template <typename T>
	requires std::is_base_of<Scene, T>::value
	void ChangeScene()
	{
		SetScene<T>();
		InitializeScene();
	}

	/*====================================================================
		アクティブなSceneを取得する
		戻り値: 現在アクティブなSceneのポインタ
	====================================================================*/
	Scene* GetActiveScene();

	/*====================================================================
		GameObjectを登録する
		引数:
		  go - 登録するGameObject
		戻り値: 登録されたGameObjectのポインタ
		例外: アクティブなSceneが存在しない場合はruntime_errorをスロー
	====================================================================*/
	GameObject* RegisterGameObject(GameObject* go);

	/*====================================================================
		ゲームループのティック処理
		物理更新、フレーム更新、描画を実行する。
	====================================================================*/
	void Tick();
};

#endif // WORLD_H
