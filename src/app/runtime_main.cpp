#include "app_funcs.h"
#include "../core/Scene.h"

#include "../../ext/CppUtils/commandline.h"

CommandLineParser parser;

void AppPostInit() {
	GlobalScene = new Scene();

	glClearColor(1.0f, 0.0f, 1.0f, 1.0f);
	glEnable(GL_DEPTH_TEST);
	glViewport(0, 0, 1280, 720);
	glLoadIdentity();

	GlobalScene->StartUp();

	if (parser.IsFlagPresent("-port")) {
		int port = parser.FlagIntValue("-port");
		GlobalScene->net.Initialize(port);
	}

	if (parser.IsFlagPresent("-connPort")) {
		GlobalScene->net.debugPortToConnectTo = parser.FlagIntValue("-connPort");
	}
}

bool AppUpdate() {
	GlobalScene->Update();
	GlobalScene->Render();

	GlobalScene->gui.EndFrame();
	GlobalScene->input.EndFrame();

	return true;
}

void AppShutdown() {
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
	StartUpSocketSystem();

	// A bit redundant, since we do this already on Windows to get argc/argv...
	CommandLineOption options[] = {
		{ "-port", "Port to bind socket to", 1, 1 },
		{ "-connPort", "Local port to connect to", 1, 1 }
	};

	parser.SetOptions(options, BNS_ARRAY_COUNT(options));
	parser.isProgramNamePresent = false;
	parser.InitializeFromArgcArgv(argc, (const char**)argv);
}

void AppSetWindowSize(int w, int h){
	if (GlobalScene){
		GlobalScene->cam.widthPixels = w;
		GlobalScene->cam.heightPixels = h;
	}
}

// TODO: Code dup w/ editor main?
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

