/*====================================================================

	SceneManager 関数群 (Scene/GameObject作成/破棄管理) [scene_manager.h]
	シングルトン的にアクティブ Scene を保持し、
	GameObject/Component の登録 API を提供する。

	Author : Ryosuke Kageyama
	Date   : 2025/11/26

====================================================================*/

#ifndef SCENE_MANAGER_H
#define SCENE_MANAGER_H

#include <memory>
#include <type_traits>

class Scene;
class GameObject;

/*====================================================================
	SceneManagerクラス
	グローバルなScene管理とGameObjectの登録を行うシングルトンクラス。
====================================================================*/
class SceneManager
{
private:
	// アクティブなScene
	std::unique_ptr<Scene> m_ActiveScene;

	// プライベートコンストラクタ（シングルトン）
	SceneManager() = default;

	// デストラクタ
	~SceneManager() = default;

public:
	// コピー・ムーブを禁止
	SceneManager(const SceneManager&) = delete;
	SceneManager& operator=(const SceneManager&) = delete;
	SceneManager(SceneManager&&) = delete;
	SceneManager& operator=(SceneManager&&) = delete;

	/*====================================================================
		シングルトンインスタンスを取得する
		戻り値: SceneManagerの唯一のインスタンスへの参照
	====================================================================*/
	static SceneManager& Instance()
	{
		static SceneManager instance;
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
		アクティブなSceneのリソースを読み込む
		現在アクティブなSceneのResourceLoadメソッドを呼び出す。
	====================================================================*/
	void ResourceLoadScene();

	/*====================================================================
		アクティブなSceneを初期化する
		現在アクティブなSceneのInitializeメソッドを呼び出す。
	====================================================================*/
	void InitializeScene();
	
	/*====================================================================
		アクティブなSceneを終了する
		現在アクティブなSceneのFinalizeメソッドを呼び出す。
	====================================================================*/
	void FinalizeScene();

	/*====================================================================
		Sceneを変更する
		現在のSceneを終了処理してから、型Tの新しいSceneを生成して設定し、初期化を行う。
		
		テンプレート引数:
		  T - 変更するSceneの型
	====================================================================*/
	template <typename T>
	requires std::is_base_of<Scene, T>::value
	void ChangeScene()
	{
		// 現在のSceneが存在する場合は終了処理を実行
		FinalizeScene();

		// 新しいSceneを設定して初期化
		SetScene<T>();
		ResourceLoadScene();
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
};

#endif // SCENE_MANAGER_H
