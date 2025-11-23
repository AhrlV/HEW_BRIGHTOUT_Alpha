#ifndef GAME_H
#define GAME_H

#include "lifecycle.h"
#include "physics.h"
#include <iostream>

// プレイヤー移動コンポーネント
class PlayerMove : public Component
{
public:
	float m_Speed = 0.1f;

public:
	void Update() override
	{
		// 自動で付与されているTransformを取得
		TransForm* tf = m_Owner->GetComponent<TransForm>();
		if (tf)
		{
			tf->position.x += m_Speed * Time::DeltaTime();
			std::cout << "Player Position X: " << tf->position.x << std::endl;
		}
	}
};

// ゲームシーンクラス
class Game : public Scene
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