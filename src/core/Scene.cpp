#include "Scene.h"

Scene* GlobalScene = nullptr;

Scene::Scene() : entities(100), transforms(120), gfx() {
	GlobalScene = this;
}