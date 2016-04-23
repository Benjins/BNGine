#include "Editor.h"

void Editor::Update() {
	//scene.Update();

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

	editorCamTrans.position = editorCamTrans.position + moveVec/50;
	editorCamTrans.rotation = Quaternion(Y_AXIS, scene.input.cursorX / 80) * Quaternion(X_AXIS, scene.input.cursorY / 80 - 2);
}

void Editor::Render() {
	Transform* sceneCamTrans = scene.transforms.GetById(scene.cam.transform);

	Transform oldSceneCamTrans = *sceneCamTrans;

	*sceneCamTrans = editorCamTrans;
	sceneCamTrans->id = oldSceneCamTrans.id;
	scene.Render();
	*sceneCamTrans = oldSceneCamTrans;
}

void Editor::StartUp() {
	cam.fov = 80;
	cam.nearClip = 0.001f;
	cam.farClip = 1000.0f;
	
	scene.StartUp();
}
