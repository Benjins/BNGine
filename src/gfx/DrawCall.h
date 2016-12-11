#ifndef DRAWCALL_H
#define DRAWCALL_H

#pragma once

#include "../../ext/CppUtils/idbase.h"

struct Material;
struct Mesh;
struct Entity;

struct DrawCall : IDBase {
	IDHandle<Mesh> meshId;
	IDHandle<Material> matId;
	IDHandle<Entity> entId;
};

void ExecuteDrawCalls(DrawCall* calls, int count);

#endif
