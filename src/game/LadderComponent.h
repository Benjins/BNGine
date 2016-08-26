#ifndef LADDERCOMPONENT_H
#define LADDERCOMPONENT_H

#pragma once

#include "../core/Component.h"

struct Vector3;

struct LadderComponent : Component {
	/*[DoNotSerialize]*/
	int collider;

	LadderComponent() {
		collider = -1;
	}

	void Update();

	bool IsInside(Vector3 pos);
};

#endif
