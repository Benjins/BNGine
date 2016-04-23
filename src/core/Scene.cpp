#include "Scene.h"

#include "../assets/AssetFile.h"

#include "../metagen/MetaParse.h"

#include "../util/LevelLoading.h"

//#include <windows.h>

Scene* GlobalScene = nullptr;

Scene::Scene() : entities(100), transforms(120), res() {
	GlobalScene = this;
}

Entity* Scene::AddVisibleEntity(uint32 matId, uint32 meshId) {
	Entity* newEnt = entities.CreateAndAdd();
	Transform* newTrans = transforms.CreateAndAdd();
	newEnt->transform = newTrans->id;
	newTrans->entity = newEnt->id;

	ASSERT(res.materials.GetById(matId) != nullptr);
	ASSERT(res.meshes.GetById(meshId) != nullptr);

	DrawCall* newDc = res.drawCalls.CreateAndAdd();
	newDc->entId = newEnt->id;
	newDc->matId = matId;
	newDc->meshId = meshId;

	return newEnt;
}

void Scene::StartUp() {
	PackAssetFile("assets", "assets.bna");

	res.LoadAssetFile("assets.bna");

	LoadLevel("Level1.lvl");
}

void Scene::Update() {
	player.Update();
}

void Scene::Reset() {
	entities.Reset();
	transforms.Reset();
	res.drawCalls.Reset();

	ResetComponents();
}

void Scene::LoadLevel(const char* name) {
	int levelId = -1;
	bool exists = res.assetIdMap.LookUp(name, &levelId);
	ASSERT_MSG(exists, "Could not load scene '%s'", name);

	Level* level = res.levels.GetById(levelId);

	Reset();

	cam = level->cam;

	LoadVectorToIDTracker(level->entities, entities);

	ASSERT(level->meshIds.count == level->entities.count);
	ASSERT(level->matIds.count == level->entities.count);

	for (int i = 0; i < entities.currentCount; i++) {
		if (level->meshIds.Get(i) >= 0 && level->matIds.Get(i) >= 0) {
			DrawCall* dc = res.drawCalls.CreateAndAdd();
			dc->entId = entities.vals[i].id;
			dc->matId = level->matIds.Get(i);
			dc->meshId = level->meshIds.Get(i);
		}
	}

	LoadVectorToIDTracker(level->transforms, transforms);

	LoadCustomComponentsFromLevel(level);
}

void Scene::Render() {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	res.Render();
}
