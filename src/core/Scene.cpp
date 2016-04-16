#include "Scene.h"

#include "../assets/AssetFile.h"

Scene* GlobalScene = nullptr;

Scene::Scene() : entities(100), transforms(120), gfx() {
	GlobalScene = this;
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

	DrawCall* dc = gfx.drawCalls.CreateAndAdd();
	dc->meshId = monkeyMesh;
	dc->matId = matId;

	Transform* trans = transforms.CreateAndAdd();
	trans->parent = -1;

	Entity* ent = entities.CreateAndAdd();
	ent->transform = trans->id;

	Transform* camTrans = transforms.CreateAndAdd();
	camTrans->position = Vector3(0, 1.2f, -4);
	camTrans->rotation = QUAT_IDENTITY;
	camTrans->scale = Vector3(1, 1, 1);
	camTrans->parent = -1;

	cam.transform = camTrans;

	dc->entId = ent->id;

	DrawCall* dc2 = gfx.drawCalls.CreateAndAdd();

	Transform* trans2 = transforms.CreateAndAdd();
	trans2->parent = trans->id;

	Entity* ent2 = entities.CreateAndAdd();
	ent2->transform = trans2->id;

	dc2->entId = ent2->id;
	dc2->matId = matId;
	dc2->meshId = boxMesh;

	int floorMatId = -1;
	gfx.assetIdMap.LookUp("floor.mat", &floorMatId);

	{
		DrawCall* floorDc = gfx.drawCalls.CreateAndAdd();
		floorDc->matId = floorMatId;

		floorDc->meshId = floorMesh;

		Entity* floorEnt = entities.CreateAndAdd();
		Transform* floorTrans = transforms.CreateAndAdd();
		floorTrans->parent = -1;
		floorTrans->position.y = -1;
		floorEnt->transform = floorTrans->id;
		floorDc->entId = floorEnt->id;

		BoxCollider* floorCol = phys.boxCols.CreateAndAdd();
		floorCol->entity = floorEnt->id;
		floorCol->position = Vector3(0, -0.25f, 0);
		floorCol->size = Vector3(5, 0.2f, 5);
	}

	{
		DrawCall* boxDc = gfx.drawCalls.CreateAndAdd();
		boxDc->matId = floorMatId;
		boxDc->meshId = boxMesh;

		Entity* boxEnt = entities.CreateAndAdd();
		Transform* boxTrans = transforms.CreateAndAdd();
		boxTrans->parent = -1;
		boxTrans->position = Vector3(2, -0.4f, 2);
		boxTrans->scale = Vector3(0.6f, 0.3f, 0.6f);
		boxEnt->transform = boxTrans->id;
		boxDc->entId = boxEnt->id;

		BoxCollider* boxCol = phys.boxCols.CreateAndAdd();
		boxCol->entity = boxEnt->id;
		boxCol->position = Vector3(0, 0, 0);
		boxCol->size = Vector3(1, 1, 1);
	}
}

void Scene::Update() {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	player.Update();

	gfx.Render();
}