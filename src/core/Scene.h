#ifndef SCENE_H
#define SCENE_H

#pragma once

#include "Entity.h"
#include "../../ext/CppUtils/vector.h"
#include "../../ext/3dbasics/Mat4.h"

#include "Component.h"
#include "Transform.h"
#include "Camera.h"
#include "Input.h"
#include "Timer.h"

#include "../game/GameplayComponents.h"

#include "../gfx/ResourceManager.h"
#include "../physics/PhysicsSystem.h"

#include "../gui/GuiSystem.h"

struct Scene {
	IDTracker<Entity> entities;
	IDTracker<Transform> transforms;
	ResourceManager res;

	uint32 currentLevel;

	Input input;

	GameplayComponents gameplay;

	PhysicsSystem phys;
	
	GuiSystem gui;

	Camera cam;

	Timer frameTimer;

	double GetDeltaTime() {
		return frameTimer.GetTimeSince();
	}

	Scene();

	void StartUp();
	void Update();

	void Render();

	void Reset();
	void ResetComponents();

	void LoadLevel(const char* name);
	void LoadCustomComponentsFromLevel(const Level* level);

	void SaveLevel(Level* level);
	void SaveCustomComponentsToLevel(Level* level);

	Entity* AddVisibleEntity(uint32 matId, uint32 meshId);

	//Maybe later.  How to go from transform to global transform? 
	//Cache global pos in transform struct?
	//Vector<Mat4x4> cachedTransforms;
};


extern Scene* GlobalScene;

#endif
