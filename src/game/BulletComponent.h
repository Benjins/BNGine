#ifndef BULLETCOMPONENT_H
#define BULLETCOMPONENT_H

#pragma once

#include "../core/Component.h"

struct Collision;

struct BulletComponent : Component {
	float killTime;
	float speed;

	/*[DoNotSerialize]*/
	float currentTime;

	/*[DoNotSerialize]*/
	int ownerId;

	/*[DoNotSerialize]*/
	bool hasDealtDamage;

	BulletComponent() {
		hasDealtDamage = false;
		killTime = 2;
		speed = 1;
		currentTime = 0;
		ownerId = -1;
	}

	void Update();

	void OnCollision(Collision col);
};

#endif
