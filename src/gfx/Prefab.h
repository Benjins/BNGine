#ifndef PREFAB_H
#define PREFAB_H

#pragma once

#include "../core/Transform.h"

#include "../../ext/CppUtils/idbase.h"
#include "../../ext/CppUtils/memstream.h"

struct Entity;

struct Prefab : IDBase{
	Transform transform;
	uint32 matId;
	uint32 meshId;

	MemStream customComponents;

	Entity* Instantiate(Vector3 position, Quaternion rotation = QUAT_IDENTITY);
};


#endif
