#ifndef RIGIDBODY_H
#define RIGIDBODY_H
/*====================================================================

	Rigidbody (物理挙動用プレースホルダー) [rigidbody.h]

	Author : Ryosuke Kageyama
	Date   : 2025/11/19
====================================================================*/

#include "physicssystem.h"

class Rigidbody : public Component
{
public:
    // 簡易的なプレースホルダー。質量や速度などは後で追加。
    float mass = 1.0f;
    // 速度など必要なら追加予定
    // 生成/破壊時にPhysicsSystemへ登録/登録解除
    Rigidbody() { PhysicsSystem::Instance().RegisterRigidbody(this); }

    virtual ~Rigidbody() override { PhysicsSystem::Instance().UnregisterRigidbody(this); }

private:
    friend class PhysicsSystem; // 物理更新の呼び出しを許可
};

#endif
