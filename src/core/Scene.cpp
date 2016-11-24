#include "Scene.h"

#include "../assets/AssetFile.h"

#include "../metagen/MetaParse.h"

#include "../util/LevelLoading.h"

#include "../../ext/CppUtils/filesys.h"
#include "../../ext/CppUtils/unicode.h"

Scene* GlobalScene = nullptr;

Scene::Scene() : entities(100), transforms(120), res()	 {
	GlobalScene = this;
	frameRateIsLocked = false;
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

DrawCall* Scene::GetDrawCallForEntity(uint32 entityId) {
	for (int i = 0; i < res.drawCalls.currentCount; i++) {
		if (entityId == res.drawCalls.vals[i].entId) {
			return &res.drawCalls.vals[i];
		}
	}

	return nullptr;
}

Entity* Scene::CloneEntity(Entity* srcEnt) {
	ASSERT(srcEnt != nullptr);
	if (srcEnt != nullptr) {
		Transform* srcTrans = transforms.GetById(srcEnt->transform);
		ASSERT(srcTrans != nullptr);
		if (srcTrans != nullptr) {
			DrawCall* dc = GetDrawCallForEntity(srcEnt->id);
			Entity* newEnt = AddVisibleEntity(dc->matId, dc->meshId);
			Transform* newTrans = transforms.GetById(newEnt->transform);
			newTrans->position = srcTrans->position;
			newTrans->rotation = srcTrans->rotation;
			newTrans->scale = srcTrans->scale;

			for (int i = 0; i < CCT_Count; i++) {
				Component* comp = FindComponentByEntity<Component>((CustomComponentType)i, srcEnt->id);
				if (comp != nullptr) {
					Component* newComp = addComponentFuncs[i]();
					newComp->entity = newEnt->id;

					MemStream memStr;
					componentMemSerializeFuncs[i](comp, &memStr);
					componentMemDeserializeFuncs[i](newComp, &memStr);
				}
			}

			return newEnt;
		}
	}

	return nullptr;
}

void Scene::StartUp() {
	glEnable(GL_SCISSOR_TEST);
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	glEnable(GL_DEPTH_TEST);
	glViewport(0, 0, 1280, 720);
	glLoadIdentity();

	PackAssetFile("assets", "assets.bna");

	res.LoadAssetFile("assets.bna");

	gui.Init();

	LoadLevel("Level1.lvl");

	frameTimer.Reset();
}

void Scene::ShutDown(){
	gui.ShutDown();
}

void Scene::Update() {
	phys.AdvanceTime(GetDeltaTime());

	script.Update();

	UpdateCustomComponents();

	if (GlobalScene->input.KeyIsReleased('R')) {
		gui.ShutDown();
		PackAssetFile("assets", "assets.bna");
		res.LoadAssetFile("assets.bna");
		gui.Init();
	}

	for (int i = 0; i < deferredActions.count; i++) {
		ExecuteAction(deferredActions.data[i]);
	}

	frameTimer.Reset();
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

	currentLevel = levelId;
	Level* level = res.levels.GetById(currentLevel);

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

	// TODO: Less hacky way of this
	script.Start();

	for (int i = 0; i < res.armatures.currentCount; i++) {
		Armature* arm = &res.armatures.vals[i];

		for (int j = 0; j < res.drawCalls.currentCount; j++) {
			if (res.drawCalls.vals[j].matId == arm->modelId) {

				for (int k = 0; k < arm->boneCount; k++) {
					int boneAnims[3];
					for (int c = 0; c < 3; c++) {
						Component* comp = addComponentFuncs[CCT_AnimationInstance]();
						AnimationInstance* inst = (AnimationInstance*)comp;
						inst->entity = res.drawCalls.vals[j].entId;

						inst->flags = (ComponentFlags)((inst->flags) | CF_RuntimeOnly);

						inst->autoPlay = true;
						inst->isPlaying = true;
						inst->shouldLoop = true;
						inst->target.targetType = ATT_BoneTransform;
						inst->target.bone.armId = arm->id;
						inst->target.bone.boneIndex = k;

						boneAnims[c] = inst->id;
					}

					anims.animInsts.GetById(boneAnims[0])->animId = arm->boneTrackData[k].posTrack;
					anims.animInsts.GetById(boneAnims[1])->animId = arm->boneTrackData[k].rotTrack;
					anims.animInsts.GetById(boneAnims[2])->animId = arm->boneTrackData[k].scaleTrack;

					anims.animInsts.GetById(boneAnims[0])->target.bone.target3d = A3DT_Position;
					anims.animInsts.GetById(boneAnims[1])->target.bone.target3d = A3DT_Rotation;
					anims.animInsts.GetById(boneAnims[2])->target.bone.target3d = A3DT_Scale;
				}
			}
		}
	}
}

void Scene::SaveLevel(Level* level) {
	level->cam = cam;
	LoadIDTrackerToVector(entities, level->entities);
	LoadIDTrackerToVector(transforms, level->transforms);

	level->matIds.EnsureCapacity(entities.currentCount);
	level->meshIds.EnsureCapacity(entities.currentCount);

	level->matIds.count = entities.currentCount;
	level->meshIds.count = entities.currentCount;

	for (int i = 0; i < entities.currentCount; i++) {
		bool foundDC = false;

		for (int j = 0; j < res.drawCalls.currentCount; j++) {
			DrawCall dc = res.drawCalls.vals[j];
			if (dc.entId == entities.vals[i].id) {
				level->matIds.data[i] = dc.matId;
				level->meshIds.data[i] = dc.meshId;

				foundDC = true;
				break;
			}
		}

		if (!foundDC) {
			level->matIds.data[i] = -1;
			level->meshIds.data[i] = -1;
		}
	}

	SaveCustomComponentsToLevel(level);
}

void Scene::DestroyEntity(uint32 entId) {
	Action& act = deferredActions.EmplaceBack();
	act.type = AT_DestroyEntityImmediate;
	act.DestroyEntityImmediate_data.entId = entId;
}

// TODO: Parent-child destruction?
void DestroyEntityImmediate(uint32 entId) {
	Entity* ent = GlobalScene->entities.GetById(entId);
	if (ent != nullptr) {
		uint32 transformId = ent->transform;
		GlobalScene->entities.RemoveById(entId);
		GlobalScene->transforms.RemoveById(transformId);

		for (int i = 0; i < GlobalScene->res.drawCalls.currentCount; i++) {
			if (GlobalScene->res.drawCalls.vals[i].entId == entId) {
				uint32 dcId = GlobalScene->res.drawCalls.vals[i].id;
				GlobalScene->res.drawCalls.RemoveById(dcId);
				break;
			}
		}

		GlobalScene->DestroyCustomComponentsByEntity(entId);
	}
}

void Scene::Render() {
	glViewport((int)cam.xOffset, (int)cam.yOffset, (int)cam.widthPixels, (int)cam.heightPixels);
	glScissor((int)cam.xOffset, (int)cam.yOffset, (int)cam.widthPixels, (int)cam.heightPixels);

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glScissor((int)cam.xOffset, (int)cam.yOffset + 100, (int)cam.widthPixels, (int)cam.heightPixels - 100);

	glEnable(GL_DEPTH_TEST);

	res.Render();

	glDisable(GL_DEPTH_TEST);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	// Do gui stuff.
	// TODO: Separate function, so that editor doesn't do it?

	//gui.DrawTextLabel("Hello world", 1, 1, 300, 300);
	//gui.DrawTextLabel("This is the end.", 1, 1, 0, 0);
	//gui.DrawTextLabel("This is *not* the end.", 1, 1, 12, 12);

	glScissor((int)cam.xOffset, (int)cam.yOffset, (int)cam.widthPixels, (int)cam.heightPixels);

	static bool firstPass = true;
	static U32String unicodeText = {};
	if (firstPass) {
		int fileLength;
		char* unicodeEncoded = ReadTextFile("assets/strings/chinese_test.txt", &fileLength);
		unicodeText = DecodeUTF8(unicodeEncoded, fileLength);
		free(unicodeEncoded);
	}
	else {
		static int fc = 0;
		fc++;
		if (fc > 1200000) {
			unicodeText.start[0]++;
			fc = 0;
		}
	}
	firstPass = false;

	gui.DrawUnicodeLabel(unicodeText, 0, 18, 30, 30);

	gui.Render();

	glDisable(GL_BLEND);
}

// This is unfortunately needed, until we exclude editor headers from metagen for non-editor builds,
// Or autogen stubs if we mark things as editor only, or something.
// TODO: Yeah, that.
#if !defined(BNS_EDITOR)

struct Editor;

void EditorShiftButton(Editor* ed, uint32 buttonId) {
}

void EditorResetButton(Editor* ed, uint32 buttonId) {
}

#endif

