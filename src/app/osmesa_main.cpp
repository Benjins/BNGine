#include <GL/osmesa.h>

#include "../core/Scene.h"
#include "../gfx/GLExtInit.h"

int main(int argc, char** argv){

	OSMesaContext ctx = OSMesaCreateContext(OSMESA_RGBA, NULL);
	
	int width = 1280, height = 720;
	unsigned char* imgBuffer = (unsigned char*)malloc(width*height*4);
	
	OSMesaMakeCurrent(ctx,  imgBuffer, GL_UNSIGNED_BYTE, width, height);
	
	InitGlExts();
	
	Scene scn;
	
	scn.StartUp();
	
	for(int i = 0; i < 10; i++){
		scn.Update();
		scn.Render();
	}
	
	FILE* frameMid = fopen("frame10.raw", "wb");
	fwrite(imgBuffer, width*height*4, 1, frameMid);
	fclose(frameMid);
	
	OSMesaDestroyContext(ctx);
	
	free(imgBuffer);
	
	return 0;
}
