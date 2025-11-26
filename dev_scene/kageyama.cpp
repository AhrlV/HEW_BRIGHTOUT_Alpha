#include "scene/kageyama.h"
#include "lifecycle/world.h"

/*====================================================================
	Kageyamaシーンの自動登録
	プログラム開始時にKageyamaシーンをアクティブシーンとして設定する。
====================================================================*/
static volatile int change = []()
{
	World::Instance().SetScene<Kageyama>();
	return 0;
}();
