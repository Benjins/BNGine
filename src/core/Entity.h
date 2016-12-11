#ifndef ENTITY_H
#define ENTITY_H

#pragma once

#include "../../ext/CppUtils/idbase.h"

struct Transform;

struct Entity : IDBase{
	IDHandle<Transform> transform;
};


#endif