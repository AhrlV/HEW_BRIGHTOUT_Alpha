#pragma once

/*====================================================================

	Quaternionクラス [quaternion.h]
	DirectXMathライブラリのXMVECTORとXMQuaternionをラップした
	クォータニオンクラス。内部でXYZW成分を保持し、
	オイラー角による操作を提供する。
	
	【重要】インターフェースは度数法（degree）で統一
	内部処理では自動的にラジアンに変換される

	Author : AI Assistant
	Date   : 2025/01/20

====================================================================*/

#ifndef QUATERNION_H
#define QUATERNION_H

#include <DirectXMath.h>
#include <stdexcept>
#include <cmath>
#include "math/vector3.h"

/*====================================================================
	Quaternionクラス
	回転を表現するクラス。
	内部でクォータニオン成分（XYZW）を保持し、
	オイラー角による操作インターフェースを提供する。
	
	【使用方法】
	全ての角度パラメータは度数法（0～360度）で指定
	例: SetEulerY(90.0f) → Y軸周りに90度回転
====================================================================*/
class Quaternion
{
public:
	/*====================================================================
		コンストラクタ
	====================================================================*/
	
	// デフォルトコンストラクタ（単位クォータニオン）
	Quaternion();
	
	// クォータニオン成分指定コンストラクタ
	Quaternion(float _x, float _y, float _z, float _w);
	
	// XMFLOAT4からの変換コンストラクタ
	Quaternion(const DirectX::XMFLOAT4& q);
	
	// XMVECTORからの変換コンストラクタ
	Quaternion(const DirectX::XMVECTOR& q);

	/*====================================================================
		変換関数
	====================================================================*/
	
	// XMFLOAT4への変換
	DirectX::XMFLOAT4 ToXMFLOAT4() const;
	
	// XMVECTORへの変換
	DirectX::XMVECTOR ToXMVECTOR() const;

	/*====================================================================
		オイラー角による操作（度数法で指定）
	====================================================================*/
	
	// X軸回転を設定（度数法: 0～360）
	void SetEulerX(float angleDegrees);
	
	// Y軸回転を設定（度数法: 0～360）
	void SetEulerY(float angleDegrees);
	
	// Z軸回転を設定（度数法: 0～360）
	void SetEulerZ(float angleDegrees);
	
	// オイラー角を一括設定（度数法、XYZ順）
	void SetEulerAngles(float pitchDegrees, float yawDegrees, float rollDegrees);
	
	// オイラー角を一括設定（Vector3版、度数法）
	void SetEulerAngles(const Vector3& eulerAnglesDegrees);
	
	// X軸回転を取得（度数法: 0～360）
	float GetEulerX() const;
	
	// Y軸回転を取得（度数法: 0～360）
	float GetEulerY() const;
	
	// Z軸回転を取得（度数法: 0～360）
	float GetEulerZ() const;
	
	// オイラー角を一括取得（度数法）
	Vector3 GetEulerAngles() const;
	
	// X軸回転を追加（度数法: 0～360）
	void RotateX(float angleDegrees);
	
	// Y軸回転を追加（度数法: 0～360）
	void RotateY(float angleDegrees);
	
	// Z軸回転を追加（度数法: 0～360）
	void RotateZ(float angleDegrees);
	
	// オイラー角を追加回転（Vector3版、度数法）
	void Rotate(const Vector3& eulerAnglesDegrees);
	
	// 現在の回転にオイラー角を加算（Vector3版、度数法）
	void AddEulerAngles(const Vector3& eulerAnglesDegrees);

	/*====================================================================
		クォータニオン演算
	====================================================================*/
	
	// クォータニオンの長さを取得
	float Length() const;
	
	// クォータニオンの長さの2乗を取得（高速）
	float LengthSquared() const;
	
	// クォータニオンを正規化
	Quaternion& Normalize();
	
	// 正規化されたクォータニオンを取得
	Quaternion Normalized() const;
	
	// 共役クォータニオンを取得
	Quaternion Conjugate() const;
	
	// 逆クォータニオンを取得
	Quaternion Inverse() const;

	/*====================================================================
		回転関数（度数法で指定）
	====================================================================*/
	
	// オイラー角からクォータニオンを作成（度数法、XYZ順）
	static Quaternion FromEulerAngles(float pitchDegrees, float yawDegrees, float rollDegrees);
	
	// オイラー角からクォータニオンを作成（Vector3版、度数法）
	static Quaternion FromEulerAngles(const Vector3& eulerAnglesDegrees);
	
	// クォータニオンをオイラー角に変換（度数法）
	Vector3 ToEulerAngles() const;
	
	// 軸と角度からクォータニオンを作成（度数法）
	static Quaternion FromAxisAngle(const Vector3& axis, float angleDegrees);
	
	// ベクトルを回転
	Vector3 RotateVector(const Vector3& v) const;

	/*====================================================================
		静的ユーティリティ関数
	====================================================================*/
	
	// 内積
	static float Dot(const Quaternion& a, const Quaternion& b);
	
	// 球面線形補間（SLERP）
	static Quaternion Slerp(const Quaternion& a, const Quaternion& b, float t);
	
	// 2つの方向ベクトル間の回転を求める
	static Quaternion LookRotation(const Vector3& forward, const Vector3& up = Vector3::Up);

	/*====================================================================
		演算子オーバーロード
	====================================================================*/
	
	// クォータニオンの乗算（回転の合成）
	Quaternion operator*(const Quaternion& other) const;
	
	// スカラー乗算
	Quaternion operator*(float scalar) const;
	
	// スカラー除算
	Quaternion operator/(float scalar) const;
	
	// 加算
	Quaternion operator+(const Quaternion& other) const;
	
	// 減算
	Quaternion operator-(const Quaternion& other) const;
	
	// 単項マイナス
	Quaternion operator-() const;
	
	// 乗算代入
	Quaternion& operator*=(const Quaternion& other);
	
	// スカラー乗算代入
	Quaternion& operator*=(float scalar);
	
	// スカラー除算代入
	Quaternion& operator/=(float scalar);
	
	// 等価比較
	bool operator==(const Quaternion& other) const;
	
	// 非等価比較
	bool operator!=(const Quaternion& other) const;

	/*====================================================================
		定数クォータニオン
	====================================================================*/
	
	static const Quaternion Identity;

private:
	/*====================================================================
		内部メンバ変数（クォータニオン成分）
	====================================================================*/
	
	float m_x;  // X成分
	float m_y;  // Y成分
	float m_z;  // Z成分
	float m_w;  // W成分
	
	/*====================================================================
		内部ヘルパー関数
	====================================================================*/
	
	// 度数法をラジアンに変換
	static inline float DegreesToRadians(float degrees)
	{
		return DirectX::XMConvertToRadians(degrees);
	}
	
	// ラジアンを度数法に変換
	static inline float RadiansToDegrees(float radians)
	{
		return DirectX::XMConvertToDegrees(radians);
	}
};

// スカラー * クォータニオン の演算子（左側のスカラー乗算）
inline Quaternion operator*(float scalar, const Quaternion& q)
{
	return q * scalar;
}

// 定数クォータニオンの定義
inline const Quaternion Quaternion::Identity(0.0f, 0.0f, 0.0f, 1.0f);

#endif // QUATERNION_H
