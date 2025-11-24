#ifndef LIFECYCLE_SCENE_H
#define LIFECYCLE_SCENE_H

/*====================================================================


	Sceneクラス [scene.h]

							Author : Ryosuke Kageyama
							Date   : 2025/11/18
====================================================================*/


#include <vector>
#include <memory>

// 前方宣言
class GameObject;
class PhysicsSystem;

class Scene 
{
private:
	// 所有するGameObjectのvector
    std::vector<std::unique_ptr<GameObject>> m_GameObjects;
	// 新規作成されたGameObjectの一時保持vector
    std::vector<GameObject*> m_NewlyCreated;

public:
    void DestroyGameObject(GameObject* target);
    GameObject* AdoptGameObject(GameObject* raw);

    Scene();
	virtual ~Scene();

	virtual void Initialize();
    void ProcessAwake();
    void ProcessStart();
    void Update();
    void LateUpdate();
    void FixedUpdate();
    void Render();
};

#endif
