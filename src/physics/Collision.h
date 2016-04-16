#ifndef COLLISION_H
#define COLLISION_H

#pragma once

#include "../../ext/CppUtils/idbase.h"

enum ColliderType {
	CT_BOX,
	CT_SPHERE
};

enum CollisionState {
	CS_ENTER,
	CS_STAY,
	CS_EXIT
};

struct Collision {
	CollisionState state;

	ColliderType colType1;
	ColliderType colType2;

	uint32 colId1;
	uint32 colId2;
};

struct BoxCollider;
struct SphereCollider;

bool BoxBoxCollision(const BoxCollider& box1, const BoxCollider& box2);

#endif
