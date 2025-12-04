/*====================================================================

	Vector3クラス [vector3.h]
	DirectXMathライブラリのXMVECTORとXMFLOAT3をラップした
	3次元ベクトルクラス。演算子のオーバーロードを提供。

	Author : AI Assistant
	Date   : 2025/01/20

====================================================================*/

#pragma once

#include <DirectXMath.h>
#include <stdexcept>
#include <cmath>

/*====================================================================
	Vector3クラス
	3次元ベクトルを表すクラス。
	DirectXMathのXMVECTORをラップし、直感的な演算子を提供する。
====================================================================*/
class Vector3
{
public:
	// メンバ変数
	float x;
	float y;
	float z;

	/*====================================================================
		コンストラクタ
	====================================================================*/
	
	// デフォルトコンストラクタ（ゼロベクトル）
	Vector3() : x(0.0f), y(0.0f), z(0.0f) {}
	
	// 成分指定コンストラクタ
	Vector3(float _x, float _y, float _z) : x(_x), y(_y), z(_z) {}
	
	// 単一値コンストラクタ（全成分に同じ値）
	explicit Vector3(float value) : x(value), y(value), z(value) {}
	
	// XMFLOAT3からの変換コンストラクタ
	Vector3(const DirectX::XMFLOAT3& v) : x(v.x), y(v.y), z(v.z) {}
	
	// XMVECTORからの変換コンストラクタ
	Vector3(const DirectX::XMVECTOR& v)
	{
		DirectX::XMFLOAT3 temp;
		DirectX::XMStoreFloat3(&temp, v);
		x = temp.x;
		y = temp.y;
		z = temp.z;
	}

	/*====================================================================
		変換関数
	====================================================================*/
	
	// XMFLOAT3への変換
	DirectX::XMFLOAT3 ToXMFLOAT3() const
	{
		return DirectX::XMFLOAT3(x, y, z);
	}
	
	// XMVECTORへの変換
	DirectX::XMVECTOR ToXMVECTOR() const
	{
		DirectX::XMFLOAT3 temp(x, y, z);
		return DirectX::XMLoadFloat3(&temp);
	}

	/*====================================================================
		ベクトル演算
	====================================================================*/
	
	// ベクトルの長さを取得
	float Length() const
	{
		return std::sqrt(x * x + y * y + z * z);
	}
	
	// ベクトルの長さの2乗を取得（高速）
	float LengthSquared() const
	{
		return x * x + y * y + z * z;
	}
	
	// ベクトルを正規化
	Vector3& Normalize()
	{
		float len = Length();
		if (len < 1e-6f)
		{
			throw std::runtime_error("Cannot normalize zero vector");
		}
		x /= len;
		y /= len;
		z /= len;
		return *this;
	}
	
	// 正規化されたベクトルを取得（元のベクトルは変更しない）
	Vector3 Normalized() const
	{
		Vector3 result(*this);
		result.Normalize();
		return result;
	}

	/*====================================================================
		静的ユーティリティ関数
	====================================================================*/
	
	// 内積
	static float Dot(const Vector3& a, const Vector3& b)
	{
		return a.x * b.x + a.y * b.y + a.z * b.z;
	}
	
	// 外積
	static Vector3 Cross(const Vector3& a, const Vector3& b)
	{
		return Vector3(
			a.y * b.z - a.z * b.y,
			a.z * b.x - a.x * b.z,
			a.x * b.y - a.y * b.x
		);
	}
	
	// 2つのベクトル間の距離
	static float Distance(const Vector3& a, const Vector3& b)
	{
		return (b - a).Length();
	}
	
	// 線形補間
	static Vector3 Lerp(const Vector3& a, const Vector3& b, float t)
	{
		return a + (b - a) * t;
	}

	/*====================================================================
		演算子オーバーロード
	====================================================================*/
	
	// 加算
	Vector3 operator+(const Vector3& other) const
	{
		return Vector3(x + other.x, y + other.y, z + other.z);
	}
	
	// 減算
	Vector3 operator-(const Vector3& other) const
	{
		return Vector3(x - other.x, y - other.y, z - other.z);
	}
	
	// スカラー乗算
	Vector3 operator*(float scalar) const
	{
		return Vector3(x * scalar, y * scalar, z * scalar);
	}
	
	// ベクトル同士の成分ごとの乗算
	Vector3 operator*(const Vector3& other) const
	{
		return Vector3(x * other.x, y * other.y, z * other.z);
	}
	
	// スカラー除算
	Vector3 operator/(float scalar) const
	{
		if (std::abs(scalar) < 1e-6f)
		{
			throw std::runtime_error("Division by zero");
		}
		return Vector3(x / scalar, y / scalar, z / scalar);
	}
	
	// 単項マイナス
	Vector3 operator-() const
	{
		return Vector3(-x, -y, -z);
	}
	
	// 加算代入
	Vector3& operator+=(const Vector3& other)
	{
		x += other.x;
		y += other.y;
		z += other.z;
		return *this;
	}
	
	// 減算代入
	Vector3& operator-=(const Vector3& other)
	{
		x -= other.x;
		y -= other.y;
		z -= other.z;
		return *this;
	}
	
	// スカラー乗算代入
	Vector3& operator*=(float scalar)
	{
		x *= scalar;
		y *= scalar;
		z *= scalar;
		return *this;
	}
	
	// スカラー除算代入
	Vector3& operator/=(float scalar)
	{
		if (std::abs(scalar) < 1e-6f)
		{
			throw std::runtime_error("Division by zero");
		}
		x /= scalar;
		y /= scalar;
		z /= scalar;
		return *this;
	}
	
	// 等価比較
	bool operator==(const Vector3& other) const
	{
		return std::abs(x - other.x) < 1e-6f &&
		       std::abs(y - other.y) < 1e-6f &&
		       std::abs(z - other.z) < 1e-6f;
	}
	
	// 非等価比較
	bool operator!=(const Vector3& other) const
	{
		return !(*this == other);
	}

	/*====================================================================
		定数ベクトル
	====================================================================*/
	
	static const Vector3 Zero;
	static const Vector3 One;
	static const Vector3 UnitX;
	static const Vector3 UnitY;
	static const Vector3 UnitZ;
	static const Vector3 Up;
	static const Vector3 Down;
	static const Vector3 Left;
	static const Vector3 Right;
	static const Vector3 Forward;
	static const Vector3 Backward;
};

// スカラー * ベクトル の演算子（左側のスカラー乗算）
inline Vector3 operator*(float scalar, const Vector3& v)
{
	return v * scalar;
}

// 定数ベクトルの定義
inline const Vector3 Vector3::Zero(0.0f, 0.0f, 0.0f);
inline const Vector3 Vector3::One(1.0f, 1.0f, 1.0f);
inline const Vector3 Vector3::UnitX(1.0f, 0.0f, 0.0f);
inline const Vector3 Vector3::UnitY(0.0f, 1.0f, 0.0f);
inline const Vector3 Vector3::UnitZ(0.0f, 0.0f, 1.0f);
inline const Vector3 Vector3::Up(0.0f, 1.0f, 0.0f);
inline const Vector3 Vector3::Down(0.0f, -1.0f, 0.0f);
inline const Vector3 Vector3::Left(-1.0f, 0.0f, 0.0f);
inline const Vector3 Vector3::Right(1.0f, 0.0f, 0.0f);
inline const Vector3 Vector3::Forward(0.0f, 0.0f, 1.0f);
inline const Vector3 Vector3::Backward(0.0f, 0.0f, -1.0f);
