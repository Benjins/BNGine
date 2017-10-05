#include "Scene.h"

#include "../assets/AssetFile.h"

#include "../gfx/GLExtInit.h"

#include "../gui/Debug3DGUI.h"

#include "../metagen/MetaParse.h"

#include "../util/LevelLoading.h"

#include "../../ext/CppUtils/filesys.h"
#include "../../ext/CppUtils/unicode.h"

Scene* GlobalScene = nullptr;

ConfigVarTable globalConfigTable;

CONFIG_VAR(float, sceneTimeScale, "time_scale", 1.0f);
CONFIG_VAR(bool, wireFrameRendering, "wire_render", false);

Scene::Scene() : entities(100), transforms(120), res() {
	GlobalScene = this;
	frameRateIsLocked = false;
}

Entity* Scene::AddEntity() {
	Entity* newEnt = entities.CreateAndAdd();
	Transform* newTrans = transforms.CreateAndAdd();
	newEnt->transform = IDHandle<Transform>(newTrans->id);
	newTrans->entity = IDHandle<Entity>(newEnt->id);

	return newEnt;
}

Entity* Scene::AddEntityWithId(uint32 id) {
	Entity* newEnt = entities.AddWithId(id);
	Transform* newTrans = transforms.CreateAndAdd();
	newEnt->transform = IDHandle<Transform>(newTrans->id);
	newTrans->entity = IDHandle<Entity>(newEnt->id);

	return newEnt;
}

Entity* Scene::AddVisibleEntityWithIdAndTransId(uint32 id, uint32 transId, IDHandle<Material> matId, IDHandle<Mesh> meshId) {
	Entity* newEnt = AddEntityWithIdAndTrandId(id, transId);
	AddVisibleEntityByEntityPtr(newEnt, matId, meshId);

	return newEnt;
}

Entity* Scene::AddEntityWithIdAndTrandId(uint32 id, uint32 transId) {
	Entity* newEnt = entities.AddWithId(id);
	Transform* newTrans = transforms.AddWithId(transId);
	newEnt->transform = IDHandle<Transform>(newTrans->id);
	newTrans->entity = IDHandle<Entity>(newEnt->id);

	return newEnt;
}

void Scene::AddVisibleEntityByEntityPtr(Entity* newEnt, IDHandle<Material> matId, IDHandle<Mesh> meshId) {
	ASSERT(res.materials.GetById(matId) != nullptr);
	ASSERT(res.meshes.GetById(meshId) != nullptr);

	DrawCall* newDc = res.drawCalls.CreateAndAdd();
	newDc->entId = IDHandle<Entity>(newEnt->id);
	newDc->matId = matId;
	newDc->meshId = meshId;
}

Entity* Scene::AddVisibleEntity(IDHandle<Material> matId, IDHandle<Mesh> meshId) {
	Entity* newEnt = AddEntity();
	AddVisibleEntityByEntityPtr(newEnt, matId, meshId);

	return newEnt;
}

Entity* Scene::AddVisibleEntityWithId(uint32 entId, IDHandle<Material> matId, IDHandle<Mesh> meshId) {
	Entity* newEnt = AddEntityWithId(entId);
	AddVisibleEntityByEntityPtr(newEnt, matId, meshId);

	return newEnt;
}

DrawCall* Scene::GetDrawCallForEntity(IDHandle<Entity> entityId) {
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
			DrawCall* dc = GetDrawCallForEntity(IDHandle<Entity>(srcEnt->id));
			Entity* newEnt = nullptr;
			if (dc != nullptr) {
				newEnt = AddVisibleEntity(dc->matId, dc->meshId);
			}
			else {
				newEnt = AddEntity();
			}

			Transform* newTrans = transforms.GetById(newEnt->transform);
			newTrans->position = srcTrans->position;
			newTrans->rotation = srcTrans->rotation;
			newTrans->scale = srcTrans->scale;

			for (int i = 0; i < CCT_Count; i++) {
				Component* comp = FindComponentByEntity<Component>((CustomComponentType)i, IDHandle<Entity>(srcEnt->id));
				if (comp != nullptr) {
					Component* newComp = addComponentFuncs[i]();
					newComp->entity = IDHandle<Entity>(newEnt->id);

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

	LoadLevel("Level3.lvl");

	IDHandle<Prefab> playerPrefab;
	res.assetIdMap.LookUp("player.bnp", &playerPrefab.id);

	IDHandle<Prefab> enemyPlayerPrefab;
	res.assetIdMap.LookUp("enemy_player.bnp", &enemyPlayerPrefab.id);
	net.playerPrefab = enemyPlayerPrefab;

	for (int i = 0; i < gameplay.prefabInsts.currentCount; i++) {
		if (gameplay.prefabInsts.vals[i].prefab == playerPrefab) {
			net.RegisterPlayer(gameplay.prefabInsts.vals[i].entity);
			break;
		}
	}

	{
		auto& rb = phys.physModules.vals[0].sys.rigidbodies.EmplaceBack();
		InitPhys2Rigidbody(&rb, 10.5f, 1.0f);
		rb.position = Vector3(0, 5.5f, 0);
		rb.orientation = Quaternion(X_AXIS, 0.2f);
	}

	{
		auto& rb = phys.physModules.vals[0].sys.rigidbodies.EmplaceBack();
		InitPhys2Rigidbody(&rb, 0.5f, 0.4f);
		rb.position = Vector3(4, 5.5f, 0);
		rb.orientation = Quaternion(Z_AXIS, 0.7f);
	}

	StartUpCustomComponents();

	// TODO: Only do this if needed
	//net.Initialize(0);

	frameTimer.Reset();
	totalTimer.Reset();
}

void Scene::ShutDown(){
	gui.ShutDown();
}

void Scene::ReloadAssets() {
	gui.ShutDown();
	PackAssetFile("assets", "assets.bna");
	res.LoadAssetFile("assets.bna");
	gui.Init();
}

void Scene::Update() {
	phys.AdvanceTime(GetDeltaTime());

	script.Update();

	UpdateCustomComponents();

	if (!gameConsole.shouldDisplayConsole && GlobalScene->input.KeyIsReleased('R')) {
		ReloadAssets();
	}

	// TODO: Remove this until it's stable
	/*
	if (!gameConsole.shouldDisplayConsole && GlobalScene->input.KeyIsReleased('P')) {
		IPV4Addr addr = IPV4Addr(127, 0, 0, 1, net.debugPortToConnectTo);
		net.OpenNewConnection(addr);
	}
	*/

	for (int i = 0; i < deferredActions.count; i++) {
		ExecuteAction(deferredActions.data[i]);
	}

	net.NetworkUpdate();

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
	Level* level = res.levels.GetByIdNum(currentLevel);

	Reset();

	cam = level->cam;

	LoadVectorToIDTracker(level->entities, entities);

	ASSERT(level->meshIds.count == level->entities.count);
	ASSERT(level->matIds.count == level->entities.count);

	for (int i = 0; i < entities.currentCount; i++) {
		if (level->meshIds.Get(i) >= 0 && level->matIds.Get(i) >= 0) {
			DrawCall* dc = res.drawCalls.CreateAndAdd();
			dc->entId = IDHandle<Entity>(entities.vals[i].id);
			dc->matId = IDHandle<Material>(level->matIds.Get(i));
			dc->meshId = IDHandle<Mesh>(level->meshIds.Get(i));
		}
	}

	LoadVectorToIDTracker(level->transforms, transforms);

	LoadCustomComponentsFromLevel(level);

	for (int i = 0; i < level->prefabInsts.count; i++) {
		PrefabInstance* inst = &level->prefabInsts.data[i];
		Prefab* prefab = res.prefabs.GetById(inst->prefabId);

		Entity* ent = prefab->InstantiateWithIdAndTransId(inst->instanceId, inst->instanceTransformId, inst->pos, inst->rot);
		transforms.GetById(ent->transform)->parent = IDHandle<Transform>(inst->parentTransform);
	}

	// TODO: Less hacky way of this
	script.Start();

	for (int i = 0; i < res.armatures.currentCount; i++) {
		Armature* arm = &res.armatures.vals[i];

		for (int j = 0; j < res.drawCalls.currentCount; j++) {
			if (res.drawCalls.vals[j].meshId == arm->modelId) {

				for (int k = 0; k < arm->boneCount; k++) {
					int trackIds[] = {
						arm->boneTrackData[k].posTrack,
						arm->boneTrackData[k].rotTrack,
						arm->boneTrackData[k].scaleTrack
					};

					Animation3DTarget targetTypes[] = {
						A3DT_Position,
						A3DT_Rotation,
						A3DT_Scale
					};

					for (int c = 0; c < 3; c++) {
						Component* comp = addComponentFuncs[CCT_AnimationInstance]();
						AnimationInstance* inst = (AnimationInstance*)comp;
						inst->entity = res.drawCalls.vals[j].entId;

						inst->flags = (ComponentFlags)((inst->flags) | CF_RuntimeOnly);

						inst->autoPlay = true;
						inst->isPlaying = true;
						inst->shouldLoop = true;

						AnimationBoneTarget bone;
						bone.armId = IDHandle<Armature>(arm->id);
						bone.boneIndex = k;
						bone.target3d = targetTypes[c];

						inst->target = bone;
						inst->animId = IDHandle<AnimationTrack>(trackIds[c]);
					}
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
			if (dc.entId.id == entities.vals[i].id) {
				level->matIds.data[i] = dc.matId.id;
				level->meshIds.data[i] = dc.meshId.id;

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

void Scene::DestroyEntity(IDHandle<Entity> entId) {
	Action& act = deferredActions.EmplaceBack();
	act.type = AT_DestroyEntityImmediate;
	act.DestroyEntityImmediate_data.entId = entId.id;
}

// TODO: Parent-child destruction?
void DestroyEntityImmediate(uint32 entId) {
	Entity* ent = GlobalScene->entities.GetByIdNum(entId);
	if (ent != nullptr) {
		IDHandle<Transform> transformId = ent->transform;
		GlobalScene->entities.RemoveByIdNum(entId);
		GlobalScene->transforms.RemoveById(transformId);

		for (int i = 0; i < GlobalScene->res.drawCalls.currentCount; i++) {
			if (GlobalScene->res.drawCalls.vals[i].entId.id == entId) {
				uint32 dcId = GlobalScene->res.drawCalls.vals[i].id;
				GlobalScene->res.drawCalls.RemoveByIdNum(dcId);
				break;
			}
		}

		GlobalScene->DestroyCustomComponentsByEntity(IDHandle<Entity>(entId));
	}
}

void RenderSkyBox() {
	int skyBoxMatId;
	bool hasSkyBox = GlobalScene->res.assetIdMap.LookUp("skybox.mat", &skyBoxMatId);

	int ballMeshId;
	bool hasBall = GlobalScene->res.assetIdMap.LookUp("ball.obj", &ballMeshId);

	ASSERT(hasSkyBox);
	ASSERT(hasBall);

	GLint oldCullFaceMode;
	glGetIntegerv(GL_CULL_FACE_MODE, &oldCullFaceMode);
	GLint oldDepthFuncMode;
	glGetIntegerv(GL_DEPTH_FUNC, &oldDepthFuncMode);

	glCullFace(GL_FRONT);
	glDepthFunc(GL_LEQUAL);

	Material* skyBoxMat = GlobalScene->res.materials.GetByIdNum(skyBoxMatId);
	Mesh* ballMesh = GlobalScene->res.meshes.GetByIdNum(ballMeshId);

	CubeMap* skyBox = GlobalScene->res.cubeMaps.GetById(skyBoxMat->cubeMap);
	ASSERT(skyBox != nullptr);
	skyBox->Bind(GL_TEXTURE0 + skyBoxMat->texCount);

	skyBoxMat->SetIntUniform("_cubeMap", skyBoxMat->texCount);

	Mat4x4 camera = GlobalScene->cam.GetCameraMatrix();
	Mat4x4 persp = GlobalScene->cam.GetPerspectiveMatrix();
	skyBoxMat->SetMatrix4Uniform("_cameraMatrix", camera);
	skyBoxMat->SetMatrix4Uniform("_perspMatrix", persp);

	Program* prog = GlobalScene->res.programs.GetById(skyBoxMat->programId);
	glUseProgram(prog->programObj);
	skyBoxMat->UpdateUniforms();

	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, ballMesh->posVbo);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);

	glDrawArrays(GL_TRIANGLES, 0, ballMesh->faces.count * 3);

	glDisableVertexAttribArray(0);

	glCullFace(oldCullFaceMode);
	glDepthFunc(oldDepthFuncMode);
}

void Scene::SetupDebugRenderUniforms() {
	Mat4x4 camera = cam.GetCameraMatrix();
	Mat4x4 persp = cam.GetPerspectiveMatrix();

	{
		int colMatId = -1;
		res.assetIdMap.LookUp("color.mat", &colMatId);
		Material* mat = res.materials.GetByIdNum(colMatId);
		ASSERT(mat != nullptr);

		mat->SetMatrix4Uniform("_camMatrix", camera);
		mat->SetMatrix4Uniform("_perspMatrix", persp);
	}

	{
		int debugColMatId = -1;
		res.assetIdMap.LookUp("debugCol.mat", &debugColMatId);
		Material* mat = res.materials.GetByIdNum(debugColMatId);
		ASSERT(mat != nullptr);

		mat->SetMatrix4Uniform("_camMatrix", camera);
		mat->SetMatrix4Uniform("_perspMatrix", persp);
	}
}

void Scene::Render() {
	glViewport((int)cam.xOffset, (int)cam.yOffset, (int)cam.widthPixels, (int)cam.heightPixels);
	glScissor((int)cam.xOffset, (int)cam.yOffset, (int)cam.widthPixels, (int)cam.heightPixels);

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	//glScissor((int)cam.xOffset, (int)cam.yOffset + 100, (int)cam.widthPixels, (int)cam.heightPixels - 100);

	glEnable(GL_DEPTH_TEST);

	if (wireFrameRendering) {
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	}
	else {
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	}

	res.Render();

	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	BNS_VEC_FOREACH(phys.physModules.vals[0].sys.rigidbodies) {
		Transform dbgTrans;
		dbgTrans.parent.id = -1;
		dbgTrans.position = ptr->position;
		dbgTrans.rotation = ptr->orientation;
		DebugDrawWireCube(Vector3(0, 0, 0), Vector3(1, 1, 1) * ptr->boxSize, &dbgTrans);
	}

	RenderSkyBox();

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	NavMesh* currNavMesh = nav.navMeshes.GetById(nav.currentNavMesh);
	if (currNavMesh != nullptr) {
		//glEnable(GL_BLEND);

		SetupDebugRenderUniforms();

		currNavMesh->DebugRender();

		//glDisable(GL_BLEND);
	}

	glDisable(GL_DEPTH_TEST);

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
		for (int i = 0; i < unicodeText.length; i++) {
			int codepoint = unicodeText.start[i];
			OutputDebugStringA(StringStackBuffer<256>("Code point(%2d): %5d %#04x\n", i, codepoint, codepoint).buffer);
		}
		free(unicodeEncoded);
	}

	firstPass = false;

	static float x = 150;
	static float width = 900;

	width -= 0.2f;

	//gui.DrawUnicodeLabel(unicodeText, IDHandle<UniFont>(0), 18, x, 30, width);
	//gui.DrawTextLabel("Hi", IDHandle<BitmapFont>(0), 24, 40, 80, 100);

	// TODO: Hack
	if (input.KeyIsReleased(KC_BackTick)) {
		// TODO: Focus the console input as well?
		// Also defocus it?
		gameConsole.shouldDisplayConsole = !gameConsole.shouldDisplayConsole;
		// TODO: HACK: Better way to signal this?
		gameplay.players.vals[0].disablePlayerInput = !gameplay.players.vals[0].disablePlayerInput;
	}

	gameConsole.Render(&gui);

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

void EditorAddStringPicker(Editor* ed, int enumIndex, int buttonId, Vector2 pos, Vector2 size) {

}

void EditorPrintEnum(Editor* ed, int pickerId, int buttonId) {

}

#endif

