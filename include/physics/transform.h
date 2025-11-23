/*====================================================================

	Transform / Math primitives [transform.h]

	Author : Ryosuke Kageyama
	Date   : 2025/11/19
====================================================================*/


#ifndef TRANSFORM_H
#define TRANSFORM_H

#include "lifecycle/component.h"

// 3Dベクトル(簡易的)
class Vector3
{
public:
	float x = 0.0f;
	float y = 0.0f;
	float z = 0.0f;
};
// クォータニオン(簡易的)
class Quaternion
{
public:
	float x = 0.0f;
	float y = 0.0f;
	float z = 0.0f;
};

// Transformコンポーネント(親子関係は未実装)
class TransForm : public Component
{
public:
	Vector3 position;
	Quaternion rotation;
	Vector3 scale;
};


#endif // TRANSFORM_H
