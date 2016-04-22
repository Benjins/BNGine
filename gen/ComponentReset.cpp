#include "../src/core/Scene.h"

void Scene::ResetComponents(){
	GlobalScene->phys.boxCols.Reset();
	GlobalScene->phys.sphereCols.Reset();
}
