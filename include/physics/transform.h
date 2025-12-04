/*====================================================================

	TransFormクラス [transform.h]
	ゲームオブジェクトの位置、回転、スケールを管理するコンポーネント。
	階層構造をサポートし、親子関係を持つことができる。
	ダーティフラグによる最適化で、必要な時のみ行列計算を行う。

	Author : Ryosuke Kageyama (Original)
	Modified by : AI Assistant
	Date   : 2025/01/20

====================================================================*/


#ifndef TRANSFORM_H
#define TRANSFORM_H

#include <DirectXMath.h>
#include <vector>
#include "lifecycle/component.h"
#include "math/vector3.h"
#include "math/quaternion.h"

/*====================================================================
	TransFormクラス
	ゲームオブジェクトの変換（位置、回転、スケール）を管理する。
	階層構造をサポートし、親子関係による相対的な変換を扱える。
====================================================================*/
class TransForm : public Component
{
public:
	/*====================================================================
		コンストラクタとデストラクタ
	====================================================================*/
	
	// コンストラクタ
	TransForm();
	
	// デストラクタ
	virtual ~TransForm();

	/*====================================================================
		ローカル変換プロパティのアクセサ（親に対する相対的な変換）
	====================================================================*/
	
	// ローカル位置の参照を取得
	Vector3& Position();
	
	// ローカル位置の参照を取得（constで）
	const Vector3& Position() const;
	
	// ローカル回転の参照を取得
	Quaternion& Rotation();
	
	// ローカル回転の参照を取得（constで）
	const Quaternion& Rotation() const;
	
	// ローカルスケールの参照を取得
	Vector3& Scale();
	
	// ローカルスケールの参照を取得（constで）
	const Vector3& Scale() const;

	/*====================================================================
		ワールド変換の取得
	====================================================================*/
	
	// ワールド位置を取得
	Vector3 GetWorldPosition() const;
	
	// ワールド回転を取得
	Quaternion GetWorldRotation() const;
	
	// ワールドスケールを取得
	Vector3 GetWorldScale() const;
	
	// ワールド変換行列を取得（ダーティフラグによる最適化）
	DirectX::XMMATRIX GetWorldMatrix() const;
	
	// ローカル変換行列を取得
	DirectX::XMMATRIX GetLocalMatrix() const;

	/*====================================================================
		階層構造の管理
	====================================================================*/
	
	// 親TransFormを設定
	void SetParent(TransForm* parent);
	
	// 親TransFormを取得
	TransForm* GetParent() const;
	
	// 子TransFormを追加（内部で自動的に呼ばれる）
	void AddChild(TransForm* child);
	
	// 子TransFormを削除（内部で自動的に呼ばれる）
	void RemoveChild(TransForm* child);
	
	// 子TransFormのリストを取得
	const std::vector<TransForm*>& GetChildren() const;
	
	// 子TransFormの数を取得
	size_t GetChildCount() const;

	/*====================================================================
		方向ベクトルの取得
	====================================================================*/
	
	// 前方向ベクトルを取得（ワールド空間）
	Vector3 GetForward() const;
	
	// 右方向ベクトルを取得（ワールド空間）
	Vector3 GetRight() const;
	
	// 上方向ベクトルを取得（ワールド空間）
	Vector3 GetUp() const;

	/*====================================================================
		変換の適用
	====================================================================*/
	
	// ローカル位置を移動
	void Translate(const Vector3& translation);
	
	// ローカル回転を適用
	void Rotate(const Quaternion& rotation);
	
	// 指定した方向を向くように回転を設定（ワールド空間）
	void LookAt(const Vector3& target, const Vector3& up = Vector3::Up);

private:
	/*====================================================================
		ローカル変換プロパティ（親に対する相対的な変換）
	====================================================================*/
	
	Vector3 m_Position;      // ローカル位置
	Quaternion m_Rotation;   // ローカル回転
	Vector3 m_Scale;         // ローカルスケール

	/*====================================================================
		階層構造の管理
	====================================================================*/
	
	TransForm* m_Parent;                    // 親TransForm
	std::vector<TransForm*> m_Children;     // 子TransFormのリスト

	/*====================================================================
		ダーティフラグによる最適化
	====================================================================*/
	
	mutable DirectX::XMMATRIX m_WorldMatrix;  // ワールド変換行列のキャッシュ
	mutable bool m_IsDirty;                   // ダーティフラグ
	
	// ダーティフラグを設定
	void SetDirty() const;
	
	// ワールド変換行列を更新
	void UpdateWorldMatrix() const;
};

#endif // TRANSFORM_H
