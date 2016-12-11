#ifndef LADDERCOMPONENT_H
#define LADDERCOMPONENT_H

#pragma once

#include "../core/Component.h"

struct Vector3;

struct LadderComponent : Component {
	/*[DoNotSerialize]*/
	IDHandle<BoxCollider> collider;

	void Update();

	bool IsInside(Vector3 pos);
};

#endif
