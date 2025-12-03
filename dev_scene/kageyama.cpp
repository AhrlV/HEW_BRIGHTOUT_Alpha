#include "scene/kageyama.h"
#include "lifecycle/scene_manager.h"

/*====================================================================
	Kageyamaシーンの自動登録
	プログラム開始時にKageyamaシーンをアクティブシーンとして設定する。
====================================================================*/
static volatile int change = []()
{
	SceneManager::Instance().SetScene<Kageyama>();
	return 0;
}();
