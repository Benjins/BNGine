#include "Editor.h"

#include "../gfx/GLExtInit.h"
#include "../util/Serialization.h"

#include "../metagen/ComponentMeta.h"
#include "../metagen/MetaStruct.h"

#include "../../ext/3dbasics/Vector4.h"
#include "../../ext/CppUtils/filesys.h"

#include <float.h>

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
					Entity* srcEnt = scene.entities.GetById(copyPasteData.entityId);
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
				Entity* selected = scene.entities.GetById(selectedEntity);

				HandleGizmoDrag(selected);
			}
		}
	}
}

void Editor::HandleGizmoClick() {
	Vector3 cameraPos = editorCamTrans.GetGlobalPosition();
	Vector3 ray = ScreenSpaceCoordsToRay(scene.input.cursorX, scene.input.cursorY);

	Entity* selected = scene.entities.GetById(selectedEntity);
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

void Editor::Render() {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	Transform* sceneCamTrans = scene.transforms.GetById(scene.cam.transform);

	Transform oldSceneCamTrans = *sceneCamTrans;

	*sceneCamTrans = editorCamTrans;
	sceneCamTrans->id = oldSceneCamTrans.id;

	scene.cam.xOffset = leftBarWidth;
	scene.cam.widthPixels = cam.widthPixels - leftBarWidth - rightBarWidth;
	scene.cam.heightPixels = cam.heightPixels - topBarHeight;

	scene.Render();

	Mat4x4 camera = scene.cam.GetCameraMatrix();
	Mat4x4 persp = scene.cam.GetPerspectiveMatrix();

	{
		int colMatId = -1;
		scene.res.assetIdMap.LookUp("color.mat", &colMatId);
		Material* mat = scene.res.materials.GetById(colMatId);
		ASSERT(mat != nullptr);

		mat->SeMatrix4Uniform("_camMatrix", camera);
		mat->SeMatrix4Uniform("_perspMatrix", persp);

		Program* prog = scene.res.programs.GetById(mat->programId);
		glUseProgram(prog->programObj);

		for (int i = 0; i < scene.res.drawCalls.currentCount; i++) {
			Entity* ent = scene.entities.GetById(scene.res.drawCalls.vals[i].entId);
			Transform* trans = scene.transforms.GetById(ent->transform);
			mat->SeMatrix4Uniform("_objMatrix", trans->GetLocalToGlobalMatrix());

			if ((int)ent->id == selectedEntity) {
				DrawCurrentGizmo(ent, mat);
			}
		}
	}

	{
		int debugColMatId = -1;
		scene.res.assetIdMap.LookUp("debugCol.mat", &debugColMatId);
		Material* mat = scene.res.materials.GetById(debugColMatId);
		ASSERT(mat != nullptr);

		mat->SeMatrix4Uniform("_camMatrix", camera);
		mat->SeMatrix4Uniform("_perspMatrix", persp);
	}

	if (selectedEntity != -1) {
		scene.CustomComponentEditorGuiForEntity(selectedEntity);
	}

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

	gui.Render();

	glDisable(GL_BLEND);
}

void Editor::TopPanelGui() {
	float x = leftBarWidth + 20;
	float y = cam.heightPixels - topBarHeight / 2;
	if (gui.TextButton("Add Entity", 0, 12, x, y, 90, 30)) {
		int floorMesh, floorMat;
		scene.res.assetIdMap.LookUp("floor.obj", &floorMesh);
		scene.res.assetIdMap.LookUp("floor.mat", &floorMat);
		scene.AddVisibleEntity(floorMat, floorMesh);
	}

	x += 100;

	if (selectedEntity != -1) {
		if (gui.TextButton("Remove Entity", 0, 12, x, y, 90, 30)) {
			scene.DestroyEntityImmediate(selectedEntity);
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

			Prefab* chosenPrefab = scene.res.prefabs.GetById(chosenPrefabId);
			ASSERT(chosenPrefab != nullptr);

			Vector3 instantiatePos
				= editorCamTrans.GetLocalToGlobalMatrix().MultiplyAsPosition(Vector3(0, 0, 3));

			Entity* instance = chosenPrefab->Instantiate(instantiatePos);
			selectedEntity = instance->id;

			pickerType = APT_None;
		}
	}
}

void Editor::SaveScene() {
	Level* currLevel = scene.res.levels.GetById(scene.currentLevel);
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

void Editor::SidePanelGui() {
	if (selectedEntity != -1) {
		Entity* ent = scene.entities.GetById(selectedEntity);

		float y = cam.heightPixels - 20;
		float x = cam.widthPixels - rightBarWidth + 5;
		gui.DrawTextLabel(StringStackBuffer<64>("Id: %d", ent->id).buffer, 0, 12, x, y);
		y -= 14;

		Transform* entTrans = scene.transforms.GetById(ent->transform);

		gui.DrawTextLabel("Position: ", 0, 12, x, y);
		y -= 14;
		entTrans->position = Vec3Field(entTrans->position, x, y, rightBarWidth - 5);
		y -= 14;

		gui.DrawTextLabel("Scale: ", 0, 12, x, y);
		y -= 14;
		entTrans->scale = Vec3Field(entTrans->scale, x, y, rightBarWidth - 5);
		y -= 14;

		String meshName;
		String matName;
		DrawCall* dc = scene.GetDrawCallForEntity(ent->id);
		if (dc != nullptr) {
			uint32 meshId = dc->meshId;
			uint32 matId = dc->matId;

			meshName = scene.res.FindFileNameByIdAndExtension("obj", meshId);
			matName = scene.res.FindFileNameByIdAndExtension("mat", matId);
		}

		if (meshName.string != nullptr) {
			float currX = x;
			currX += gui.DrawTextLabel("Mesh: ", 0, 12, currX, y);
			String newMeshName = gui.TextInput(meshName, 0, 12, currX, y, cam.widthPixels - currX - 5);
			y -= 14;

			if (newMeshName.string != meshName.string) {
				int newMeshId = -1;
				bool isValidMesh = scene.res.assetIdMap.LookUp(newMeshName, &newMeshId);

				if (isValidMesh) {
					for (int i = 0; i < scene.res.drawCalls.currentCount; i++) {
						if (scene.res.drawCalls.vals[i].entId == ent->id) {
							scene.res.drawCalls.vals[i].meshId = newMeshId;
						}
					}
				}
			}
		}

		if (matName.string != nullptr) {
			float currX = x;
			currX += gui.DrawTextLabel("Material: ", 0, 12, currX, y);
			String newMatName = gui.TextInput(matName, 0, 12, currX, y, cam.widthPixels - currX - 5);
			y -= 14;

			if (newMatName.string != matName.string) {
				int newMatId = -1;
				bool isValidMesh = scene.res.assetIdMap.LookUp(newMatName, &newMatId);

				if (isValidMesh) {
					for (int i = 0; i < scene.res.drawCalls.currentCount; i++) {
						if (scene.res.drawCalls.vals[i].entId == ent->id) {
							scene.res.drawCalls.vals[i].matId = newMatId;
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

				if ((int)compCursor->entity == selectedEntity) {
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

		switch (pickerType) {
		case APT_ComponentType: {
			const char* compNames[CCT_Count] = {};
			for (int i = 0; i < CCT_Count; i++) {
				compNames[i] = componentMetaData[i]->name;
			}

			int pickedIndex = gui.StringPicker(compNames, CCT_Count, 0, 12, x, y, leftBarWidth - 4, cam.heightPixels - y - 2);

			if (pickedIndex > -1) {
				//Add component.
				Component* comp = addComponentFuncs[pickedIndex]();
				comp->entity = selectedEntity;

				pickerType = APT_None;
			}
		} break;

		case APT_Mesh: {
			//Vector<String> fileNames;
			//scene.res.FindFileNamesByExtension("obj", &fileNames);
		} break;

		case APT_Material: {

		} break;

		case APT_None: {

		} break;
		}
	}
}

float Editor::EditComponentGui(Component* comp, MetaStruct* meta, float x, float y, bool* outRemove) {
	float currY = y;
	gui.DrawTextLabel(meta->name, 0, 12, x, currY);

	BitmapFont* font = scene.res.fonts.GetById(0);
	float removeWidth = font->GetCursorPos("Remove", StrLen("Remove"));

	if (gui.TextButton("Remove", 0, 12, cam.widthPixels - removeWidth - 6, currY + 6, removeWidth + 4, 12)) {
		*outRemove = true;
	}

	currY -= 14;

	for (int i = 0; i < meta->fieldCount; i++) {
		const MetaField* mf = &meta->fields[i];

		char* fieldPtr = ((char*)comp) + mf->offset;

		if (mf->type >= MT_FundamentalBegin && mf->type < MT_FundamentalEnd) {
			gui.DrawTextLabel(mf->name, 0, 12, x, currY);
			currY -= 14;
		}

		float width = cam.widthPixels - x - 5;

		if (mf->type == MT_Int) {
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

	return currY;
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

	pickerType = APT_None;

	copyPasteData.type = CPT_Entity;
	copyPasteData.entityId = -1;

	scene.StartUp();
	gui.Init();
}

int Editor::GetSelectedEntity(int pixelX, int pixelY) {
	PhysicsSystem sys;

	for (int i = 0; i < scene.entities.currentCount; i++) {
		Entity* ent = &scene.entities.vals[i];

		int meshId = -1;
		for (int j = 0; j < scene.res.drawCalls.currentCount; j++) {
			DrawCall* dc = &scene.res.drawCalls.vals[j];
			if (dc->entId == ent->id) {
				meshId = dc->meshId;
				break;
			}
		}

		BoxCollider* boxCol = sys.boxCols.CreateAndAdd();
		boxCol->entity = ent->id;

		if (meshId == -1) {
			boxCol->position = Vector3(0, 0, 0);
			boxCol->size = Vector3(1, 1, 1);
		}
		else {
			Mesh* mesh = scene.res.meshes.GetById(meshId);
			Vector3 minPos = Vector3(FLT_MAX, FLT_MAX, FLT_MAX);
			Vector3 maxPos = Vector3(-FLT_MAX, -FLT_MAX, -FLT_MAX);

			for (int j = 0; j < mesh->positions.count; j++) {
				for (int k = 0; k < 3; k++) {
					minPos[k] = BNS_MIN(minPos[k], mesh->positions.data[j][k]);
					maxPos[k] = BNS_MAX(maxPos[k], mesh->positions.data[j][k]);
				}
			}

			boxCol->position = (minPos + maxPos) / 2;
			//Ensure that we're just a smidge bigger
			boxCol->size = (maxPos - minPos) / 2 + Vector3(0.02f, 0.02f, 0.02f);
		}
	}

	Mat4x4 camMat = editorCamTrans.GetLocalToGlobalMatrix();
	Vector3 origin = camMat.MultiplyAsPosition(Vector3(0, 0, 0));
	Vector3 direction = ScreenSpaceCoordsToRay(pixelX, pixelY);

	RaycastHit hit = sys.Raycast(origin, direction);
	if (hit.wasHit) {
		//Transform* trans = scene.transforms.GetById(scene.entities.vals[0].transform);
		//trans->position = hit.globalPos;
		return sys.boxCols.GetById(hit.colId)->entity;
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
	screenspaceX *= 0.5f;
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
	BitmapFont* fnt = scene.res.fonts.GetById(0);
	float usedWidth = fnt->GetCursorPos("X: ", 2) + fnt->GetCursorPos("Y: ", 2);

	ASSERT(usedWidth < w);

	float inputWidth = (w - usedWidth) / 2;

	Vector2 retVal = val;

	float currX = x;
	currX += gui.DrawTextLabel("X: ", 0, 12, currX, y);
	retVal.x = FloatField(val.x, currX, y, inputWidth);
	currX += inputWidth;

	currX += gui.DrawTextLabel("Y: ", 0, 12, currX, y);
	retVal.y = FloatField(val.y, currX, y, inputWidth);
	currX += inputWidth;

	return retVal;
}

Vector3 Editor::Vec3Field(Vector3 val, float x, float y, float w) {
	BitmapFont* fnt = scene.res.fonts.GetById(0);
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
		currX += gui.DrawTextLabel(labels[i], 0, 12, currX, y);
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

