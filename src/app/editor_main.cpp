#include "../editor/Editor.h"
#define EXT_EXTERN extern
#include "../gfx/GLExtInit.h"

#include "app_funcs.h"
#include "../core/Scene.h"

Editor* GlobalEd = nullptr;

void AppPostInit() {
	GlobalEd = new Editor();

	glClearColor(1.0f, 0.0f, 1.0f, 1.0f);
	glEnable(GL_DEPTH_TEST);
	glViewport(0, 0, 1280, 720);
	glLoadIdentity();

	GlobalEd->StartUp();
}

bool AppUpdate() {
	GlobalEd->Update();
	GlobalEd->Render();

	GlobalEd->gui.EndFrame();
	GlobalEd->scene.input.EndFrame();

	return true;
}

void AppShutdown() {
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
	if (GlobalEd) {
		GlobalEd->cam.widthPixels = w;
		GlobalEd->cam.heightPixels = h;
	}
}

void AppEventFunction(const PlatformEvent& evt) {
	if (evt.IsPreInitEvent()) {
		AppPreInit(evt.AsPreInitEvent().argc, evt.AsPreInitEvent().argv);
	}
	else if (evt.IsPostInitEvent()) {
		AppPostInit();
	}
	else if (evt.IsUpdateEvent()) {
		*evt.AsUpdateEvent().shouldContinue = AppUpdate();
	}
	else if (evt.IsShutDownEvent()) {
		AppShutdown();
	}
	else if (evt.IsMouseButtonEvent()) {
		if (evt.AsMouseButtonEvent().pressOrRelease == BNS_BUTTON_PRESS) {
			AppMouseDown(evt.AsMouseButtonEvent().button);
		}
		else if (evt.AsMouseButtonEvent().pressOrRelease == BNS_BUTTON_RELEASE) {
			AppMouseUp(evt.AsMouseButtonEvent().button);
		}
		else {
			ASSERT(false);
		}
	}
	else if (evt.IsKeyButtonEvent()) {
		if (evt.AsKeyButtonEvent().pressOrRelease == BNS_BUTTON_PRESS) {
			AppKeyDown(evt.AsKeyButtonEvent().keyCode);
		}
		else if (evt.AsKeyButtonEvent().pressOrRelease == BNS_BUTTON_RELEASE) {
			AppKeyUp(evt.AsKeyButtonEvent().keyCode);
		}
		else {
			ASSERT(false);
		}
	}
	else if (evt.IsMousePosEvent()) {
		AppMouseMove(evt.AsMousePosEvent().x, evt.AsMousePosEvent().y);
	}
	else if (evt.IsWindowResizeEvent()) {
		AppSetWindowSize(evt.AsWindowResizeEvent().width, evt.AsWindowResizeEvent().height);
	}
	else {
		ASSERT(false);
	}
}

