/*====================================================================


    Scene é¿ëï [scene.cpp]

                                    Author : Ryosuke Kageyama
                                    Date   : 2025/11/18
====================================================================*/


#include <algorithm>
#include "lifecycle/scene.h"
#include "lifecycle/gameobject.h"

Scene::Scene() = default;
Scene::~Scene() = default;


GameObject* Scene::AdoptGameObject(GameObject* raw)
{
    if (!raw) return nullptr;
    // ä˘Ç…ä«óùâ∫Ç≈Ç»Ç¢Ç±Ç∆ÇämîFÅièdï°ñhé~Åj
    auto it = std::find_if(m_GameObjects.begin(), m_GameObjects.end(), [raw](auto& ptr){ return ptr.get() == raw; });
    if (it != m_GameObjects.end()) return raw; // Ç∑Ç≈Ç…ìoò^çœÇ›
    m_GameObjects.emplace_back(std::unique_ptr<GameObject>(raw));
    m_NewlyCreated.push_back(raw);
    return raw;
}

void Scene::DestroyGameObject(GameObject* target)
{
    if (!target) return;
    m_NewlyCreated.erase(std::remove(m_NewlyCreated.begin(), m_NewlyCreated.end(), target), m_NewlyCreated.end());
    auto it = std::find_if(m_GameObjects.begin(), m_GameObjects.end(), [target](const std::unique_ptr<GameObject>& ptr){ return ptr.get() == target; });
    if (it != m_GameObjects.end()) {
        m_GameObjects.erase(it);
    }
}

void Scene::Initialize() {}

void Scene::ProcessAwake()
{
    for (auto* go : m_NewlyCreated) {
        go->AwakeNewComponents();
    }
    m_NewlyCreated.clear();
}

void Scene::ProcessStart()
{
    for (auto& gameobj : m_GameObjects) {
        gameobj->StartNewComponents();
    }
}

void Scene::Update()
{
    for (auto& gameobj : m_GameObjects) gameobj->Update();
}

void Scene::LateUpdate()
{
    for (auto& gameobj : m_GameObjects) gameobj->LateUpdate();
}

void Scene::FixedUpdate()
{
    for (auto& gameobj : m_GameObjects) gameobj->FixedUpdate();
}

void Scene::Render()
{
    for (auto& gameobj : m_GameObjects) gameobj->Render();
}
