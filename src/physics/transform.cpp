/*====================================================================

	TransFormクラス [transform.cpp]
	ゲームオブジェクトの位置、回転、スケールを管理するコンポーネントの実装。

	Author : AI Assistant
	Date   : 2025/01/20

====================================================================*/

#include "physics/transform.h"
#include <stdexcept>

using namespace DirectX;

/*====================================================================
	コンストラクタ
	デフォルト値で初期化する。
====================================================================*/
TransForm::TransForm()
	: m_Position(Vector3::Zero)
	, m_Rotation(Quaternion::Identity)
	, m_Scale(Vector3::One)
	, m_Parent(nullptr)
	, m_WorldMatrix(XMMatrixIdentity())
	, m_IsDirty(true)
{
}

/*====================================================================
	デストラクタ
	親子関係を適切に解除する。
====================================================================*/
TransForm::~TransForm()
{
	// 親から自分を削除
	if (m_Parent)
	{
		m_Parent->RemoveChild(this);
	}
	
	// 全ての子の親参照を解除
	for (TransForm* child : m_Children)
	{
		if (child)
		{
			child->m_Parent = nullptr;
		}
	}
}

/*====================================================================
	ローカル位置の参照を取得
====================================================================*/
Vector3& TransForm::Position()
{
	SetDirty();
	return m_Position;
}

/*====================================================================
	ローカル位置の参照を取得（const版）
====================================================================*/
const Vector3& TransForm::Position() const
{
	return m_Position;
}

/*====================================================================
	ローカル回転の参照を取得
====================================================================*/
Quaternion& TransForm::Rotation()
{
	SetDirty();
	return m_Rotation;
}

/*====================================================================
	ローカル回転の参照を取得（const版）
====================================================================*/
const Quaternion& TransForm::Rotation() const
{
	return m_Rotation;
}

/*====================================================================
	ローカルスケールの参照を取得
====================================================================*/
Vector3& TransForm::Scale()
{
	SetDirty();
	return m_Scale;
}

/*====================================================================
	ローカルスケールの参照を取得（const版）
====================================================================*/
const Vector3& TransForm::Scale() const
{
	return m_Scale;
}

/*====================================================================
	ワールド位置を取得
	親がいる場合は親のワールド変換を考慮した位置を返す。
====================================================================*/
Vector3 TransForm::GetWorldPosition() const
{
	if (!m_Parent)
	{
		return m_Position;
	}
	
	// 親のワールド変換を適用
	Vector3 worldPos = m_Parent->GetWorldRotation().RotateVector(m_Position * m_Parent->GetWorldScale());
	worldPos += m_Parent->GetWorldPosition();
	return worldPos;
}

/*====================================================================
	ワールド回転を取得
	親がいる場合は親のワールド回転を考慮した回転を返す。
====================================================================*/
Quaternion TransForm::GetWorldRotation() const
{
	if (!m_Parent)
	{
		return m_Rotation;
	}
	
	// 親のワールド回転を適用
	return m_Parent->GetWorldRotation() * m_Rotation;
}

/*====================================================================
	ワールドスケールを取得
	親がいる場合は親のワールドスケールを考慮したスケールを返す。
====================================================================*/
Vector3 TransForm::GetWorldScale() const
{
	if (!m_Parent)
	{
		return m_Scale;
	}
	
	// 親のワールドスケールを乗算
	Vector3 parentScale = m_Parent->GetWorldScale();
	return Vector3(
		m_Scale.x * parentScale.x,
		m_Scale.y * parentScale.y,
		m_Scale.z * parentScale.z
	);
}

/*====================================================================
	ワールド変換行列を取得
	ダーティフラグが立っている場合のみ行列を再計算する。
====================================================================*/
XMMATRIX TransForm::GetWorldMatrix() const
{
	// ダーティフラグが立っている場合は行列を更新
	if (m_IsDirty)
	{
		UpdateWorldMatrix();
	}
	
	return m_WorldMatrix;
}

/*====================================================================
	ローカル変換行列を取得
	位置、回転、スケールからローカル変換行列を計算する。
====================================================================*/
XMMATRIX TransForm::GetLocalMatrix() const
{
	// スケール行列
	XMMATRIX S = XMMatrixScaling(m_Scale.x, m_Scale.y, m_Scale.z);
	
	// 回転行列
	XMVECTOR rotQuat = m_Rotation.ToXMVECTOR();
	XMMATRIX R = XMMatrixRotationQuaternion(rotQuat);
	
	// 平行移動行列
	XMMATRIX T = XMMatrixTranslation(m_Position.x, m_Position.y, m_Position.z);
	
	// S * R * T の順で乗算
	return S * R * T;
}

/*====================================================================
	親TransFormを設定
	親子関係を適切に管理し、ダーティフラグを更新する。
	
	引数:
	  parent - 親TransForm（nullptrで親を解除）
====================================================================*/
void TransForm::SetParent(TransForm* parent)
{
	// 既存の親から自分を削除
	if (m_Parent)
	{
		m_Parent->RemoveChild(this);
	}
	
	// 新しい親を設定
	m_Parent = parent;
	
	// 新しい親に自分を追加
	if (m_Parent)
	{
		m_Parent->AddChild(this);
	}
	
	// ダーティフラグを設定
	SetDirty();
}

/*====================================================================
	親TransFormを取得
	
	戻り値: 親TransForm（親がいない場合はnullptr）
====================================================================*/
TransForm* TransForm::GetParent() const
{
	return m_Parent;
}

/*====================================================================
	子TransFormを追加
	内部で自動的に呼ばれる。直接呼ぶ必要はない。
	
	引数:
	  child - 追加する子TransForm
====================================================================*/
void TransForm::AddChild(TransForm* child)
{
	if (!child)
	{
		throw std::runtime_error("Cannot add null child to TransForm");
	}
	
	// 既に子リストに存在する場合は追加しない
	for (TransForm* existingChild : m_Children)
	{
		if (existingChild == child)
		{
			return;
		}
	}
	
	m_Children.push_back(child);
}

/*====================================================================
	子TransFormを削除
	内部で自動的に呼ばれる。直接呼ぶ必要はない。
	
	引数:
	  child - 削除する子TransForm
====================================================================*/
void TransForm::RemoveChild(TransForm* child)
{
	if (!child)
	{
		return;
	}
	
	// 子リストから削除
	for (auto it = m_Children.begin(); it != m_Children.end(); ++it)
	{
		if (*it == child)
		{
			m_Children.erase(it);
			return;
		}
	}
}

/*====================================================================
	子TransFormのリストを取得
	
	戻り値: 子TransFormのリスト
====================================================================*/
const std::vector<TransForm*>& TransForm::GetChildren() const
{
	return m_Children;
}

/*====================================================================
	子TransFormの数を取得
	
	戻り値: 子の数
====================================================================*/
size_t TransForm::GetChildCount() const
{
	return m_Children.size();
}

/*====================================================================
	前方向ベクトルを取得（ワールド空間）
	
	戻り値: 前方向の単位ベクトル
====================================================================*/
Vector3 TransForm::GetForward() const
{
	Quaternion worldRot = GetWorldRotation();
	return worldRot.RotateVector(Vector3::Forward);
}

/*====================================================================
	右方向ベクトルを取得（ワールド空間）
	
	戻り値: 右方向の単位ベクトル
====================================================================*/
Vector3 TransForm::GetRight() const
{
	Quaternion worldRot = GetWorldRotation();
	return worldRot.RotateVector(Vector3::Right);
}

/*====================================================================
	上方向ベクトルを取得（ワールド空間）
	
	戻り値: 上方向の単位ベクトル
====================================================================*/
Vector3 TransForm::GetUp() const
{
	Quaternion worldRot = GetWorldRotation();
	return worldRot.RotateVector(Vector3::Up);
}

/*====================================================================
	ローカル位置を移動
	現在の位置に指定した移動量を加算する。
	
	引数:
	  translation - 移動量
====================================================================*/
void TransForm::Translate(const Vector3& translation)
{
	m_Position += translation;
	SetDirty();
}

/*====================================================================
	ローカル回転を適用
	現在の回転に指定した回転を乗算する。
	
	引数:
	  rotation - 適用する回転
====================================================================*/
void TransForm::Rotate(const Quaternion& rotation)
{
	m_Rotation = m_Rotation * rotation;
	SetDirty();
}

/*====================================================================
	指定した方向を向くように回転を設定（ワールド空間）
	
	引数:
	  target - 向きたい対象の位置（ワールド空間）
	  up     - 上方向ベクトル（デフォルト: Vector3::Up）
====================================================================*/
void TransForm::LookAt(const Vector3& target, const Vector3& up)
{
	Vector3 worldPos = GetWorldPosition();
	Vector3 forward = (target - worldPos).Normalized();
	
	// ワールド空間での回転を計算
	Quaternion worldRotation = Quaternion::LookRotation(forward, up);
	
	// 親がいる場合はローカル回転に変換
	if (m_Parent)
	{
		Quaternion parentWorldRot = m_Parent->GetWorldRotation();
		m_Rotation = parentWorldRot.Inverse() * worldRotation;
	}
	else
	{
		m_Rotation = worldRotation;
	}
	
	SetDirty();
}

/*====================================================================
	ダーティフラグを設定
	自分と全ての子のダーティフラグを立てる。
====================================================================*/
void TransForm::SetDirty() const
{
	m_IsDirty = true;
	
	// すべての子のダーティフラグも設定
	for (TransForm* child : m_Children)
	{
		if (child)
		{
			child->SetDirty();
		}
	}
}

/*====================================================================
	ワールド変換行列を更新
	ローカル変換行列と親のワールド変換行列を乗算する。
====================================================================*/
void TransForm::UpdateWorldMatrix() const
{
	// ローカル変換行列を計算
	XMMATRIX localMatrix = GetLocalMatrix();
	
	// 親がある場合は親のワールド行列を乗算
	if (m_Parent)
	{
		m_WorldMatrix = localMatrix * m_Parent->GetWorldMatrix();
	}
	else
	{
		m_WorldMatrix = localMatrix;
	}
	
	// ダーティフラグをクリア
	m_IsDirty = false;
}