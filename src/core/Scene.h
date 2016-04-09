#ifndef SCENE_H
#define SCENE_H

#pragma once

#include "Entity.h"
#include "../../ext/CppUtils/vector.h"
#include "../../ext/3dbasics/Mat4.h"

#include "Component.h"
#include "Transform.h"

#include "../gfx/GfxResManager.h"

struct Scene {
	IDTracker<Entity> entities;
	IDTracker<Transform> transforms;
	GfxResManager gfx;

	Scene();

	//Maybe later.  How to go from transform to global transform? 
	//Cache global pos in transform struct?
	//Vector<Mat4x4> cachedTransforms;
};


extern Scene* GlobalScene;

#endif
