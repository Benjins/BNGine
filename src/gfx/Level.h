#ifndef LEVEL_H
#define LEVEL_H

#pragma once

#include "../core/Camera.h"

#include "../../ext/CppUtils/idbase.h"
#include "../../ext/CppUtils/vector.h"

struct Transform;
struct Entity;
struct BoxCollider;
struct SphereCollider;
struct PlayerComponent;
struct BulletComponent;

struct Level : IDBase{
	Camera cam;
	Vector<Transform> transforms;
	Vector<Entity> entities;

	Vector<int> meshIds;
	Vector<int> matIds;

	// TODO: Automate
	Vector<BoxCollider> boxCols;
	Vector<SphereCollider> sphereCols;
	Vector<PlayerComponent> playerComps;
	Vector<BulletComponent> bulletComps;
};

#endif
