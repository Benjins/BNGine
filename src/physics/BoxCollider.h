#ifndef BOXCOLLIDER_H
#define BOXCOLLIDER_H

#pragma once

#include "ColliderFlags.h"

#include "../core/Component.h"

#include "../../ext/3dbasics/Vector3.h"

struct BoxCollider : Component {
	ColliderFlags colFlags;
	bool isTrigger;
	Vector3 position;
	Vector3 size;

	BoxCollider() {
		isTrigger = false;
		colFlags = CF_NONE;
	}

	void EditorGui();
};

bool WithinVolume(Vector3 pos, Vector3 min, Vector3 max);

bool IsInsideBoxCollider(const BoxCollider* col, Vector3 pos);

#endif
