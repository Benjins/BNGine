#include <GL/osmesa.h>

#include "../gfx/GLExtInit.h"

#include "../app/app_funcs.h"

int main(int argc, char** argv){

	AppPreInit(argc, argv);

	OSMesaContext ctx = OSMesaCreateContext(OSMESA_BGRA, NULL);
	
	imgBuffer = (unsigned char*)malloc(width*height*4);
	
	OSMesaMakeCurrent(ctx, imgBuffer, GL_UNSIGNED_BYTE, width, height);
	
	InitGlExts();

	AppPostInit(argc, argv);
	
	bool success = AppUpdate(argc, argv);
	
	if (!success){
		AppShutdown(argc, argv);
		OSMesaDestroyContext(ctx);
		free(imgBuffer);

		return -1;
	}
	
	
	AppShutdown(argc, argv);
	OSMesaDestroyContext(ctx);
	free(imgBuffer);
	
	return 0;
}
