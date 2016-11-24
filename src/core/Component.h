#ifndef COMPONENT_H
#define COMPONENT_H

#pragma once

#include "../../ext/CppUtils/idbase.h"

struct Entity;

enum ComponentFlags {
	CF_None = 0,
	CF_RuntimeOnly = (1 << 0)
};

struct Component : IDBase{
	uint32 entity;
	ComponentFlags flags;
	int type;

	Component() 
		: IDBase() {
		flags = CF_None;
	}
};

#endif