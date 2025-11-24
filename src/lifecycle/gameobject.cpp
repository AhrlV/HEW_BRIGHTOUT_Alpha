/*====================================================================


    GameObject 実装 [gameobject.cpp]

                                    Author : Ryosuke Kageyama
                                    Date   : 2025/11/18
====================================================================*/

#include "lifecycle/gameobject.h"
#include "physics/transform.h"
#include "lifecycle/world.h"


GameObject::GameObject()
{
    // 必ず Transform を追加
    AddComponent<TransForm>();
    // アクティブな Scene があれば自動登録
    World::RegisterGameObject(this);
}

GameObject::~GameObject() = default;

void GameObject::SetActive(bool active) { m_ActiveSelf = active; }
bool GameObject::IsActiveSelf() const { return m_ActiveSelf; }
bool GameObject::IsActiveInHierarchy() const { return m_ActiveSelf; }

void GameObject::AwakeNewComponents()
{
    for (auto* c : m_AddedComponents) c->Awake();
    m_AddedComponents.clear();
}

void GameObject::StartNewComponents()
{
    for (auto& c : m_Components) {
        if (m_StartedComponents.find(c.get()) == m_StartedComponents.end()) {
            if (IsActiveInHierarchy() && c->IsActive()) {
                c->Start();
                m_StartedComponents.insert(c.get());
            }
        }
    }
}

void GameObject::Update()
{
    if (!IsActiveInHierarchy()) return;
    for (auto& c : m_Components) if (c->IsActive()) c->Update();
}

void GameObject::LateUpdate()
{
    if (!IsActiveInHierarchy()) return;
    for (auto& c : m_Components) if (c->IsActive()) c->LateUpdate();
}

void GameObject::FixedUpdate()
{
    if (!IsActiveInHierarchy()) return;
    for (auto& c : m_Components) if (c->IsActive()) c->FixedUpdate();
}

void GameObject::Render()
{
    if (!IsActiveInHierarchy()) return;
    for (auto& c : m_Components) if (c->IsActive()) c->Render();
}
