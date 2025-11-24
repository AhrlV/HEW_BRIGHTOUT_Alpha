#ifndef PHYSICS_SYSTEM_H
#define PHYSICS_SYSTEM_H
/*====================================================================

	PhysicsSystemシングルトン [physicssystem.h]

	Author : Ryosuke Kageyama
	Date   : 2025/11/19
====================================================================*/

#include <vector>


// 前方宣言
class Rigidbody;
class Collider;

class PhysicsSystem
{
private:
    std::vector<Rigidbody*> m_Rigidbodies;
    std::vector<Collider*>  m_Colliders;

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

    // 登録API（生成/破壊時に各Componentが自動で呼ぶ）
    void RegisterRigidbody(Rigidbody* rb)
    {
        if (rb) m_Rigidbodies.push_back(rb);
    }
    void RegisterCollider(Collider* col)
    {
        if (col) m_Colliders.push_back(col);
    }

    void UnregisterRigidbody(Rigidbody* rb)
    {
        if (!rb) return;
        auto it = std::remove(m_Rigidbodies.begin(), m_Rigidbodies.end(), rb);
        m_Rigidbodies.erase(it, m_Rigidbodies.end());
    }
    void UnregisterCollider(Collider* col)
    {
        if (!col) return;
        auto it = std::remove(m_Colliders.begin(), m_Colliders.end(), col);
        m_Colliders.erase(it, m_Colliders.end());
    }


    // 物理ステップ: 登録された要素に対してPhysicsステップを実行
    void PhsicsUpdate()
    {
        // 動的配列なので、イテレーション中の削除は想定しない運用とする
        for (auto* rb : m_Rigidbodies) if (rb) StepRigidbody(rb);
        for (auto* col : m_Colliders)  if (col) StepCollider(col);
        // TODO: コリジョン判定・解決などをここに追加
    }

private:
    void StepRigidbody(Rigidbody* /*rb*/) {};
    void StepCollider(Collider* /*col*/) {};
};

#endif
