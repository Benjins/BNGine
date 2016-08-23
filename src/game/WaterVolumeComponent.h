#ifndef WATERVOLUMECOMPONENT_H
#define WATERVOLUMECOMPONENT_H

#pragma once

#include "../core/Component.h"

struct Vector3;

struct WaterVolumeComponent : Component {
	/*[DoNotSerialize]*/
	int collider;

	WaterVolumeComponent() {
		collider = -1;
	}

	void Update();

	bool IsInside(Vector3 pos);
};

#endif
