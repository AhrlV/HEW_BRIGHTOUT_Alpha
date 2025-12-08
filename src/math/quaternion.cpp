/*====================================================================

	Quaternionクラス [quaternion.cpp]
	クォータニオンクラスの実装。
	
	【重要】インターフェースは度数法（degree）で統一
	内部処理では自動的にラジアンに変換される

	Author : AI Assistant
	Date   : 2025/01/20

====================================================================*/

#include "math/quaternion.h"

using namespace DirectX;

/*====================================================================
	デフォルトコンストラクタ（単位クォータニオン）
====================================================================*/
Quaternion::Quaternion()
	: m_x(0.0f)
	, m_y(0.0f)
	, m_z(0.0f)
	, m_w(1.0f)
{
}

/*====================================================================
	クォータニオン成分指定コンストラクタ
====================================================================*/
Quaternion::Quaternion(float _x, float _y, float _z, float _w)
	: m_x(_x)
	, m_y(_y)
	, m_z(_z)
	, m_w(_w)
{
}

/*====================================================================
	XMFLOAT4からの変換コンストラクタ
====================================================================*/
Quaternion::Quaternion(const XMFLOAT4& q)
	: m_x(q.x)
	, m_y(q.y)
	, m_z(q.z)
	, m_w(q.w)
{
}

/*====================================================================
	XMVECTORからの変換コンストラクタ
====================================================================*/
Quaternion::Quaternion(const XMVECTOR& q)
{
	XMFLOAT4 temp;
	XMStoreFloat4(&temp, q);
	m_x = temp.x;
	m_y = temp.y;
	m_z = temp.z;
	m_w = temp.w;
}

/*====================================================================
	XMFLOAT4への変換
====================================================================*/
XMFLOAT4 Quaternion::ToXMFLOAT4() const
{
	return XMFLOAT4(m_x, m_y, m_z, m_w);
}

/*====================================================================
	XMVECTORへの変換
====================================================================*/
XMVECTOR Quaternion::ToXMVECTOR() const
{
	XMFLOAT4 temp(m_x, m_y, m_z, m_w);
	return XMLoadFloat4(&temp);
}

/*====================================================================
	X軸回転を設定（度数法）
====================================================================*/
void Quaternion::SetEulerX(float angleDegrees)
{
	float angleRadians = DegreesToRadians(angleDegrees);
	XMVECTOR quat = XMQuaternionRotationRollPitchYaw(angleRadians, 0.0f, 0.0f);
	*this = Quaternion(quat);
}

/*====================================================================
	Y軸回転を設定（度数法）
====================================================================*/
void Quaternion::SetEulerY(float angleDegrees)
{
	float angleRadians = DegreesToRadians(angleDegrees);
	XMVECTOR quat = XMQuaternionRotationRollPitchYaw(0.0f, angleRadians, 0.0f);
	*this = Quaternion(quat);
}

/*====================================================================
	Z軸回転を設定（度数法）
====================================================================*/
void Quaternion::SetEulerZ(float angleDegrees)
{
	float angleRadians = DegreesToRadians(angleDegrees);
	XMVECTOR quat = XMQuaternionRotationRollPitchYaw(0.0f, 0.0f, angleRadians);
	*this = Quaternion(quat);
}

/*====================================================================
	オイラー角を一括設定（度数法、XYZ順）
====================================================================*/
void Quaternion::SetEulerAngles(float pitchDegrees, float yawDegrees, float rollDegrees)
{
	float pitchRadians = DegreesToRadians(pitchDegrees);
	float yawRadians = DegreesToRadians(yawDegrees);
	float rollRadians = DegreesToRadians(rollDegrees);
	XMVECTOR quat = XMQuaternionRotationRollPitchYaw(pitchRadians, yawRadians, rollRadians);
	*this = Quaternion(quat);
}

/*====================================================================
	オイラー角を一括設定（Vector3版、度数法）
====================================================================*/
void Quaternion::SetEulerAngles(const Vector3& eulerAnglesDegrees)
{
	SetEulerAngles(eulerAnglesDegrees.x, eulerAnglesDegrees.y, eulerAnglesDegrees.z);
}

/*====================================================================
	X軸回転を取得（度数法）
====================================================================*/
float Quaternion::GetEulerX() const
{
	Vector3 euler = GetEulerAngles();
	return euler.x;
}

/*====================================================================
	Y軸回転を取得（度数法）
====================================================================*/
float Quaternion::GetEulerY() const
{
	Vector3 euler = GetEulerAngles();
	return euler.y;
}

/*====================================================================
	Z軸回転を取得（度数法）
====================================================================*/
float Quaternion::GetEulerZ() const
{
	Vector3 euler = GetEulerAngles();
	return euler.z;
}

/*====================================================================
	オイラー角を一括取得（度数法）
====================================================================*/
Vector3 Quaternion::GetEulerAngles() const
{
	// ロール（X軸回転）
	float sinr_cosp = 2.0f * (m_w * m_x + m_y * m_z);
	float cosr_cosp = 1.0f - 2.0f * (m_x * m_x + m_y * m_y);
	float roll = std::atan2(sinr_cosp, cosr_cosp);
	
	// ピッチ（Y軸回転）
	float sinp = 2.0f * (m_w * m_y - m_z * m_x);
	float pitch;
	if (std::abs(sinp) >= 1.0f)
	{
		pitch = std::copysign(XM_PIDIV2, sinp);
	}
	else
	{
		pitch = std::asin(sinp);
	}
	
	// ヨー（Z軸回転）
	float siny_cosp = 2.0f * (m_w * m_z + m_x * m_y);
	float cosy_cosp = 1.0f - 2.0f * (m_y * m_y + m_z * m_z);
	float yaw = std::atan2(siny_cosp, cosy_cosp);
	
	// ラジアンから度数法に変換
	return Vector3(RadiansToDegrees(roll), RadiansToDegrees(pitch), RadiansToDegrees(yaw));
}

/*====================================================================
	X軸回転を追加（度数法）
====================================================================*/
void Quaternion::RotateX(float angleDegrees)
{
	Quaternion rotX = FromEulerAngles(angleDegrees, 0.0f, 0.0f);
	*this = *this * rotX;
}

/*====================================================================
	Y軸回転を追加（度数法）
====================================================================*/
void Quaternion::RotateY(float angleDegrees)
{
	Quaternion rotY = FromEulerAngles(0.0f, angleDegrees, 0.0f);
	*this = *this * rotY;
}

/*====================================================================
	Z軸回転を追加（度数法）
====================================================================*/
void Quaternion::RotateZ(float angleDegrees)
{
	Quaternion rotZ = FromEulerAngles(0.0f, 0.0f, angleDegrees);
	*this = *this * rotZ;
}

/*====================================================================
	オイラー角を追加回転（Vector3版、度数法）
====================================================================*/
void Quaternion::Rotate(const Vector3& eulerAnglesDegrees)
{
	Quaternion rot = FromEulerAngles(eulerAnglesDegrees);
	*this = *this * rot;
}

/*====================================================================
	現在の回転にオイラー角を加算（Vector3版、度数法）
====================================================================*/
void Quaternion::AddEulerAngles(const Vector3& eulerAnglesDegrees)
{
	// 現在のオイラー角を取得
	Vector3 currentAngles = GetEulerAngles();
	
	// 新しい角度を加算
	Vector3 newAngles = currentAngles + eulerAnglesDegrees;
	
	// 新しいオイラー角で設定
	SetEulerAngles(newAngles);
}

/*====================================================================
	クォータニオンの長さを取得
====================================================================*/
float Quaternion::Length() const
{
	XMVECTOR quat = ToXMVECTOR();
	XMVECTOR len = XMQuaternionLength(quat);
	return XMVectorGetX(len);
}

/*====================================================================
	クォータニオンの長さの2乗を取得（高速）
====================================================================*/
float Quaternion::LengthSquared() const
{
	XMVECTOR quat = ToXMVECTOR();
	XMVECTOR lenSq = XMQuaternionLengthSq(quat);
	return XMVectorGetX(lenSq);
}

/*====================================================================
	クォータニオンを正規化
====================================================================*/
Quaternion& Quaternion::Normalize()
{
	XMVECTOR quat = ToXMVECTOR();
	XMVECTOR normalized = XMQuaternionNormalize(quat);
	*this = Quaternion(normalized);
	return *this;
}

/*====================================================================
	正規化されたクォータニオンを取得
====================================================================*/
Quaternion Quaternion::Normalized() const
{
	Quaternion result(*this);
	result.Normalize();
	return result;
}

/*====================================================================
	共役クォータニオンを取得
====================================================================*/
Quaternion Quaternion::Conjugate() const
{
	XMVECTOR quat = ToXMVECTOR();
	XMVECTOR conj = XMQuaternionConjugate(quat);
	return Quaternion(conj);
}

/*====================================================================
	逆クォータニオンを取得
====================================================================*/
Quaternion Quaternion::Inverse() const
{
	XMVECTOR quat = ToXMVECTOR();
	XMVECTOR inv = XMQuaternionInverse(quat);
	if (XMQuaternionIsNaN(inv))
	{
		throw std::runtime_error("Cannot invert zero quaternion");
	}
	return Quaternion(inv);
}

/*====================================================================
	オイラー角からクォータニオンを作成（度数法、XYZ順）
====================================================================*/
Quaternion Quaternion::FromEulerAngles(float pitchDegrees, float yawDegrees, float rollDegrees)
{
	float pitchRadians = DegreesToRadians(pitchDegrees);
	float yawRadians = DegreesToRadians(yawDegrees);
	float rollRadians = DegreesToRadians(rollDegrees);
	XMVECTOR quat = XMQuaternionRotationRollPitchYaw(pitchRadians, yawRadians, rollRadians);
	return Quaternion(quat);
}

/*====================================================================
	オイラー角からクォータニオンを作成（Vector3版、度数法）
====================================================================*/
Quaternion Quaternion::FromEulerAngles(const Vector3& eulerAnglesDegrees)
{
	return FromEulerAngles(eulerAnglesDegrees.x, eulerAnglesDegrees.y, eulerAnglesDegrees.z);
}

/*====================================================================
	クォータニオンをオイラー角に変換（度数法）
====================================================================*/
Vector3 Quaternion::ToEulerAngles() const
{
	return GetEulerAngles();
}

/*====================================================================
	軸と角度からクォータニオンを作成（度数法）
====================================================================*/
Quaternion Quaternion::FromAxisAngle(const Vector3& axis, float angleDegrees)
{
	float angleRadians = DegreesToRadians(angleDegrees);
	XMVECTOR axisVec = axis.ToXMVECTOR();
	XMVECTOR quat = XMQuaternionRotationAxis(axisVec, angleRadians);
	return Quaternion(quat);
}

/*====================================================================
	ベクトルを回転
====================================================================*/
Vector3 Quaternion::RotateVector(const Vector3& v) const
{
	XMVECTOR quat = ToXMVECTOR();
	XMVECTOR vec = v.ToXMVECTOR();
	XMVECTOR result = XMVector3Rotate(vec, quat);
	return Vector3(result);
}

/*====================================================================
	内積
====================================================================*/
float Quaternion::Dot(const Quaternion& a, const Quaternion& b)
{
	XMVECTOR qa = a.ToXMVECTOR();
	XMVECTOR qb = b.ToXMVECTOR();
	XMVECTOR dot = XMQuaternionDot(qa, qb);
	return XMVectorGetX(dot);
}

/*====================================================================
	球面線形補間（SLERP）
====================================================================*/
Quaternion Quaternion::Slerp(const Quaternion& a, const Quaternion& b, float t)
{
	XMVECTOR qa = a.ToXMVECTOR();
	XMVECTOR qb = b.ToXMVECTOR();
	XMVECTOR result = XMQuaternionSlerp(qa, qb, t);
	return Quaternion(result);
}

/*====================================================================
	2つの方向ベクトル間の回転を求める
====================================================================*/
Quaternion Quaternion::LookRotation(const Vector3& forward, const Vector3& up)
{
	Vector3 f = forward.Normalized();
	Vector3 r = Vector3::Cross(up, f).Normalized();
	Vector3 u = Vector3::Cross(f, r);
	
	// 回転行列を作成
	XMFLOAT3 right = r.ToXMFLOAT3();
	XMFLOAT3 upVec = u.ToXMFLOAT3();
	XMFLOAT3 fwd = f.ToXMFLOAT3();
	
	XMMATRIX rotMatrix(
		right.x, right.y, right.z, 0.0f,
		upVec.x, upVec.y, upVec.z, 0.0f,
		fwd.x, fwd.y, fwd.z, 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f
	);
	
	// 行列からクォータニオンに変換
	XMVECTOR quat = XMQuaternionRotationMatrix(rotMatrix);
	return Quaternion(quat);
}

/*====================================================================
	クォータニオンの乗算（回転の合成）
====================================================================*/
Quaternion Quaternion::operator*(const Quaternion& other) const
{
	XMVECTOR q1 = ToXMVECTOR();
	XMVECTOR q2 = other.ToXMVECTOR();
	XMVECTOR result = XMQuaternionMultiply(q1, q2);
	return Quaternion(result);
}

/*====================================================================
	スカラー乗算
====================================================================*/
Quaternion Quaternion::operator*(float scalar) const
{
	return Quaternion(m_x * scalar, m_y * scalar, m_z * scalar, m_w * scalar);
}

/*====================================================================
	スカラー除算
====================================================================*/
Quaternion Quaternion::operator/(float scalar) const
{
	if (std::abs(scalar) < 1e-6f)
	{
		throw std::runtime_error("Division by zero");
	}
	return Quaternion(m_x / scalar, m_y / scalar, m_z / scalar, m_w / scalar);
}

/*====================================================================
	加算
====================================================================*/
Quaternion Quaternion::operator+(const Quaternion& other) const
{
	return Quaternion(m_x + other.m_x, m_y + other.m_y, m_z + other.m_z, m_w + other.m_w);
}

/*====================================================================
	減算
====================================================================*/
Quaternion Quaternion::operator-(const Quaternion& other) const
{
	return Quaternion(m_x - other.m_x, m_y - other.m_y, m_z - other.m_z, m_w - other.m_w);
}

/*====================================================================
	単項マイナス
====================================================================*/
Quaternion Quaternion::operator-() const
{
	return Quaternion(-m_x, -m_y, -m_z, -m_w);
}

/*====================================================================
	乗算代入
====================================================================*/
Quaternion& Quaternion::operator*=(const Quaternion& other)
{
	*this = *this * other;
	return *this;
}

/*====================================================================
	スカラー乗算代入
====================================================================*/
Quaternion& Quaternion::operator*=(float scalar)
{
	m_x *= scalar;
	m_y *= scalar;
	m_z *= scalar;
	m_w *= scalar;
	return *this;
}

/*====================================================================
	スカラー除算代入
====================================================================*/
Quaternion& Quaternion::operator/=(float scalar)
{
	if (std::abs(scalar) < 1e-6f)
	{
		throw std::runtime_error("Division by zero");
	}
	m_x /= scalar;
	m_y /= scalar;
	m_z /= scalar;
	m_w /= scalar;
	return *this;
}

/*====================================================================
	等価比較
====================================================================*/
bool Quaternion::operator==(const Quaternion& other) const
{
	return std::abs(m_x - other.m_x) < 1e-6f &&
	       std::abs(m_y - other.m_y) < 1e-6f &&
	       std::abs(m_z - other.m_z) < 1e-6f &&
	       std::abs(m_w - other.m_w) < 1e-6f;
}

/*====================================================================
	非等価比較
====================================================================*/
bool Quaternion::operator!=(const Quaternion& other) const
{
	return !(*this == other);
}
