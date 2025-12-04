/*====================================================================

	Object基底クラス [object.h]
	GameObjectとComponentの共通基底クラス。
	アクティブフラグ、ID、Destroyフラグ、リソースクラスIDと名前を管理する。

	Author : Ryosuke Kageyama
	Date   : 2025/11/26

====================================================================*/

#ifndef LIFECYCLE_OBJECT_H
#define LIFECYCLE_OBJECT_H

#include <cstdint>
#include <memory>
#include <type_traits>
#include <stdexcept>
#include <string>

// 前方宣言
class Scene;
class GameObject;

/*====================================================================
	リソースクラスID列挙型
	各リソースタイプを識別するための列挙型
====================================================================*/
enum class ResourceClassID
{
	Unknown = 0,    // 不明なリソースタイプ

	// === 静的リソース（ResourceManagerで管理可能） ===
	Mesh,           // メッシュリソース
	Material,       // マテリアルリソース
	Texture,        // テクスチャリソース
	Model,          // 3Dモデルリソース
	Shader,         // シェーダーリソース
	Audio,          // オーディオリソース（将来的な拡張用）

	// === 境界マーカー ===
	StaticResourceBorder, // 静的リソースの境界（この値より上は静的リソース）

	// === 動的オブジェクト（ResourceManagerで管理不可） ===
	GameObject,     // ゲームオブジェクト
	Component,      // コンポーネント

	Count           // リソースタイプ数（配列サイズ計算用）
};

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

	// リソースのクラスID（リソース型の識別）
	ResourceClassID m_ClassID;

	// オブジェクトの識別名（リソースのキャッシュキーとして使用）
	std::wstring m_Name;

	// コンストラクタ（継承クラスからのみ呼び出し可能）
	Object();

public:
	// デストラクタ
	virtual ~Object();

	/*====================================================================
		ID・アクティブ状態の取得・設定
	====================================================================*/

	// IDの取得
	uint64_t GetId() const;

	// アクティブ状態の取得・設定
	bool IsActive() const;
	void SetActive(bool active);

	// 破棄予約の取得・設定
	bool IsDestroyed() const;
	void Destroy();

	/*====================================================================
		リソースクラスID・名前の取得・設定
	====================================================================*/

	// クラスIDを取得する
	// 戻り値: オブジェクトのクラスID（ResourceClassID列挙型）
	ResourceClassID GetClassID() const;

	// クラスIDを設定する
	// 引数:
	//   classID - 設定するクラスID（ResourceClassID列挙型）
	void SetClassID(ResourceClassID classID);

	// 識別名を取得する
	// 戻り値: オブジェクトの識別名
	const std::wstring& GetName() const;

	// 識別名を設定する
	// 引数:
	//   name - 設定する識別名
	void SetName(const std::wstring& name);

	// Sceneクラスが内部データにアクセス可能にする
	friend class Scene;
	friend class GameObject;
};

#endif // LIFECYCLE_OBJECT_H
