#ifndef BOXCOLLIDER_H
#define BOXCOLLIDER_H

#pragma once

#include "ColliderFlags.h"

#include "../core/Component.h"

#include "../../ext/3dbasics/Vector3.h"

struct BoxCollider : Component {
	ColliderFlags flags;
	Vector3 position;
	Vector3 size;
};

#endif
