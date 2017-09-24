#ifndef BULLETCOMPONENT_H
#define BULLETCOMPONENT_H

#pragma once

#include "../core/Component.h"

struct Collision;

struct BulletComponent : Component {
	float killTime;

	/*[DoNotSerialize]*/
	float currentTime;

	/*[DoNotSerialize]*/
	IDHandle<Entity> ownerId;

	/*[DoNotSerialize]*/
	bool hasDealtDamage;

	/*[DoNotSerialize]*/
	bool hasAddedForce;

	float forceAtStart;

	BulletComponent() {
		hasDealtDamage = false;
		hasAddedForce = false;
		killTime = 2;
		forceAtStart = 100.0f;
		currentTime = 0;
	}

	void Update();

	void OnCollision(Collision col);
};

#endif
