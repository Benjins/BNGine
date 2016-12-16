#ifndef PREFAB_H
#define PREFAB_H

#pragma once

#include "../core/Transform.h"

#include "../../ext/CppUtils/idbase.h"
#include "../../ext/CppUtils/memstream.h"

struct Entity;

struct Prefab : IDBase{
	Transform transform;
	IDHandle<Material> matId;
	IDHandle<Mesh> meshId;

	MemStream customComponents;

	Entity* Instantiate(Vector3 position, Quaternion rotation = QUAT_IDENTITY);
	Entity* InstantiateWithId(uint32 id, Vector3 position, Quaternion rotation = QUAT_IDENTITY);
	Entity* InstantiateWithIdAndTransId(uint32 id, uint32 transId, Vector3 position, Quaternion rotation = QUAT_IDENTITY);
	void InstantiateIntoEntityPtr(Entity* toAdd, Vector3 position, Quaternion rotation);
};


#endif
