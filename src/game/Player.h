#ifndef PLAYER_H
#define PLAYER_H

#pragma once

#include "../core/Component.h"

enum ControlState {
	CS_GROUNDED,
	CS_JUMPING,
	CS_FALLING
};

struct Player : Component{
	float yVelocity;

	float playerHeight;
	float jumpVelocity;

	ControlState currState;

	Player() {
		playerHeight = 1.2f;
		jumpVelocity = 1.5f;
		yVelocity = 0.0f;

		currState = CS_GROUNDED;
	}

	void Update();
};

#endif

