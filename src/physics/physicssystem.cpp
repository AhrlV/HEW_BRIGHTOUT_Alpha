#include "physics/physicssystem.h"
#include "lifecycle/scene.h"
#include "physics/rigidbody.h"
#include "physics/collider.h"
#include "physics/transform.h"
#include "lifecycle/scene_manager.h"
#include <vector>

void PhysicsSystem::PhsicsUpdate()
{
	Scene* scene = SceneManager::Instance().GetActiveScene();

	// Collider/Rigidbodyコンポーネントを持つ全GameObjectを取得
	std::vector<Collider*> colliders = scene->GetComponentsByType<Collider>();
	std::vector<Rigidbody*> rigidbodies = scene->GetComponentsByType<Rigidbody>();


	// TODO: コリジョン判定・解決などをここに追加
}
