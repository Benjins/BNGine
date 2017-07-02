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

#include "../metagen/ComponentMeta.h"
#include "../metagen/MetaStruct.h"

#include "../game/GameplayComponents.h"

#include "../gfx/ResourceManager.h"
#include "../physics/PhysicsSystem.h"

#include "../gui/GuiSystem.h"

#include "../anim/Animations.h"

#include "../net/NetworkSystem.h"

#include "../script/ScriptSystem.h"

#include "../../gen/Actions.h"

struct DrawCall;

struct Scene {
	IDTracker<Entity> entities;
	IDTracker<Transform> transforms;
	ResourceManager res;

	uint32 currentLevel;

	Input input;

	GameplayComponents gameplay;
	PhysicsSystem phys;
	ScriptSystem script;
	GuiSystem gui;
	AnimationSystem anims;
	NetworkSystem net;

	Camera cam;

	Timer frameTimer;

	Vector<Action> deferredActions;
	
	float lockedFrameRate;
	bool frameRateIsLocked;

	void LockFrameRate(float rate){
		lockedFrameRate = rate;
		frameRateIsLocked = true;
	}
	
	void UnlockFrameRate(){
		frameRateIsLocked = false;
	}
	
	double GetDeltaTime() {
		return frameRateIsLocked ? lockedFrameRate : frameTimer.GetTimeSince();
	}

	Scene();

	void StartUp();
	void StartUpCustomComponents();
	void ShutDown();

	void Update();
	void UpdateCustomComponents();
	void CustomComponentEditorGui();
	void CustomComponentEditorGuiForEntity(IDHandle<Entity> entId);

	void Render();

	void Reset();
	void ResetComponents();

	void LoadLevel(const char* name);
	void LoadCustomComponentsFromLevel(const Level* level);

	void SaveLevel(Level* level);
	void SaveCustomComponentsToLevel(Level* level);

	Entity* AddEntity();
	Entity* AddEntityWithId(uint32 id);
	Entity* AddVisibleEntityWithIdAndTransId(uint32 id, uint32 transId, IDHandle<Material> matId, IDHandle<Mesh> meshId);
	Entity* AddEntityWithIdAndTrandId(uint32 id, uint32 transId);
	Entity* AddVisibleEntity(IDHandle<Material> matId, IDHandle<Mesh> meshId);
	Entity* AddVisibleEntityWithId(uint32 entId, IDHandle<Material> matId, IDHandle<Mesh> meshId);
	void AddVisibleEntityByEntityPtr(Entity* newEnt, IDHandle<Material> matId, IDHandle<Mesh> meshId);
	DrawCall* GetDrawCallForEntity(IDHandle<Entity> entityId);
	Entity* CloneEntity(Entity* ent);

	void DestroyEntity(IDHandle<Entity> entId);
	void DestroyCustomComponentsByEntity(IDHandle<Entity> entId);

	void SendCollisionToCustomComponents(IDHandle<Entity> entity, Collision col);

	template<typename T>
	T* FindComponentByEntity(CustomComponentType type, IDHandle<Entity> entityId) {
		Component* comp = nullptr;
		Component* compCursor = getComponentArrayFuncs[type]();
		int compCount = getComponentCountFuncs[type]();
		
		for (int i = 0; i < compCount; i++) {
			if (compCursor->entity == entityId) {
				comp = compCursor;
				break;
			}

			compCursor = (Component*)(((char*)compCursor) + componentMetaData[type]->size);
		}

		return static_cast<T*>(comp);
	}

	//Maybe later.  How to go from transform to global transform? 
	//Cache global pos in transform struct?
	//Vector<Mat4x4> cachedTransforms;
};

#define FIND_COMPONENT_BY_ENTITY(compType, entityId) GlobalScene->FindComponentByEntity<compType>(CCT_##compType, entityId)

/*[Action]*/
void DestroyEntityImmediate(uint32 entId);

extern Scene* GlobalScene;

#endif
