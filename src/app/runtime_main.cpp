#include "app_funcs.h"
#include "../core/Scene.h"

void AppPostInit(int argc, char** argv) {
	GlobalScene = new Scene();

	glClearColor(1.0f, 0.0f, 1.0f, 1.0f);
	glEnable(GL_DEPTH_TEST);
	glViewport(0, 0, 1280, 720);
	glLoadIdentity();

	GlobalScene->StartUp();
}

bool AppUpdate(int argc, char** argv) {
	GlobalScene->Update();
	GlobalScene->Render();

	GlobalScene->gui.EndFrame();
	GlobalScene->input.EndFrame();

	return true;
}

void AppShutdown(int argc, char** argv) {
	GlobalScene->ShutDown();
	delete GlobalScene;
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

void AppSetWindowSize(int w, int h){
	if (GlobalScene){
		GlobalScene->cam.widthPixels = w;
		GlobalScene->cam.heightPixels = h;
	}
}

