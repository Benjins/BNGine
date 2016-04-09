#ifndef COMPONENT_H
#define COMPONENT_H

#pragma once

#include "../../ext/CppUtils/idbase.h"

struct Entity;

struct Component : IDBase{
	uint32 entity;
	uint32 flags;
};

#endif