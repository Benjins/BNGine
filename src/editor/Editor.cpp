#include "Editor.h"

#include "../gfx/GLExtInit.h"
#include "../util/Serialization.h"

#include "../metagen/ComponentMeta.h"
#include "../metagen/MetaStruct.h"

#include "../../ext/3dbasics/Vector4.h"
#include "../../ext/CppUtils/filesys.h"
#include "../../ext/CppUtils/idbase.h"

#include <float.h>

const char* viewNames[EV_Count] = {
	"Scene",
	"Prefab"
};

void Editor::Update() {
	//scene.Update();

	if (gui.textInputState.activeIndex == -1){
		if (!scene.input.KeyIsDown(KC_Control)
			&& !scene.input.KeyIsDown(KC_Shift)) {
			Vector3 moveVec;
			if (scene.input.KeyIsDown('W')) {
				moveVec = moveVec + editorCamTrans.Forward();
			}
			if (scene.input.KeyIsDown('S')) {
				moveVec = moveVec - editorCamTrans.Forward();
			}
			if (scene.input.KeyIsDown('A')) {
				moveVec = moveVec - editorCamTrans.Right();
			}
			if (scene.input.KeyIsDown('D')) {
				moveVec = moveVec + editorCamTrans.Right();
			}

			if (scene.input.KeyIsDown('Q')) {
				moveVec = moveVec + Y_AXIS;
			}
			if (scene.input.KeyIsDown('Z')) {
				moveVec = moveVec - Y_AXIS;
			}

			editorCamTrans.position = editorCamTrans.position + moveVec / 10;
		}

		if (scene.input.KeyIsDown(KC_Control)) {
			if (scene.input.KeyIsReleased('R')) {
				gizmoType = EG_Rotation;
			}
			else if (scene.input.KeyIsReleased('P')) {
				gizmoType = EG_Position;
			}
			else if (scene.input.KeyIsReleased('S')) {
				SaveScene();
			}
			else if (scene.input.KeyIsReleased('C')) {
				if (selectedEntity != -1) {
					copyPasteData.type = CPT_Entity;
					copyPasteData.entityId = selectedEntity;
				}
			}
			else if (scene.input.KeyIsReleased('V')) {
				if (copyPasteData.type == CPT_Entity && copyPasteData.entityId != -1) {
					Entity* srcEnt = scene.entities.GetByIdNum(copyPasteData.entityId);
					if (srcEnt != nullptr) {
						Entity* newEnt = scene.CloneEntity(srcEnt);
						selectedEntity = newEnt->id;
					}
				}
			}
		}
	}

	if (scene.input.cursorX > leftBarWidth && scene.input.cursorX < cam.widthPixels - rightBarWidth 
		&& scene.input.cursorY > topBarHeight
		&& scene.input.MouseButtonIsDown(MouseButton::SECONDARY)) {
		cameraCursorX -= scene.input.cursorDeltaX;
		cameraCursorY -= scene.input.cursorDeltaY;
		
		editorCamTrans.rotation = Quaternion(Y_AXIS, cameraCursorX / 80) * Quaternion(X_AXIS, cameraCursorY / 80);
	}

	if (currentView == EV_Scene) {
		if (scene.input.MouseButtonIsReleased(MouseButton::PRIMARY)) {
			if (scene.input.cursorX > leftBarWidth && scene.input.cursorX < cam.widthPixels - rightBarWidth
				&& scene.input.cursorY > topBarHeight) {

				selectedEntity = GetSelectedEntity((int)scene.input.cursorX, (int)scene.input.cursorY);
				selectedAxis = -1;
			}
		}
		else if (scene.input.MouseButtonIsPressed(MouseButton::PRIMARY)) {
			if (scene.input.cursorX > leftBarWidth && scene.input.cursorX < cam.widthPixels - rightBarWidth
				&& scene.input.cursorY > topBarHeight && selectedEntity != -1) {

				if (selectedEntity == GetSelectedEntity((int)scene.input.cursorX, (int)scene.input.cursorY)) {
					HandleGizmoClick();
				}
			}
		}
		else if (scene.input.MouseButtonIsDown(MouseButton::PRIMARY)) {
			if (scene.input.cursorX > leftBarWidth && scene.input.cursorX < cam.widthPixels - rightBarWidth
				&& scene.input.cursorY > topBarHeight) {
				if (selectedEntity != -1) {
					Entity* selected = scene.entities.GetByIdNum(selectedEntity);

					HandleGizmoDrag(selected);
				}
			}
		}
	}
}

void Editor::SwitchView(EditorView newView) {
	if (currentView != newView) {

		// Guess we don't actually need to do anything here yet?

		//if (newView == EV_Scene) {
		//	scene.LoadLevel("Level1.lvl");
		//}
		//else if (newView == EV_Prefab) {
		//	
		//}

		currentView = newView;
	}
}

void Editor::HandleGizmoClick() {
	Vector3 cameraPos = editorCamTrans.GetGlobalPosition();
	Vector3 ray = ScreenSpaceCoordsToRay(scene.input.cursorX, scene.input.cursorY);

	Entity* selected = scene.entities.GetByIdNum(selectedEntity);
	Transform* selectedTrans = scene.transforms.GetById(selected->transform);
	Vector3 objectPos = selectedTrans->GetGlobalPosition();

	Vector3 cameraToObj = objectPos - cameraPos;
	Vector3 projectionDiff = VectorProject(cameraToObj, ray) - cameraToObj;

	if (gizmoType == EG_Position) {
		int bestIndex = -1;
		float bestOverlap = 0;
		for (int i = 0; i < 3; i++) {
			if (projectionDiff[i] > bestOverlap) {
				bestIndex = i;
				bestOverlap = projectionDiff[i];
			}
		}

		if (bestIndex >= 0) {
			selectedAxis = bestIndex;
			selectionOffset.position = Vector3(0, 0, 0);
			selectionOffset.position[bestIndex] = bestOverlap;
		}
	}
	else if (gizmoType == EG_Rotation) {
		int bestIndex = -1;
		float bestError = 1.2f; // Used for sensitivity, i.e. how close to the gimbal you need to select
		Vector3 errorVec(-1,-1,-1);
		for (int i = 0; i < 3; i++) {
			Vector3 normal;
			normal[i] = 1.0f;
			normal = selectedTrans->GetLocalToGlobalMatrix().MultiplyAsDirection(normal);

			float rayNormalOverlap = BNS_ABS(DotProduct(ray, normal));
			if (rayNormalOverlap < 0) {
				// Unlikely, since we're facing it, but just to be safe
				continue;
			}
			else {
				float planeProjectDistance = rayNormalOverlap / cameraToObj.MagnitudeSquared();
				Vector3 planeProjection = cameraPos + ray * planeProjectDistance;
				float projectionDistanceFromObj = (planeProjection - objectPos).Magnitude();

				float error = BNS_ABS(projectionDistanceFromObj - 1);
				errorVec[i] = error;
				if (error < bestError) {
					bestIndex = i;
					bestError = error;

					Vector3 objectToProjection = planeProjection - objectPos;
					objectToProjection[i] = 0;

					float slope = objectToProjection[(i + 1) % 3] / objectToProjection[(i + 2) % 3];
					float sign = -1;// (objectToProjection[(i + 2) % 3] >= 0) ? 1.0f : -1.0f;

					float angle = atanf(slope) * sign;

					if (objectToProjection[(i + 2) % 3] == 0.0f) {
						angle = sign * 3.141592653589f/2;
					}

					ASSERT(angle == angle);

					selectionOffset.rotation = angle;
				}
			}
		}

		StringStackBuffer<256> errorLog("Error: (%f, %f, %f)\n", errorVec.x, errorVec.y, errorVec.z);
		
		if (bestIndex >= 0) {
			selectedAxis = bestIndex;
		}
	}
}

void Editor::HandleGizmoDrag(Entity* selected) {
	Vector3 cameraPos = editorCamTrans.GetGlobalPosition();
	Vector3 ray = ScreenSpaceCoordsToRay(scene.input.cursorX, scene.input.cursorY);

	Transform* selectedTrans = scene.transforms.GetById(selected->transform);
	Vector3 objectPos = selectedTrans->GetGlobalPosition();

	Vector3 cameraToObj = objectPos - cameraPos;
	Vector3 projectionDiff = VectorProject(cameraToObj, ray) - cameraToObj;

	if (gizmoType == EG_Position) {
		if (selectedAxis >= 0) {
			for (int i = 0; i < 3; i++) {
				if (selectedAxis != i) {
					projectionDiff[i] = 0;
				}
			}

			selectedTrans->position = selectedTrans->position - selectionOffset.position + projectionDiff;
		}
	}
	else if (gizmoType == EG_Rotation) {
		if (selectedAxis >= 0) {
			Vector3 normal;
			normal[selectedAxis] = 1.0f;
			normal = selectedTrans->GetLocalToGlobalMatrix().MultiplyAsDirection(normal);

			float rayNormalOverlap = BNS_ABS(DotProduct(ray, normal));

			float planeProjectDistance = rayNormalOverlap / cameraToObj.MagnitudeSquared();
			Vector3 planeProjection = cameraPos + ray * planeProjectDistance;

			Vector3 objectToProjection = planeProjection - objectPos;
			objectToProjection[selectedAxis] = 0;

			float slope = objectToProjection[(selectedAxis + 1) % 3] / objectToProjection[(selectedAxis + 2) % 3];
			float sign = -1;// (objectToProjection[(selectedAxis + 2) % 3] >= 0) ? 1.0f : -1.0f;

			float angle = atanf(slope) * sign;

			if (objectToProjection[(selectedAxis + 2) % 3] == 0.0f) {
				angle = sign * 3.141592653589f / 2;
			}

			ASSERT(angle == angle);

			float angleDelta = angle - selectionOffset.rotation;
			selectedTrans->rotation = Quaternion(normal, angleDelta) * selectedTrans->rotation;

			ASSERT(selectedTrans->rotation == selectedTrans->rotation);

			selectionOffset.rotation = angle;
		}
	}
}

void Editor::RenderPrefab() {
	glViewport((int)cam.xOffset, (int)cam.yOffset, (int)cam.widthPixels, (int)cam.heightPixels);
	glScissor((int)cam.xOffset, (int)cam.yOffset, (int)cam.widthPixels, (int)cam.heightPixels);

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	if (selectedPrefab == -1) {
		return;
	}

	Prefab* pf = scene.res.prefabs.GetByIdNum(selectedPrefab);
	ASSERT(pf != nullptr);

	Mat4x4 camera = scene.cam.GetCameraMatrix();
	Mat4x4 persp  = scene.cam.GetPerspectiveMatrix();

	Material* mat = scene.res.materials.GetById(pf->matId);
	Program* prog = scene.res.programs.GetById(mat->programId);
	glUseProgram(prog->programObj);

	Mesh* mesh = GlobalScene->res.meshes.GetById(pf->meshId);

	Transform* trans = &pf->transform;

	mat->SetMatrix4Uniform("_objMatrix", trans->GetLocalToGlobalMatrix());
	mat->SetMatrix4Uniform("_camMatrix", camera);
	mat->SetMatrix4Uniform("_perspMatrix", persp);
	
	// TODO: better lighting engine
	//mat->SetVector3Uniform("_lightAngle", lightVec);

	for (int i = 0; i < mat->texCount; i++) {
		Texture* tex = GlobalScene->res.textures.GetById(mat->texIds[i]);
		tex->Bind(GL_TEXTURE0 + i);
	}

	mat->UpdateUniforms();

	GLint posAttribLoc = glGetAttribLocation(prog->programObj, "pos");
	glEnableVertexAttribArray(posAttribLoc);
	glBindBuffer(GL_ARRAY_BUFFER, mesh->posVbo);
	glVertexAttribPointer(posAttribLoc, 3, GL_FLOAT, GL_FALSE, 0, 0);

	GLint uvAttribLoc = glGetAttribLocation(prog->programObj, "uv");
	if (uvAttribLoc >= 0) {
		glEnableVertexAttribArray(uvAttribLoc);
		glBindBuffer(GL_ARRAY_BUFFER, mesh->uvVbo);
		glVertexAttribPointer(uvAttribLoc, 2, GL_FLOAT, GL_FALSE, 0, 0);
	}

	GLint normAttribLoc = glGetAttribLocation(prog->programObj, "normal");
	if (normAttribLoc >= 0) {
		glEnableVertexAttribArray(normAttribLoc);
		glBindBuffer(GL_ARRAY_BUFFER, mesh->normalVbo);
		glVertexAttribPointer(normAttribLoc, 3, GL_FLOAT, GL_FALSE, 0, 0);
	}

	glDrawArrays(GL_TRIANGLES, 0, mesh->faces.count * 3);

	glDisableVertexAttribArray(posAttribLoc);

	if (uvAttribLoc >= 0) {
		glDisableVertexAttribArray(uvAttribLoc);
	}

	if (normAttribLoc >= 0) {
		glDisableVertexAttribArray(normAttribLoc);
	}


}

void Editor::Render() {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	Transform* sceneCamTrans = scene.transforms.GetById(scene.cam.transform);

	Transform oldSceneCamTrans = *sceneCamTrans;

	*sceneCamTrans = editorCamTrans;
	sceneCamTrans->id = oldSceneCamTrans.id;

	scene.cam.xOffset = leftBarWidth;
	scene.cam.widthPixels = cam.widthPixels - leftBarWidth - rightBarWidth;
	scene.cam.heightPixels = cam.heightPixels - topBarHeight;

	if (currentView == EV_Scene) {
		scene.Render();
	}
	else if (currentView == EV_Prefab) {
		// Uhhhh...
		RenderPrefab();
	}

	Mat4x4 camera = scene.cam.GetCameraMatrix();
	Mat4x4 persp = scene.cam.GetPerspectiveMatrix();

	{
		int colMatId = -1;
		scene.res.assetIdMap.LookUp("color.mat", &colMatId);
		Material* mat = scene.res.materials.GetByIdNum(colMatId);
		ASSERT(mat != nullptr);

		mat->SetMatrix4Uniform("_camMatrix", camera);
		mat->SetMatrix4Uniform("_perspMatrix", persp);

		Program* prog = scene.res.programs.GetById(mat->programId);
		glUseProgram(prog->programObj);

		for (int i = 0; i < scene.res.drawCalls.currentCount; i++) {
			Entity* ent = scene.entities.GetById(scene.res.drawCalls.vals[i].entId);
			Transform* trans = scene.transforms.GetById(ent->transform);
			mat->SetMatrix4Uniform("_objMatrix", trans->GetLocalToGlobalMatrix());

			if ((int)ent->id == selectedEntity) {
				DrawCurrentGizmo(ent, mat);
			}
		}
	}

	{
		int debugColMatId = -1;
		scene.res.assetIdMap.LookUp("debugCol.mat", &debugColMatId);
		Material* mat = scene.res.materials.GetByIdNum(debugColMatId);
		ASSERT(mat != nullptr);

		mat->SetMatrix4Uniform("_camMatrix", camera);
		mat->SetMatrix4Uniform("_perspMatrix", persp);
	}

	scene.CustomComponentEditorGuiForEntity(IDHandle<Entity>(selectedEntity));

	scene.cam.xOffset = 0;
	scene.cam.yOffset = 0;
	scene.cam.widthPixels = cam.widthPixels;
	scene.cam.heightPixels = cam.heightPixels;

	glViewport(0, 0, cam.widthPixels, cam.heightPixels);
	glScissor(0, 0, cam.widthPixels, cam.heightPixels);

	sceneCamTrans = scene.transforms.GetById(scene.cam.transform);
	*sceneCamTrans = oldSceneCamTrans;

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	TopPanelGui();

	SidePanelGui();

	static String fff = "The only wat out";
	fff = gui.TextInput(fff, 0, 12, 300, 180, 120);

	static float myVal = 0.2f;
	myVal = gui.SimpleSlider(myVal, 50, 100, 150, 30);

	gui.Render();

	glDisable(GL_BLEND);
}

void Editor::TopPanelGui() {
	float x = leftBarWidth + 20;
	float y = cam.heightPixels - topBarHeight / 2;

	if (gui.TextButton("Change View", 0, 12, x, y, 90, 30)) {
		pickerType = APT_EditorView;
	}

	if (pickerType == APT_EditorView) {
		int pickedIndex = gui.StringPicker(viewNames, EV_Count, 0, 12, x, y - 100, 80, EV_Count * 14 + 10);

		if (pickedIndex > -1) {
			SwitchView((EditorView)pickedIndex);

			pickerType = APT_None;
		}
	}

	x += 100;

	if (currentView == EV_Scene) {

		if (gui.TextButton("Open Level", 0, 12, x, y, 90, 30)) {
			pickerType = APT_Level;
		}

		if (pickerType == APT_Level) {

			Vector<String> levelNames;
			scene.res.FindFileNamesByExtension("lvl", &levelNames);
			int pickedIndex = gui.StringPicker((const char**)levelNames.data, levelNames.count, 0, 12, x, y - 100, 80, levelNames.count * 16);

			if (pickedIndex >= 0) {
				scene.LoadLevel(levelNames.data[pickedIndex].string);
				pickerType = APT_None;
			}
		}

		x += 100;


		if (gui.TextButton("Add Entity", 0, 12, x, y, 90, 30)) {
			int floorMesh, floorMat;
			scene.res.assetIdMap.LookUp("floor.obj", &floorMesh);
			scene.res.assetIdMap.LookUp("floor.mat", &floorMat);
			scene.AddVisibleEntity(IDHandle<Material>(floorMat), IDHandle<Mesh>(floorMesh));
		}

		x += 100;

		if (selectedEntity != -1) {
			if (gui.TextButton("Remove Entity", 0, 12, x, y, 90, 30)) {
				DestroyEntityImmediate(selectedEntity);
				selectedEntity = -1;
			}

			x += 100;
		}

		if (gui.TextButton("Save Scene", 0, 12, x, y, 90, 30)) {
			SaveScene();
		}

		x += 100;

		if (gui.TextButton("Place Prefab", 0, 12, x, y, 90, 30)) {
			pickerType = APT_Prefab;
		}

		if (pickerType == APT_Prefab) {
			Vector<String> prefabFileNames;
			scene.res.FindFileNamesByExtension("bnp", &prefabFileNames);

			int pickedIndex = gui.StringPicker((const char**)prefabFileNames.data, prefabFileNames.count,
				0, 12, x, y - 20, 150, prefabFileNames.count * 16);

			if (pickedIndex > -1) {
				const String& chosenPrefabFileName = prefabFileNames.Get(pickedIndex);

				int chosenPrefabId = -1;
				scene.res.assetIdMap.LookUp(chosenPrefabFileName, &chosenPrefabId);
				ASSERT(chosenPrefabId >= 0);

				Prefab* chosenPrefab = scene.res.prefabs.GetByIdNum(chosenPrefabId);
				ASSERT(chosenPrefab != nullptr);

				Vector3 instantiatePos =
					editorCamTrans.GetLocalToGlobalMatrix().MultiplyAsPosition(Vector3(0, 0, 3));

				Entity* instance = chosenPrefab->Instantiate(instantiatePos);
				selectedEntity = instance->id;

				pickerType = APT_None;
			}
		}
	}
	else if (currentView == EV_Prefab) {
		if (gui.TextButton("Open Prefab", 0, 12, x, y, 90, 30)) {
			pickerType = APT_Prefab;
		}

		if (pickerType == APT_Prefab) {
			Vector<String> prefabFileNames;
			scene.res.FindFileNamesByExtension("bnp", &prefabFileNames);

			int pickedIndex = gui.StringPicker((const char**)prefabFileNames.data, prefabFileNames.count,
				0, 12, x, y - 20, 150, prefabFileNames.count * 16);

			if (pickedIndex > -1) {
				bool found = scene.res.assetIdMap.LookUp(prefabFileNames.Get(pickedIndex), &selectedPrefab);
				ASSERT(found);

				pickerType = APT_None;
			}
		}

		x += 100;

		if (gui.TextButton("Save Prefab", 0, 12, x, y, 90, 30)) {
			SavePrefab();
		}
	}
}

void Editor::SaveScene() {
	if (currentView == EV_Scene) {
		Level* currLevel = scene.res.levels.GetByIdNum(scene.currentLevel);
		scene.SaveLevel(currLevel);
		String levelFileName = scene.res.FindFileNameByIdAndExtension("lvl", currLevel->id);

		File assets;
		assets.Load("assets");
		Vector<File*> levelFiles;
		assets.FindFilesWithExt("lvl", &levelFiles);

		for (int i = 0; i < levelFiles.count; i++) {
			if (levelFileName == levelFiles.Get(i)->fileName) {
				scene.res.SaveLevelToFile(currLevel, levelFiles.Get(i)->fullName);
				break;
			}
		}
	}
	else {
		// TODO: Assert?
	}
}

void Editor::SavePrefab() {
	if (currentView == EV_Prefab) {
		Prefab* prefab = scene.res.prefabs.GetByIdNum(selectedPrefab);
		String prefabFileName = scene.res.FindFileNameByIdAndExtension("bnp", prefab->id);

		File assets;
		assets.Load("assets");
		Vector<File*> prefabFiles;
		assets.FindFilesWithExt("bnp", &prefabFiles);

		for (int i = 0; i < prefabFiles.count; i++) {
			if (prefabFileName == prefabFiles.Get(i)->fileName) {
				scene.res.SavePrefabToFile(prefab, prefabFiles.Get(i)->fullName);
				break;
			}
		}
	}
	else {
		// TODO: Assert?
	}
}

void Editor::SidePanelGui() {
	float y = cam.heightPixels - 20;
	float x = cam.widthPixels - rightBarWidth + 5;

	if (currentView == EV_Scene) {
		if (selectedEntity != -1) {
			Entity* ent = scene.entities.GetByIdNum(selectedEntity);
			gui.DrawTextLabel(StringStackBuffer<64>("Id: %d", ent->id).buffer, IDHandle<BitmapFont>(0), 12, x, y);
			y -= 14;

			Transform* entTrans = scene.transforms.GetById(ent->transform);

			gui.DrawTextLabel("Position: ", IDHandle<BitmapFont>(0), 12, x, y);
			y -= 14;
			entTrans->position = Vec3Field(entTrans->position, x, y, rightBarWidth - 5);
			y -= 14;

			gui.DrawTextLabel("Scale: ", IDHandle<BitmapFont>(0), 12, x, y);
			y -= 14;
			entTrans->scale = Vec3Field(entTrans->scale, x, y, rightBarWidth - 5);
			y -= 14;

			String meshName;
			String matName;
			DrawCall* dc = scene.GetDrawCallForEntity(IDHandle<Entity>(ent->id));
			if (dc != nullptr) {
				IDHandle<Mesh> meshId = dc->meshId;
				IDHandle<Material> matId = dc->matId;

				meshName = scene.res.FindFileNameByIdAndExtension("obj", meshId.id);
				if (meshName == "") {
					meshName = scene.res.FindFileNameByIdAndExtension("dae", meshId.id);
				}
				matName = scene.res.FindFileNameByIdAndExtension("mat", matId.id);
			}

			if (meshName.string != nullptr) {
				float currX = x;
				currX += gui.DrawTextLabel("Mesh: ", IDHandle<BitmapFont>(0), 12, currX, y);
				String newMeshName = gui.TextInput(meshName, 0, 12, currX, y, cam.widthPixels - currX - 5);
				y -= 14;

				if (newMeshName.string != meshName.string) {
					int newMeshId = -1;
					bool isValidMesh = scene.res.assetIdMap.LookUp(newMeshName, &newMeshId);

					if (isValidMesh) {
						for (int i = 0; i < scene.res.drawCalls.currentCount; i++) {
							if (scene.res.drawCalls.vals[i].entId.id == ent->id) {
								scene.res.drawCalls.vals[i].meshId = IDHandle<Mesh>(newMeshId);
							}
						}
					}
				}
			}

			if (matName.string != nullptr) {
				float currX = x;
				currX += gui.DrawTextLabel("Material: ", IDHandle<BitmapFont>(0), 12, currX, y);
				String newMatName = gui.TextInput(matName, 0, 12, currX, y, cam.widthPixels - currX - 5);
				y -= 14;

				if (newMatName.string != matName.string) {
					int newMatId = -1;
					bool isValidMesh = scene.res.assetIdMap.LookUp(newMatName, &newMatId);

					if (isValidMesh) {
						for (int i = 0; i < scene.res.drawCalls.currentCount; i++) {
							if (scene.res.drawCalls.vals[i].entId.id == ent->id) {
								scene.res.drawCalls.vals[i].matId = IDHandle<Material>(newMatId);
							}
						}
					}
				}
			}

			for (int i = 0; i < CCT_Count; i++) {
				Component* compArray = getComponentArrayFuncs[i]();
				int compCount = getComponentCountFuncs[i]();

				Component* compCursor = compArray;
				MetaStruct* ms = componentMetaData[i];
				for (int j = 0; j < compCount; j++) {

					if (compCursor->entity.id == selectedEntity) {
						bool toRemove = false;
						y = EditComponentGui(compCursor, ms, x, y, &toRemove);

						if (toRemove) {
							removeComponentFuncs[i](compCursor->id);
							j--;
							compCursor = (Component*)(((char*)compCursor) - ms->size);
						}
					}

					compCursor = (Component*)(((char*)compCursor) + ms->size);
				}
			}

			// Just to give us some breathing room
			y -= 10;

			if (gui.TextButton("Add Component", 0, 12, x, y, (rightBarWidth - 10), 26)) {
				pickerType = APT_ComponentType;
			}

			y -= 30;

			if (pickerType == APT_ComponentType) {
				const char* compNames[CCT_Count] = {};
				for (int i = 0; i < CCT_Count; i++) {
					compNames[i] = componentMetaData[i]->name;
				}

				int pickedIndex = gui.StringPicker(compNames, CCT_Count, 0, 12, x, y, leftBarWidth - 4, cam.heightPixels - y - 2);

				if (pickedIndex > -1) {
					//Add component.
					Component* comp = addComponentFuncs[pickedIndex]();
					comp->entity.id = selectedEntity;

					pickerType = APT_None;
				}
			}
		}
	}
	else if (currentView == EV_Prefab){
		if (selectedPrefab != -1) {
			Prefab* prefab = GlobalScene->res.prefabs.GetByIdNum(selectedPrefab);
			ASSERT(prefab != nullptr);

			IDHandle<Mesh> meshId = prefab->meshId;
			IDHandle<Material> matId = prefab->matId;

			String meshName = scene.res.FindFileNameByIdAndExtension("obj", meshId.id);
			String matName = scene.res.FindFileNameByIdAndExtension("mat", matId.id);

			if (meshName.string != nullptr) {
				float currX = x;
				currX += gui.DrawTextLabel("Mesh: ", IDHandle<BitmapFont>(0), 12, currX, y);
				String newMeshName = gui.TextInput(meshName, 0, 12, currX, y, cam.widthPixels - currX - 5);
				y -= 14;

				if (newMeshName.string != meshName.string) {
					int newMeshId = -1;
					bool isValidMesh = scene.res.assetIdMap.LookUp(newMeshName, &newMeshId);

					if (isValidMesh) {
						prefab->meshId.id = newMeshId;
					}
				}
			}

			if (matName.string != nullptr) {
				float currX = x;
				currX += gui.DrawTextLabel("Material: ", IDHandle<BitmapFont>(0), 12, currX, y);
				String newMatName = gui.TextInput(matName, 0, 12, currX, y, cam.widthPixels - currX - 5);
				y -= 14;

				if (newMatName.string != matName.string) {
					int newMatId = -1;
					bool isValidMesh = scene.res.assetIdMap.LookUp(newMatName, &newMatId);

					if (isValidMesh) {
						prefab->matId.id = newMatId;
					}
				}
			}

			while (prefab->customComponents.GetLength() > 0) {
				int id = prefab->customComponents.Read<int>();
				ASSERT(id >= 0 && id < CCT_Count);

				Component* comp = componentSerializeBuffer[id];

				(componentMemDeserializeFuncs[id])(comp, &prefab->customComponents);

				MetaStruct* ms = componentMetaData[id];

				bool toRemove = false;
				y = EditComponentGui(comp, ms, x, y, &toRemove);

				if (toRemove) {
					// TODO: Remove component from stream
					//removeComponentFuncs[i](compCursor->id);
					//j--;
					//compCursor = (Component*)(((char*)compCursor) - ms->size);
				}
			}

			prefab->customComponents.readHead = prefab->customComponents.base;
		}
	}
}

float Editor::EditComponentGui(Component* comp, MetaStruct* meta, float x, float y, bool* outRemove) {
	float currY = y;
	gui.DrawTextLabel(meta->name, IDHandle<BitmapFont>(0), 12, x, currY);

	BitmapFont* font = scene.res.fonts.GetByIdNum(0);
	float removeWidth = font->GetCursorPos("Remove", StrLen("Remove"));

	if (gui.TextButton("Remove", 0, 12, cam.widthPixels - removeWidth - 6, currY + 6, removeWidth + 4, 12)) {
		*outRemove = true;
	}

	currY -= 14;

	for (int i = 0; i < meta->fieldCount; i++) {
		const MetaField* mf = &meta->fields[i];

		if ((((int)mf->flags) & FSF_DoNotSerialize) == 0) {
			char* fieldPtr = ((char*)comp) + mf->offset;

			if (mf->type >= MT_FundamentalBegin && mf->type < MT_FundamentalEnd) {
				gui.DrawTextLabel(mf->name, IDHandle<BitmapFont>(0), 12, x, currY);
				currY -= 14;
			}

			float width = cam.widthPixels - x - 5;
			if (((int)mf->flags) & FSF_SerializeAsEnum) {
				int* fieldVal = (int*)fieldPtr;
				const char* enumStr = EncodeEnum(mf->type, *fieldVal);

				String newStr = gui.TextInput(enumStr, 0, 12, x, currY, width);

				int newVal = ParseEnum(mf->type, newStr.string);

				if (newVal != -1) {
					*fieldPtr = newVal;
				}

				/*
				int entryCount = enumMetaData[mf->type]->entryCount;
				char** names = malloc(sizeof(char*) * entryCount);
				for (int j = 0; j < entryCount; j++) {
					names[j] = enumMetaData[mf->type]->entries[j].serial;
				}

				int chosenIndex = gui.StringPicker(names, entryCount, 0, 12.0f, x, currX, width, entryCount * 14);

				if (chosenIndex)

				currY += 14;
				currY -= (entryCount * 14);

				free(names);
				*/
			}
			else if (((int)mf->flags) & FSF_SerializeFromId) {
				int* fieldVal = (int*)fieldPtr;

				String fileName = scene.res.FindFileNameByIdAndExtension(mf->serializeExt, *fieldVal);

				fileName = gui.TextInput(fileName, 0, 12, x, currY, width);

				int newVal;
				if (scene.res.assetIdMap.LookUp(fileName, &newVal)) {
					*fieldVal = newVal;
				}
			}
			else if (mf->type == MT_Int) {
				int* fieldVal = (int*)fieldPtr;
				*fieldVal = IntField(*fieldVal, x, currY, width);
			}
			else if (mf->type == MT_Float) {
				float* fieldVal = (float*)fieldPtr;
				*fieldVal = FloatField(*fieldVal, x, currY, width);
			}
			else if (mf->type == MT_Bool) {
				bool* fieldVal = (bool*)fieldPtr;
				*fieldVal = BoolField(*fieldVal, x, currY, width);
			}
			else if (mf->type == MT_Vector2) {
				Vector2* fieldVal = (Vector2*)fieldPtr;
				*fieldVal = Vec2Field(*fieldVal, x, currY, width);
			}
			else if (mf->type == MT_Vector3) {
				Vector3* fieldVal = (Vector3*)fieldPtr;
				*fieldVal = Vec3Field(*fieldVal, x, currY, width);
			}
			else {
				// Skip field, don't decrease y
				currY += 14;
			}

			currY -= 14;
		}
	}

	return currY;
}

void EditorShiftButton(Editor* ed, uint32 buttonId) {
	GuiButton* button = ed->gui.buttons.GetByIdNum(buttonId);
	button->content.asciiStr = button->content.asciiStr.Insert('#', 0);
}

void EditorResetButton(Editor* ed, uint32 buttonId) {
	GuiButton* button = ed->gui.buttons.GetByIdNum(buttonId);
	button->content.asciiStr = "PUSH";
}

void EditorAddStringPicker(Editor* ed, int enumIndex, int buttonId, Vector2 pos, Vector2 size) {
	Vector<String> strings;

	MetaEnum* me = enumMetaData[enumIndex];
	for (int i = 0; i < me->entryCount; i++) {
		strings.PushBack(me->entries[i].serial);
	}

	GuiStringPicker* picker = ed->gui.AddStringPicker(pos, size);
	picker->choice = (me->flags & MEF_EnumIsFlag) ? 0 : -1;
	picker->choices = strings;
	picker->options = (me->flags & MEF_EnumIsFlag) ? GSPO_MultipleChoice : GSPO_SingleChoice;
	picker->onSelect.type = AT_EditorPrintEnum;
	picker->onSelect.EditorPrintEnum_data.buttonId = buttonId;
	picker->onSelect.EditorPrintEnum_data.pickerId = picker->id;
	picker->onSelect.EditorPrintEnum_data.ed = ed;
	picker->allIndex = me->allIdx;
	picker->clearIndex = me->noneIdex;
}

void EditorPrintEnum(Editor* ed, int pickerId, int buttonId) {
	GuiStringPicker* picker = ed->gui.stringPickers.GetByIdNum(pickerId);
	ed->gui.buttons.GetByIdNum(buttonId)->content.asciiStr = Itoa(picker->choice);
}

void Editor::StartUp() {
	cam.fov = 80;
	cam.nearClip = 0.001f;
	cam.farClip = 1000.0f;

	leftBarWidth = 250;
	rightBarWidth = 200;
	topBarHeight = 120;

	cameraCursorX = 0;
	cameraCursorY = 0;

	selectedEntity = -1;
	selectedAxis = -1;
	gizmoType = EG_Position;

	selectedPrefab = -1;

	pickerType = APT_None;

	copyPasteData.type = CPT_Entity;
	copyPasteData.entityId = -1;

	scene.StartUp();
	gui.Init();

	GuiCheckbox* cBox = gui.AddCheckbox(Vector2(200, 10), Vector2(30, 30));
	(void)cBox;

	/*
	{
		GuiButton* button = gui.AddButton(Vector2(50, 300), Vector2(80, 30));
		button->content.SetType(GCT_Ascii);
		button->content.asciiStr = "Enum1";
		button->content.bmpFontId = 0;
		button->content.textScale = 12;

		button->onClick.type = AT_EditorAddStringPicker;
		button->onClick.EditorAddStringPicker_data.ed = this;
		button->onClick.EditorAddStringPicker_data.enumIndex = FindEnumByName("ColliderFlags");
		button->onClick.EditorAddStringPicker_data.pos = Vector2(50, 430);
		button->onClick.EditorAddStringPicker_data.size = Vector2(100, 120);
		button->onClick.EditorAddStringPicker_data.buttonId = button->id;
	}
	*/

	{
		GuiButton* button = gui.AddButton(Vector2(50, 250), Vector2(80, 30));
		button->content.SetType(GCT_Ascii);
		button->content.asciiStr = "Enum2";
		button->content.bmpFontId = IDHandle<BitmapFont>(0);
		button->content.textScale = 12;

		button->onClick.type = AT_EditorAddStringPicker;
		button->onClick.EditorAddStringPicker_data.ed = this;
		button->onClick.EditorAddStringPicker_data.enumIndex = FindEnumByName("EnemyState");
		button->onClick.EditorAddStringPicker_data.pos = Vector2(160, 430);
		button->onClick.EditorAddStringPicker_data.size = Vector2(100, 120);
		button->onClick.EditorAddStringPicker_data.buttonId = button->id;

	}

	//GuiButton* button = gui.AddButton(Vector2(5, 5), Vector2(80, 30));
	//button->content.SetType(GCT_Ascii);
	//button->content.asciiStr = "PUSH";
	//button->content.bmpFontId = 0;
	//button->content.textScale = 12;
	//
	//button->onClick.type = AT_EditorShiftButton;
	//button->onClick.EditorShiftButton_data.ed = this;
	//button->onClick.EditorShiftButton_data.buttonId = button->id;
	//
	//GuiButton* button2 = gui.AddButton(Vector2(100, 150), Vector2(80, 30));
	//button2->content.SetType(GCT_Ascii);
	//button2->content.asciiStr = "RESET";
	//button2->content.bmpFontId = 0;
	//button2->content.textScale = 12;
	//
	//button2->onClick.type = AT_EditorResetButton;
	//button2->onClick.EditorShiftButton_data.ed = this;
	//button2->onClick.EditorShiftButton_data.buttonId = button->id;
}

int Editor::GetSelectedEntity(int pixelX, int pixelY) {
	PhysicsSystem sys;

	for (int i = 0; i < scene.entities.currentCount; i++) {
		Entity* ent = &scene.entities.vals[i];

		IDHandle<Mesh> meshId;
		for (int j = 0; j < scene.res.drawCalls.currentCount; j++) {
			DrawCall* dc = &scene.res.drawCalls.vals[j];
			if (dc->entId.id == ent->id) {
				meshId = dc->meshId;
				break;
			}
		}

		if (meshId.id == -1) {
			BoxCollider* boxCol = sys.boxCols.CreateAndAdd();
			boxCol->entity.id = ent->id;
			boxCol->position = Vector3(0, 0, 0);
			boxCol->size = Vector3(0.5f, 0.5f, 0.5f);
		}
		else {
			MeshCollider* meshCol = sys.meshCols.CreateAndAdd();
			meshCol->entity.id = ent->id;
			Mesh* mesh = scene.res.meshes.GetById(meshId);
			meshCol->mesh.id = mesh->id;
		}
	}

	Mat4x4 camMat = editorCamTrans.GetLocalToGlobalMatrix();
	Vector3 origin = camMat.MultiplyAsPosition(Vector3(0, 0, 0));
	Vector3 direction = ScreenSpaceCoordsToRay(pixelX, pixelY);

	RaycastHit hit = sys.Raycast(origin, direction);
	if (hit.wasHit) {
		if (hit.type == CT_MESH) {
			return sys.meshCols.GetByIdNum(hit.colId)->entity.id;
		}
		else if (hit.type == CT_BOX) {
			return sys.boxCols.GetByIdNum(hit.colId)->entity.id;
		}
		else {
			ASSERT(false);
			return -1;
		}
	}
	else {
		return -1;
	}
}

Vector3 Editor::ScreenSpaceCoordsToRay(float pixelX, float pixelY) {
	float screenspaceX = (pixelX - leftBarWidth) / (cam.widthPixels - leftBarWidth - rightBarWidth) * 2 - 1;
	float screenspaceY = (pixelY - topBarHeight) / (cam.heightPixels - topBarHeight) * 2 - 1;
	screenspaceY *= -1;

	// Don't know why, but this makes the editor selection much better.
	screenspaceX *= 0.25f;
	screenspaceY *= 0.5f;

	Mat4x4 camMat = editorCamTrans.GetLocalToGlobalMatrix();

	// FOV(rectilinear) = 2 * arctan(frame size / (focal length * 2))
	// tan(FOV/2) = frame size / (focal length * 2)
	// AR / tan(FOV/2) / 2 = focalLength
	float aspect = cam.widthPixels / cam.heightPixels;
	float halfFovInRads = cam.fov / 360 * 3.141592653589f;
	float tangentHalfFov = tan(halfFovInRads);
	float focalLength = (aspect / 2) / tangentHalfFov;

	Vector3 mouseWorldPos = camMat.MultiplyAsPosition(Vector3(screenspaceX, screenspaceY * aspect, 1.0f * focalLength));
	Vector3 origin = camMat.MultiplyAsPosition(Vector3(0, 0, 0));
	Vector3 direction = (mouseWorldPos - origin).Normalized();

	return direction;
}

void Editor::DrawCurrentGizmo(const Entity* ent, Material* mat) {
	switch (gizmoType) {
	case EG_Select: {
		DrawSelectGizmo(ent, mat);
	}break;

	case EG_Position: {
		DrawPositionGizmo(ent, mat);
	}break;

	case EG_Rotation: {
		DrawRotationGizmo(ent, mat);
	}break;

	case EG_Scale: {
		DrawScaleGizmo(ent, mat);
	}break;
	}
}

void Editor::DrawSelectGizmo(const Entity* ent, Material* mat) {

}

void Editor::DrawPositionGizmo(const Entity* ent, Material* mat) {
	for (int i = 0; i < 3; i++) {
		Vector4 col = Vector4(0, 0, 0, 1);
		((float*)&col)[i] = (i == selectedAxis ? 1 : 0.6f);
		mat->SetVector4Uniform("_col", col);
		mat->UpdateUniforms();

		glBegin(GL_LINE_LOOP);
		glVertex3f(0, 0, 0);
		glVertex3f(col.x, col.y, col.z);
		glEnd();
	}
}

bool Editor::BoolField(bool val, float x, float y, float w) {
	String text = EncodeBool(val);
	text = gui.TextInput(text, 0, 12, x, y, w);

	return (text.string != nullptr) ? ParseBool(text.string) : false;
}

int Editor::IntField(int val, float x, float y, float w) {
	String text = Itoa(val);
	text = gui.TextInput(text, 0, 12, x, y, w);

	return (text.string != nullptr) ? Atoi(text.string) : 0;
}

float Editor::FloatField(float val, float x, float y, float w) {
	String text = Ftoa(val);
	text = gui.TextInput(text, 0, 12, x, y, w);

	return (text.string != nullptr) ? Atof(text.string) : 0.0f;
}

Vector2 Editor::Vec2Field(Vector2 val, float x, float y, float w) {
	BitmapFont* fnt = scene.res.fonts.GetByIdNum(0);
	float usedWidth = fnt->GetCursorPos("X: ", 2) + fnt->GetCursorPos("Y: ", 2);

	ASSERT(usedWidth < w);

	float inputWidth = (w - usedWidth) / 2;

	Vector2 retVal = val;

	float currX = x;
	currX += gui.DrawTextLabel("X: ", IDHandle<BitmapFont>(0), 12, currX, y);
	retVal.x = FloatField(val.x, currX, y, inputWidth);
	currX += inputWidth;

	currX += gui.DrawTextLabel("Y: ", IDHandle<BitmapFont>(0), 12, currX, y);
	retVal.y = FloatField(val.y, currX, y, inputWidth);
	currX += inputWidth;

	return retVal;
}

Vector3 Editor::Vec3Field(Vector3 val, float x, float y, float w) {
	BitmapFont* fnt = scene.res.fonts.GetByIdNum(0);
	float usedWidth = fnt->GetCursorPos("X: ", 3) + fnt->GetCursorPos("Y: ", 3) + fnt->GetCursorPos("Z: ", 3);

	ASSERT(usedWidth < w);

	float inputWidth = (w - usedWidth - 5) / 3;

	Vector3 retVal = val;

	float currX = x;

	const char* labels[3] = {
		"X: ",
		"Y: ",
		"Z: "
	};

	for (int i = 0; i < 3; i++) {
		currX += gui.DrawTextLabel(labels[i], IDHandle<BitmapFont>(0), 12, currX, y);
		retVal[i] = FloatField(val[i], currX, y, inputWidth);
		currX += inputWidth;
	}

	return retVal;
}

void Editor::DrawRotationGizmo(const Entity* ent, Material* mat) {
	static const int numVerts = 48;
	float cosTable[numVerts] = { 0 };

	for (int i = 0; i < numVerts; i++) {
		float val = ((float)i) / numVerts * 2 * 3.14159265359f;
		cosTable[i] = cosf(val);
	}

	for (int i = 0; i < 3; i++) {
		Vector4 col = Vector4(0, 0, 0, 1);
		((float*)&col)[i] = (i == selectedAxis ? 1 : 0.6f);
		mat->SetVector4Uniform("_col", col);
		mat->UpdateUniforms();

		glBegin(GL_LINE_LOOP);
		for (int j = 0; j < numVerts; j++) {
			Vector3 pos;
			pos[i] = 0;
			pos[(i + 1) % 3] = cosTable[j];
			pos[(i + 2) % 3] = sqrtf(1 - cosTable[j] * cosTable[j]) * (j < numVerts/2 ? -1 : 1);
			glVertex3f(pos.x/2, pos.y/2, pos.z/2);
		}
		glEnd();
	}
}

void Editor::DrawScaleGizmo(const Entity* ent, Material* mat) {

}

