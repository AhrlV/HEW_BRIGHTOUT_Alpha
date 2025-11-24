

#include "lifecycle.h"
#include "scene/kageyama.h"

volatile static inline int init = []()
	{
		ChangeScene<Kageyama>();
		return 0;
	}();