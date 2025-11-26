/*====================================================================

	Object基底クラス [object.h]
	GameObjectとComponentの共通基底クラス。
	アクティブフラグ、ID、Destroyフラグを管理する。

	Author : Ryosuke Kageyama
	Date   : 2025/11/26

====================================================================*/

#ifndef LIFECYCLE_OBJECT_H
#define LIFECYCLE_OBJECT_H

#include <cstdint>

// 前方宣言
class Scene;

/*====================================================================
	Objectクラス
	GameObjectとComponentの基底クラス。
	全てのオブジェクトに共通するプロパティを持つ。
====================================================================*/
class Object
{
protected:
	// シーン内での一意なID
	uint64_t m_Id;

	// アクティブフラグ
	bool m_Active;

	// 破棄フラグ（trueの場合、次のフレームで削除される）
	bool m_Destroy;

	// コンストラクタ（継承クラスからのみ呼び出し可能）
	Object();

public:
	// デストラクタ
	virtual ~Object();

	// IDの取得
	uint64_t GetId() const;

	// アクティブ状態の取得・設定
	bool IsActive() const;
	void SetActive(bool active);

	// 破棄予約の取得・設定
	bool IsDestroyed() const;
	void Destroy();

	// Sceneクラスから内部データにアクセス可能にする
	friend class Scene;
	friend class GameObject;
};

#endif // LIFECYCLE_OBJECT_H
