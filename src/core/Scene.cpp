#include "Scene.h"

#include "../assets/AssetFile.h"

Scene* GlobalScene = nullptr;

Scene::Scene() : entities(100), transforms(120), gfx() {
	GlobalScene = this;
}

Entity* Scene::AddVisibleEntity(uint32 matId, uint32 meshId) {
	Entity* newEnt = entities.CreateAndAdd();
	Transform* newTrans = transforms.CreateAndAdd();
	newEnt->transform = newTrans->id;
	newTrans->entity = newEnt->id;

	ASSERT(gfx.materials.GetById(matId) != nullptr);
	ASSERT(gfx.meshes.GetById(meshId) != nullptr);

	DrawCall* newDc = gfx.drawCalls.CreateAndAdd();
	newDc->entId = newEnt->id;
	newDc->matId = matId;
	newDc->meshId = meshId;

	return newEnt;
}

void Scene::StartUp() {
	PackAssetFile("assets", "assets.bna");

	gfx.LoadAssetFile("assets.bna");

	int matId = -1;
	gfx.assetIdMap.LookUp("standard.mat", &matId);

	int boxMesh = -1;
	gfx.assetIdMap.LookUp("test_2.obj", &boxMesh);

	int monkeyMesh = -1;
	gfx.assetIdMap.LookUp("monkey.obj", &monkeyMesh);

	int floorMesh = -1;
	gfx.assetIdMap.LookUp("floor.obj", &floorMesh);

	AddVisibleEntity(matId, monkeyMesh);

	Transform* camTrans = transforms.CreateAndAdd();
	camTrans->position = Vector3(0, 1.2f, -4);
	camTrans->rotation = QUAT_IDENTITY;
	camTrans->scale = Vector3(1, 1, 1);
	camTrans->parent = -1;

	cam.transform = camTrans;

	AddVisibleEntity(matId, boxMesh);

	int floorMatId = -1;
	gfx.assetIdMap.LookUp("floor.mat", &floorMatId);

	{
		Entity* floorEnt = AddVisibleEntity(floorMatId, floorMesh);
		Transform* floorTrans = transforms.GetById(floorEnt->transform);
		floorTrans->position.y = -1;

		BoxCollider* floorCol = phys.boxCols.CreateAndAdd();
		floorCol->entity = floorEnt->id;
		floorCol->position = Vector3(0, -0.25f, 0);
		floorCol->size = Vector3(5, 0.2f, 5);
	}

	{
		Entity* boxEnt = AddVisibleEntity(floorMatId, boxMesh);
		Transform* boxTrans = transforms.GetById(boxEnt->transform);
		boxTrans->position = Vector3(2, -0.4f, 2);
		boxTrans->scale = Vector3(0.6f, 0.3f, 0.6f);

		BoxCollider* boxCol = phys.boxCols.CreateAndAdd();
		boxCol->entity = boxEnt->id;
		boxCol->position = Vector3(0, 0, 0);
		boxCol->size = Vector3(1, 1, 1);
	}
}

void Scene::Update() {
	player.Update();
}

void Scene::Render() {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	gfx.Render();
}