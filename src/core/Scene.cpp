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
	glEnable(GL_SCISSOR_TEST);

	PackAssetFile("assets", "assets.bna");

	res.LoadAssetFile("assets.bna");

	gui.Init();
	
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

String str = "Never again!#$%!#%!#@235231";

String str2 = "This isn't even my final form.";

void Scene::Render() {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glEnable(GL_DEPTH_TEST);

	res.Render();

	glDisable(GL_DEPTH_TEST);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	gui.DrawTextLabel("Hello world", 1, 1, 300, 300);
	gui.DrawTextLabel("This is the end.", 1, 1, 0, 0);
	gui.DrawTextLabel("This is *not* the end.", 1, 1, 12, 12);

	{
		String newStr = gui.TextInput(str, 1, 12, 400, 120, 320);

		str = newStr;
	}

	{
		String newStr = gui.TextInput(str2, 1, 12, 400, 320, 320);

		str2 = newStr;
	}

	glDisable(GL_BLEND);
}
