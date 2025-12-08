#include "scene/kageyama.h"
#include "lifecycle.h"


static volatile int change = []()
{
	SceneManager::Instance().SetScene<Kageyama>();
	return 0;
}();
