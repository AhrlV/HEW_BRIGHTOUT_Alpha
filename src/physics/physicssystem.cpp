/*====================================================================

	PhysicsSystem 実装 [physicssystem.cpp]

	Author : Ryosuke Kageyama
	Date   : 2025/11/19
====================================================================*/

#include "engine/lifecycle/lifecycle.h"

// 個別物理更新 (簡易的なプレースホルダー)
void PhysicsSystem::StepRigidbody(Rigidbody* rb)
{
    if (!rb || !rb->IsActive()) return;
    // TODO: 速度積分など
}

void PhysicsSystem::StepCollider(Collider* col)
{
    if (!col || !col->IsActive()) return;
    // TODO: 衝突形状の更新など
}
