#include <stdio.h>

#include "../app/app_funcs.h"


int main(int argc, char** argv) {

	AppPreInit(argc, argv);

	AppPostInit(argc, argv);

	bool isRunning = true;
	while (isRunning) {
		isRunning = AppUpdate(argc, argv);
	}

	AppShutdown(argc, argv);

	return 0;
}
