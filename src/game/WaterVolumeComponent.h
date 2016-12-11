#ifndef WATERVOLUMECOMPONENT_H
#define WATERVOLUMECOMPONENT_H

#pragma once

#include "../core/Component.h"

struct Vector3;
struct BoxCollider;

struct WaterVolumeComponent : Component {
	/*[DoNotSerialize]*/
	IDHandle<BoxCollider> collider;

	void Update();

	bool IsInside(Vector3 pos);
};

#endif
