#ifndef PHYSICS_SYSTEM_H
#define PHYSICS_SYSTEM_H
/*====================================================================

	PhysicsSystemシングルトン [physicssystem.h]

	Author : Ryosuke Kageyama
	Date   : 2025/11/19
====================================================================*/


// 前方宣言
class Rigidbody;
class Collider;

class PhysicsSystem
{
private:

    PhysicsSystem() = default;
    ~PhysicsSystem() = default;

public:
    PhysicsSystem(const PhysicsSystem&) = delete;
    PhysicsSystem& operator=(const PhysicsSystem&) = delete;

    static PhysicsSystem& Instance()
    {
        static PhysicsSystem inst;
        return inst;
    }

    // 物理ステップ
    void PhsicsUpdate();

private:
};

#endif
