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
		playerHeight = 0.6f;
		jumpVelocity = 3.0f;
		yVelocity = 0.0f;

		currState = CS_GROUNDED;
	}

	void Update();
};

#endif

