#include "Editor.h"

#include "../gfx/GLExtInit.h"

#include "../../ext/3dbasics/Vector4.h"

void Editor::Update() {
	//scene.Update();

	if (gui.textInputState.activeIndex == -1) {
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

		editorCamTrans.position = editorCamTrans.position + moveVec / 50;
	}

	if (scene.input.cursorX > leftBarWidth && scene.input.cursorX < cam.widthPixels - rightBarWidth 
		&& scene.input.cursorY > topBarHeight
		&& scene.input.MouseButtonIsDown(MouseButton::SECONDARY)) {
		cameraCursorX -= scene.input.cursorDeltaX;
		cameraCursorY -= scene.input.cursorDeltaY;

		OutputDebugStringA(StringStackBuffer<256>("cameraCursorX: %f, cameraCursorY: %f\n", cameraCursorX, cameraCursorY).buffer);

		editorCamTrans.rotation = Quaternion(Y_AXIS, cameraCursorX / 80) * Quaternion(X_AXIS, cameraCursorY / 80);
	}

	if (scene.input.KeyIsReleased('R')) {
		gizmoType = EG_Rotation;
	}
	if (scene.input.KeyIsReleased('P')) {
		gizmoType = EG_Position;
	}

	if (scene.input.MouseButtonIsReleased(MouseButton::PRIMARY)) {
		if (scene.input.cursorX > leftBarWidth && scene.input.cursorX < cam.widthPixels - rightBarWidth
			&& scene.input.cursorY > topBarHeight) {

			selectedEntity = GetSelectedEntity((int)scene.input.cursorX, (int)scene.input.cursorY);
			selectedAxis = -1;
			OutputDebugStringA(StringStackBuffer<256>("The following entity was selected: %d\n", selectedEntity).buffer);
		}
	}

	if (scene.input.MouseButtonIsPressed(MouseButton::PRIMARY)) {
		if (scene.input.cursorX > leftBarWidth && scene.input.cursorX < cam.widthPixels - rightBarWidth
			&& scene.input.cursorY > topBarHeight && selectedEntity != -1) {

			if (selectedEntity == GetSelectedEntity((int)scene.input.cursorX, (int)scene.input.cursorY)) {
				HandleGizmoClick();
			}
		}
	}
	else if (scene.input.MouseButtonIsDown(MouseButton::PRIMARY)) {
		if (selectedEntity != -1) {
			Entity* selected = scene.entities.GetById(selectedEntity);

			HandleGizmoDrag(selected);
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
		OutputDebugStringA(errorLog.buffer);

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
			float projectionDistanceFromObj = (planeProjection - objectPos).Magnitude();

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
	Transform* sceneCamTrans = scene.transforms.GetById(scene.cam.transform);

	Transform oldSceneCamTrans = *sceneCamTrans;

	*sceneCamTrans = editorCamTrans;
	sceneCamTrans->id = oldSceneCamTrans.id;

	scene.cam.xOffset = leftBarWidth;
	scene.cam.widthPixels = cam.widthPixels - leftBarWidth - rightBarWidth;
	scene.cam.heightPixels = cam.heightPixels - topBarHeight;

	scene.Render();

	int colMatId = -1;
	scene.res.assetIdMap.LookUp("color.mat", &colMatId);
	Material* mat = scene.res.materials.GetById(colMatId);

	Mat4x4 camera = scene.cam.GetCameraMatrix();
	Mat4x4 persp = scene.cam.GetPerspectiveMatrix();

	mat->SeMatrix4Uniform("_camMatrix", camera);
	mat->SeMatrix4Uniform("_perspMatrix", persp);

	Program* prog = scene.res.programs.GetById(mat->programId);
	glUseProgram(prog->programObj);

	for (int i = 0; i < scene.res.drawCalls.currentCount; i++) {
		Entity* ent = scene.entities.GetById(scene.res.drawCalls.vals[i].entId);
		Transform* trans = scene.transforms.GetById(ent->transform);
		mat->SeMatrix4Uniform("_objMatrix", trans->GetLocalToGlobalMatrix());

		if (ent->id == selectedEntity) {
			DrawCurrentGizmo(ent, mat);
		}
	}

	scene.cam.xOffset = 0;
	scene.cam.yOffset = 0;
	scene.cam.widthPixels = cam.widthPixels;
	scene.cam.heightPixels = cam.heightPixels;

	glViewport(0, 0, cam.widthPixels, cam.heightPixels);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	SidePanelGui();

	glDisable(GL_BLEND);

	*sceneCamTrans = oldSceneCamTrans;
}

void Editor::SidePanelGui() {
	if (selectedEntity != -1) {
		Entity* ent = scene.entities.GetById(selectedEntity);

		float y = cam.heightPixels - 20;
		float x = cam.widthPixels - rightBarWidth + 5;
		gui.DrawTextLabel(StringStackBuffer<64>("%d", ent->id).buffer, 0, 12, x, y);
		y -= 14;

		String meshName;
		String matName;
		for (int i = 0; i < scene.res.drawCalls.currentCount; i++) {
			if (scene.res.drawCalls.vals[i].entId == ent->id) {
				uint32 meshId = scene.res.drawCalls.vals[i].meshId;
				uint32 matId = scene.res.drawCalls.vals[i].matId;

				meshName = scene.res.FindFileNameByIdAndExtension("obj", meshId);
				matName = scene.res.FindFileNameByIdAndExtension("mat", matId);
				break;
			}
		}

		if (meshName.string != nullptr) {
			gui.DrawTextLabel(StringStackBuffer<64>("Mesh: %s", meshName.string).buffer, 0, 12, x, y);
			y -= 14;
		}

		if (matName.string != nullptr) {
			gui.DrawTextLabel(StringStackBuffer<64>("Material: %s", matName.string).buffer, 0, 12, x, y);
			y -= 14;
		}

		Transform* entTrans = scene.transforms.GetById(ent->transform);

		{
			float currX = x;
			currX += gui.DrawTextLabel("X:", 0, 12, currX, y);
			currX += gui.DrawTextLabel(StringStackBuffer<64>("%.2f", entTrans->position.x).buffer, 0, 12, currX, y);
			currX += gui.DrawTextLabel("Y:", 0, 12, currX, y);
			currX += gui.DrawTextLabel(StringStackBuffer<64>("%.2f", entTrans->position.y).buffer, 0, 12, currX, y);
			currX += gui.DrawTextLabel("Z:", 0, 12, currX, y);
			currX += gui.DrawTextLabel(StringStackBuffer<64>("%.2f", entTrans->position.z).buffer, 0, 12, currX, y);
		}

		y -= 14;
	}
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

