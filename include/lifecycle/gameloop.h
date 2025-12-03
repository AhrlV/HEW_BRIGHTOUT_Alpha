/*====================================================================

	GameLoopクラス [gameloop.h]
	ゲームの更新ループを管理するシングルトンクラス。
	全てのComponentのライフサイクル（Awake/Start/Update/LateUpdate/FixedUpdate）を制御する。

	Author : Ryosuke Kageyama
	Date   : 2025/11/26

====================================================================*/

#ifndef GAMELOOP_H
#define GAMELOOP_H

#include <vector>

// 前方宣言
class Component;
class Scene;

/*====================================================================
	GameLoopクラス
	ゲームループを管理するシングルトンクラス。
	未StartなComponentを追跡し、適切なタイミングでStartを呼び出す。
====================================================================*/
class GameLoop
{
private:
	// 未Start状態のComponentリスト
	std::vector<Component*> m_PendingStartComponents;

	// プライベートコンストラクタ（シングルトン）
	GameLoop() = default;

	// デストラクタ
	~GameLoop() = default;

	// 内部処理メソッド
	void ProcessPhysics(Scene* scene);
	void ProcessStart(Scene* scene);
	void ProcessUpdate(Scene* scene);
	void ProcessLateUpdate(Scene* scene);
	void ProcessFixedUpdate(Scene* scene);
	void ProcessCleanup(Scene* scene);
	void ProcessRender(Scene* scene);

public:
	// コピー・ムーブを禁止
	GameLoop(const GameLoop&) = delete;
	GameLoop& operator=(const GameLoop&) = delete;
	GameLoop(GameLoop&&) = delete;
	GameLoop& operator=(GameLoop&&) = delete;

	/*====================================================================
		シングルトンインスタンスを取得する
		戻り値: GameLoopの唯一のインスタンスへの参照
	====================================================================*/
	static GameLoop& Instance()
	{
		static GameLoop instance;
		return instance;
	}

	/*====================================================================
		ComponentのAwake後に呼び出される
		未Startリストに追加する。
		
		引数:
		  comp - Awakeが呼ばれたComponent
	====================================================================*/
	void RegisterAwakeComponent(Component* comp);

	/*====================================================================
		ゲームループのメインティック処理
		物理更新、Awake、Start、Update、LateUpdate、破棄、描画を実行する。
		
		例外:
		  アクティブなSceneが存在しない場合はruntime_errorをスロー
	====================================================================*/
	void Tick();

	/*====================================================================
		未Startリストをクリアする
		Scene切り替え時などに呼び出す。
	====================================================================*/
	void ClearPendingStart();
};

#endif // GAMELOOP_H
