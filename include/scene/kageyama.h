

#ifndef KAGEYAMA_H
#define KAGEYAMA_H

#include "lifecycle.h"
#include "physics.h"

// プレイヤー移動コンポーネント
class PlayerMove : public Component
{
public:
	float m_Speed = 0.1f;
	float elapsedTime = 0.0f;

public:
	void Update() override
	{
		// 自動で付与されているTransformを取得
		TransForm* tf = m_Owner->GetComponent<TransForm>();
		if (tf)
		{
			tf->position.x += m_Speed * Time::DeltaTime();
		}

		if (elapsedTime >= 5.0f)
		{
			elapsedTime = 0.0f;
		}
		elapsedTime += Time::DeltaTime();
	}
};

class Kageyama : public Scene
{
public:
	void Initialize() override
	{
		// new するだけで自動登録 & Transform付与
		GameObject* player = new GameObject();
		player->AddComponent<PlayerMove>();
	}
};


#endif
