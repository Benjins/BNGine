#ifndef SPHERECOLLIDER_H
#define SPHERECOLLIDER_H

#pragma once

#include "ColliderFlags.h"

#include "../core/Component.h"

#include "../../ext/3dbasics/Vector3.h"

struct SphereCollider : Component {
	ColliderFlags colFlags;
	Vector3 position;
	float radius;

	SphereCollider() {
		colFlags = CF_NONE;
	}

	SphereCollider(const SphereCollider& orig) {
		colFlags = orig.colFlags;
	}

	SphereCollider& operator=(const SphereCollider& orig) {
		colFlags = orig.colFlags;
		return *this;
	}
};

#endif
