#ifndef PHYS_MODULE_COMP_H
#define PHYS_MODULE_COMP_H

#pragma once

#include "PhysicsModule.h"
#include "../core/Component.h"

#include "../gui/Debug3DGUI.h"

struct PhysModuleComp : Component {
	Phys2System sys;

	void Update();
};

#endif
