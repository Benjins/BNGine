#include "../editor/Editor.h"
#define EXT_EXTERN extern
#include "../gfx/GLExtInit.h"

#include "app_funcs.h"
#include "../core/Scene.h"

Editor* GlobalEd = nullptr;

void AppPostInit(int argc, char** argv) {
	GlobalEd = new Editor();

	glClearColor(1.0f, 0.0f, 1.0f, 1.0f);
	glEnable(GL_DEPTH_TEST);
	glViewport(0, 0, 1280, 720);
	glLoadIdentity();

	GlobalEd->StartUp();
}

bool AppUpdate(int argc, char** argv) {
	GlobalEd->Update();
	GlobalEd->Render();

	GlobalEd->gui.EndFrame();
	GlobalEd->scene.input.EndFrame();

	return true;
}

void AppShutdown(int argc, char** argv) {
	//GlobalEd->ShutDown();
	delete GlobalEd;
}

void AppMouseMove(int x, int y) {
	GlobalScene->input.SetCursorPos(x, y);
}

void AppMouseUp(int button) {
	GlobalScene->input.MouseButtonReleased((MouseButton)button);
}

void AppMouseDown(int button) {
	GlobalScene->input.MouseButtonPressed((MouseButton)button);
}

void AppKeyUp(unsigned char key) {
	GlobalScene->input.KeyReleased(key);
}

void AppKeyDown(unsigned char key) {
	GlobalScene->input.KeyPressed(key);
}

void AppPreInit(int argc, char** argv) {

}

void AppSetWindowSize(int w, int h) {
	GlobalEd->cam.widthPixels = w;
	GlobalEd->cam.heightPixels = h;
}
