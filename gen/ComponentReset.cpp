#include "../src/core/Scene.h"
#include "../src/util/LevelLoading.h"

void Scene::ResetComponents(){
	GlobalScene->phys.boxCols.Reset();
	GlobalScene->phys.sphereCols.Reset();
}
void Scene::LoadCustomComponentsFromLevel(const Level* level){
	LoadVectorToIDTracker<BoxCollider>(level->boxCols, GlobalScene->phys.boxCols);
	LoadVectorToIDTracker<SphereCollider>(level->sphereCols, GlobalScene->phys.sphereCols);
}
void Scene::SaveCustomComponentsToLevel(Level* level){
	LoadIDTrackerToVector<BoxCollider>(GlobalScene->phys.boxCols, level->boxCols);
	LoadIDTrackerToVector<SphereCollider>(GlobalScene->phys.sphereCols, level->sphereCols);
}
