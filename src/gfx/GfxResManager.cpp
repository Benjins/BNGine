#include "GfxResManager.h"
#include "../core/Scene.h"

GfxResManager::GfxResManager() 
	: programs(20), shaders(30), materials(15), meshes(30), drawCalls(40){

}

void GfxResManager::Render() {
	glViewport(0, 0, (int)GlobalScene->cam.widthPixels, (int)GlobalScene->cam.heightPixels);

	//TODO: Camera matrix

	ExecuteDrawCalls(drawCalls.vals, drawCalls.currentCount);
}