#include "Scene.h"

#include "../assets/AssetFile.h"

#include "../metagen/MetaParse.h"

#include <windows.h>

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

	Vector<ParseMetaStruct> structDefs = ParseStructDefsFromFile("src/metagen/MetaParse.h");

	for (int i = 0; i < structDefs.count; i++) {
		StringStackBuffer<256> strName("struct %.*s:\n", structDefs.data[i].name.length, structDefs.data[i].name.start);
		OutputDebugStringA(strName.buffer);
		for (int j = 0; j < structDefs.data[i].fields.count; j++) {
			ParseMetaField mf = structDefs.data[i].fields.data[j];
			
			StringStackBuffer<256> fieldDef("\t%.*s", mf.type.length, mf.type.start);

			if (mf.typeParam.start != nullptr) {
				fieldDef.AppendFormat("<%.*s>", mf.typeParam.length, mf.typeParam.start);
			}

			for (int i = 0; i < mf.indirectionLevel; i++) {
				fieldDef.AppendFormat("*");
			}

			fieldDef.AppendFormat(" %.*s", mf.name.length, mf.name.start);

			if (mf.arrayCount != NOT_AN_ARRAY) {
				fieldDef.AppendFormat("[%d]", mf.arrayCount);
			}

			fieldDef.AppendFormat("\n");

			OutputDebugStringA(fieldDef.buffer);
		}
		OutputDebugStringA("\n");
	}

	PackAssetFile("assets", "assets.bna");

	res.LoadAssetFile("assets.bna");

	LoadLevel("Level1.lvl");

	{
		Entity* floorEnt = entities.GetById(3);

		BoxCollider* floorCol = phys.boxCols.CreateAndAdd();
		floorCol->entity = floorEnt->id;
		floorCol->position = Vector3(0, -0.25f, 0);
		floorCol->size = Vector3(5, 0.2f, 5);
	}

	{
		Entity* boxEnt = entities.GetById(4);

		BoxCollider* boxCol = phys.boxCols.CreateAndAdd();
		boxCol->entity = boxEnt->id;
		boxCol->position = Vector3(0, 0, 0);
		boxCol->size = Vector3(1, 1, 1);
	}
}

void Scene::Update() {
	player.Update();
}

void Scene::LoadLevel(const char* name) {
	int levelId = -1;
	bool exists = res.assetIdMap.LookUp(name, &levelId);
	ASSERT_MSG(exists, "Could not load scene '%s'", name);

	Level* level = res.levels.GetById(levelId);

	entities.Reset();
	transforms.Reset();
	res.drawCalls.Reset();
	phys.boxCols.Reset();
	phys.sphereCols.Reset();

	cam = level->cam;

	entities.SetSize(level->entities.count);
	MemCpy(entities.vals, level->entities.data, sizeof(Entity)*level->entities.count);
	entities.currentCount = level->entities.count;
	entities.currentMaxId = entities.vals[entities.currentCount - 1].id;

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

	transforms.SetSize(level->transforms.count);
	MemCpy(transforms.vals, level->transforms.data, sizeof(Transform)*level->transforms.count);
	transforms.currentCount = level->transforms.count;
	transforms.currentMaxId = transforms.vals[transforms.currentCount - 1].id;
}

void Scene::Render() {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	res.Render();
}
