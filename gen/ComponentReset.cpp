#include "../src/core/Scene.h"
#include "../src/util/LevelLoading.h"

void Scene::ResetComponents(){
	GlobalScene->phys.sphereCols.Reset();
	GlobalScene->phys.boxCols.Reset();
}
void Scene::LoadCustomComponentsFromLevel(const Level* level){
	LoadVectorToIDTracker<SphereCollider>(level->sphereCols, GlobalScene->phys.sphereCols);
	LoadVectorToIDTracker<BoxCollider>(level->boxCols, GlobalScene->phys.boxCols);
}
