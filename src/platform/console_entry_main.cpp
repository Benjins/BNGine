#include <stdio.h>

#include "../app/app_funcs.h"


int main(int argc, char** argv) {

	{
		PreInitEvent evt;
		evt.argc = argc;
		evt.argv = argv;
		AppEventFunction(evt);
	}

	bool isRunning = true;
	while (isRunning) {
		UpdateEvent evt;
		evt.shouldContinue = &isRunning;
		AppEventFunction(evt);
	}

	AppEventFunction(ShutDownEvent());

	return 0;
}
