/*====================================================================

	Prefabクラス [prefab.h]
	GameObjectのテンプレートとして使用される静的リソース。
	Objectクラスを継承し、ルートGameObjectと全ての子GameObjectを保持する。

	Author : Ryosuke Kageyama
	Date   : 2025/11/26

====================================================================*/

#ifndef RESOURCEMANAGEMENT_PREFAB_H
#define RESOURCEMANAGEMENT_PREFAB_H

#include "lifecycle/object.h"
#include "lifecycle/gameobject.h"
#include <memory>
#include <string>
#include <vector>
#include <stdexcept>

// 前方宣言
class GameObject;

/*====================================================================
	Prefabクラス
	GameObjectのテンプレートを保持する静的リソース。
	Objectを継承し、ResourceManagerで管理可能。
	ルートGameObjectと全ての子GameObjectを保持する。
====================================================================*/
class Prefab : public Object
{
private:
	// プレハブの識別名
	std::wstring m_PrefabName;

	// ルートGameObject（テンプレート）
	std::shared_ptr<GameObject> m_RootGameObject;

	// ルートを含むすべてのGameObject（階層構造内のすべて）
	std::vector<std::shared_ptr<GameObject>> m_AllGameObjects;

public:
	/*====================================================================
		コンストラクタ
		Prefabオブジェクトを生成する。
		ClassIDをResourceClassID::Modelに設定する。
	====================================================================*/
	Prefab();

	/*====================================================================
		デストラクタ
	====================================================================*/
	virtual ~Prefab();

	/*====================================================================
		Initialize - Prefabを初期化する
		ルートGameObjectと名前を設定する。
		
		引数:
		  rootGameObject - プレハブのルートとなるGameObject
		  allGameObjects - ルートを含むすべてのGameObject
		  name - プレハブの識別名
		
		例外:
		  std::runtime_error - rootGameObjectがnullptrの場合
	====================================================================*/
	void Initialize(
		std::shared_ptr<GameObject> rootGameObject,
		std::vector<std::shared_ptr<GameObject>> allGameObjects,
		const std::wstring& name
	);

	/*====================================================================
		GetRootGameObject - ルートGameObjectを取得する
		
		戻り値: ルートGameObjectのshared_ptr
	====================================================================*/
	std::shared_ptr<GameObject> GetRootGameObject() const;

	/*====================================================================
		GetAllGameObjects - すべてのGameObjectを取得する
		
		戻り値: すべてのGameObjectのvector
	====================================================================*/
	const std::vector<std::shared_ptr<GameObject>>& GetAllGameObjects() const;

	/*====================================================================
		GetPrefabName - プレハブ名を取得する
		
		戻り値: プレハブの識別名
	====================================================================*/
	const std::wstring& GetPrefabName() const;

	/*====================================================================
		Instantiate - Prefabから GameObject を生成する
		元のルートGameObjectを複製し、現在のアクティブシーンに登録する。
		すべての子GameObjectもシーンに登録される。
		生成されたルートGameObjectの生ポインタを返す。
		
		引数:
		  original - 元となるPrefab
		戻り値: 生成されたルートGameObjectの生ポインタ
		
		例外:
		  std::runtime_error - originalがnullptr、またはルートGameObjectが未設定の場合
	====================================================================*/
	static GameObject* Instantiate(Prefab* original);

	/*====================================================================
		Destroy - Prefabを削除する
		削除フラグを立てる。
		
		引数:
		  obj - 削除するPrefab
		例外: objがnullptrの場合はruntime_errorをスロー
	====================================================================*/
	static void Destroy(Prefab* obj);
};

#endif // RESOURCEMANAGEMENT_PREFAB_H
