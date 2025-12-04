#pragma once

/*====================================================================

	Quaternionクラス [quaternion.h]
	DirectXMathライブラリのXMVECTORとXMQuaternionをラップした
	クォータニオンクラス。内部でXYZW成分を保持し、
	オイラー角による操作も提供する。

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
	回転を表すクラス。
	内部でクォータニオン成分（XYZW）を保持し、
	オイラー角による操作インターフェースも提供する。
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
		オイラー角による操作
	====================================================================*/
	
	// X軸回転を設定（ラジアン）
	void SetEulerX(float angleX);
	
	// Y軸回転を設定（ラジアン）
	void SetEulerY(float angleY);
	
	// Z軸回転を設定（ラジアン）
	void SetEulerZ(float angleZ);
	
	// オイラー角を一括設定（ラジアン、XYZ順）
	void SetEulerAngles(float pitch, float yaw, float roll);
	
	// オイラー角を一括設定（Vector3版）
	void SetEulerAngles(const Vector3& eulerAngles);
	
	// X軸回転を取得（ラジアン）
	float GetEulerX() const;
	
	// Y軸回転を取得（ラジアン）
	float GetEulerY() const;
	
	// Z軸回転を取得（ラジアン）
	float GetEulerZ() const;
	
	// オイラー角を一括取得（ラジアン）
	Vector3 GetEulerAngles() const;
	
	// X軸回転を追加（ラジアン）
	void RotateX(float angleX);
	
	// Y軸回転を追加（ラジアン）
	void RotateY(float angleY);
	
	// Z軸回転を追加（ラジアン）
	void RotateZ(float angleZ);

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
		回転関数
	====================================================================*/
	
	// オイラー角からクォータニオンを作成（ラジアン、XYZ順）
	static Quaternion FromEulerAngles(float pitch, float yaw, float roll);
	
	// オイラー角からクォータニオンを作成（Vector3版）
	static Quaternion FromEulerAngles(const Vector3& eulerAngles);
	
	// クォータニオンをオイラー角に変換（ラジアン）
	Vector3 ToEulerAngles() const;
	
	// 軸と角度からクォータニオンを作成
	static Quaternion FromAxisAngle(const Vector3& axis, float angle);
	
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
		メンバ変数（クォータニオン成分）
	====================================================================*/
	
	float m_x;  // X成分
	float m_y;  // Y成分
	float m_z;  // Z成分
	float m_w;  // W成分
};

// スカラー * クォータニオン の演算子（左側のスカラー乗算）
inline Quaternion operator*(float scalar, const Quaternion& q)
{
	return q * scalar;
}

// 定数クォータニオンの定義
inline const Quaternion Quaternion::Identity(0.0f, 0.0f, 0.0f, 1.0f);

#endif // QUATERNION_H
