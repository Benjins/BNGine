#include "Scene.h"

Scene* GlobalScene = nullptr;

Scene::Scene() : entities(), transforms(), gfx() {
	GlobalScene = this;
}