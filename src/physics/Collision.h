#ifndef COLLISION_H
#define COLLISION_H

#pragma once

#include "../../ext/CppUtils/idbase.h"

#include "../../ext/3dbasics/Vector3.h"

enum ColliderType {
	CT_BOX,
	CT_SPHERE,
	CT_MESH
};

enum CollisionState {
	CS_NoCollide,
	CS_Enter,
	CS_Stay,
	CS_Exit
};

struct Collision {
	bool isColliding;

	Vector3 normal;
	float depth;

	ColliderType colType1;
	ColliderType colType2;

	uint32 colId1;
	uint32 colId2;
};

struct BoxCollider;
struct SphereCollider;

Collision BoxBoxCollision(const BoxCollider& box1, const BoxCollider& box2);

#endif
