#include "Editor.h"

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

	if (scene.input.cursorX > leftBarWidth && scene.input.cursorX < cam.widthPixels - leftBarWidth - rightBarWidth 
		&& scene.input.cursorY < cam.heightPixels - topBarHeight
		&& scene.input.MouseButtonIsDown(MouseButton::SECONDARY)) {
		cameraCursorX -= scene.input.cursorDeltaX;
		cameraCursorY -= scene.input.cursorDeltaY;

		editorCamTrans.rotation = Quaternion(Y_AXIS, cameraCursorX / 80) * Quaternion(X_AXIS, cameraCursorY / 80);
	}
}

String str = "Never again!#$%!#%!#@235231";

String str2 = "This isn't even my final form.";

void Editor::Render() {
	Transform* sceneCamTrans = scene.transforms.GetById(scene.cam.transform);

	Transform oldSceneCamTrans = *sceneCamTrans;

	*sceneCamTrans = editorCamTrans;
	sceneCamTrans->id = oldSceneCamTrans.id;

	scene.cam.xOffset = leftBarWidth;
	scene.cam.widthPixels = cam.widthPixels - leftBarWidth - rightBarWidth;
	scene.cam.heightPixels = cam.heightPixels - topBarHeight;

	scene.Render();
	
	scene.cam.xOffset = 0;
	scene.cam.yOffset = 0;
	scene.cam.widthPixels = cam.widthPixels;
	scene.cam.heightPixels = cam.heightPixels;

	glViewport(0, 0, cam.widthPixels, cam.heightPixels);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	{
		str = gui.TextInput(str, 1, 40, 5, 400, 240);
	}

	{
		str2 = gui.TextInput(str2, 1, 12, 5, 500, 240);
	}

	glDisable(GL_BLEND);

	*sceneCamTrans = oldSceneCamTrans;
}

void Editor::StartUp() {
	cam.fov = 80;
	cam.nearClip = 0.001f;
	cam.farClip = 1000.0f;

	leftBarWidth = 250;
	rightBarWidth = 200;
	topBarHeight = 120;

	scene.StartUp();
	gui.Init();
}
