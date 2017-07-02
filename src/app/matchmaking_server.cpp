#include <stdio.h>

#include "app_funcs.h"

#include "../net/NetworkingServer.h"

void AppPostInit(int argc, char** argv) {}
bool AppUpdate(int argc, char** argv) { return false; }
void AppShutdown(int argc, char** argv) {}
void AppMouseMove(int x, int y) {}
void AppMouseUp(int button) {}
void AppMouseDown(int button) {}
void AppKeyUp(unsigned char key) {}
void AppKeyDown(unsigned char key) {}

void AppPreInit(int argc, char** argv) {
	if (!StartUpSocketSystem()) {
		printf("ERROR: Failed to startup socket system!\n");
	}
	else {
		DoServer();
	}
}
