#ifndef COLLIDER_H
#define COLLIDER_H
/*====================================================================

    Collider (衝突判定用プレースホルダー) [collider.h]

    Author : Ryosuke Kageyama
    Date   : 2025/11/19
====================================================================*/

#include "lifecycle/component.h"
#include "physics/physicssystem.h"

class Collider : public Component
{
public:
	// 生成/破壊時にPhysicsSystemへ登録/登録解除
    Collider() { PhysicsSystem::Instance().RegisterCollider(this); }
    virtual ~Collider() override { PhysicsSystem::Instance().UnregisterCollider(this); }
private:
    friend class PhysicsSystem; // 物理更新の呼び出しを許可
};

#endif
