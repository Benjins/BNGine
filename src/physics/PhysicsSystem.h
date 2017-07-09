#ifndef PHYSICSSYSTEM_H
#define PHYSICSSYSTEM_H

#pragma once

#include "BoxCollider.h"
#include "SphereCollider.h"
#include "MeshCollider.h"
#include "Collision.h"

#include "../../ext/CppUtils/idbase.h"
#include "../../ext/CppUtils/vector.h"

#include "../../ext/3dbasics/Vector2.h"

struct RaycastHit {
	Vector3 globalPos;
	Vector3 globalNormal;
	Vector2 texCoords;
	float depth;
	uint32 colId;
	ColliderType type;
	bool wasHit;
};

struct PhysicsSystem {
	IDTracker<BoxCollider> boxCols;
	IDTracker<SphereCollider> sphereCols;
	IDTracker<MeshCollider> meshCols;

	Vector<Collision> collisions;
	Vector<Collision> prevCollisions;

	RaycastHit Raycast(Vector3 origin, Vector3 direction);

	static float fixedTimestep;

	float timeOffset;

	PhysicsSystem() {
		timeOffset = 0;
	}

	void AdvanceTime(float time);
	void StepFrame(float dt);
	void EndFrame();
};

RaycastHit RaycastBox(Vector3 origin, Vector3 direction, BoxCollider* boxCol);
RaycastHit RaycastSphere(Vector3 origin, Vector3 direction, SphereCollider* sphereCol);
RaycastHit RaycastMesh(Vector3 origin, Vector3 direction, MeshCollider* meshCol);

#endif
