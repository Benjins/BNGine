#ifndef BOXCOLLIDER_H
#define BOXCOLLIDER_H

#pragma once

#include "ColliderFlags.h"

#include "../core/Component.h"

#include "../../ext/3dbasics/Vector3.h"

struct BoxCollider : Component {
	ColliderFlags flags;
	bool isTrigger;
	Vector3 position;
	Vector3 size;

	BoxCollider() {
		isTrigger = false;
	}
};

bool WithinVolume(Vector3 pos, Vector3 min, Vector3 max);

#endif
