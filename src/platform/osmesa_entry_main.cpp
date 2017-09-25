#include <GL/osmesa.h>

#include "../gfx/GLExtInit.h"

#include "../app/app_funcs.h"

int main(int argc, char** argv){

	{
		PreInitEvent evt;
		evt.argc = argc;
		evt.argv = argv;
		AppEventFunction(evt);
	}

	OSMesaContext ctx = OSMesaCreateContext(OSMESA_BGRA, NULL);
	
	imgBuffer = (unsigned char*)malloc(width*height*4);
	
	OSMesaMakeCurrent(ctx, imgBuffer, GL_UNSIGNED_BYTE, width, height);
	
	InitGlExts();

	AppEventFunction(PostInitEvent());
	
	bool success = true;
	{
		UpdateEvent evt;
		evt.shouldContinue = &success;
		AppEventFunction(evt);
	}

	if (!success){
		AppEventFunction(ShutDownEvent());
		OSMesaDestroyContext(ctx);
		free(imgBuffer);

		return -1;
	}

	AppEventFunction(ShutDownEvent());
	OSMesaDestroyContext(ctx);
	free(imgBuffer);
	
	return 0;
}
