
#include "../src/core/Scene.h"

void Scene::ResetComponents() {
	phys.boxCols.Reset();
	phys.sphereCols.Reset();
}
