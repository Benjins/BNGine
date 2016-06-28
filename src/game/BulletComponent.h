#ifndef BULLETCOMPONENT_H
#define BULLETCOMPONENT_H

#pragma once

#include "../core/Component.h"

struct BulletComponent : Component {
	float killTime;
	float speed;
	float currentTime;

	BulletComponent() {
		killTime = 2;
		speed = 1;
		currentTime = 0;
	}

	void Update();
};

#endif
