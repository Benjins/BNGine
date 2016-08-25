#ifndef PLAYER_H
#define PLAYER_H

#pragma once

#include "../core/Component.h"

enum ControlState {
	CS_GROUNDED,
	CS_JUMPING,
	CS_FALLING,
	CS_FALLINGWATER,
	CS_RISINGWATER,
	CS_LADDERCLIMB
};

struct PlayerComponent : Component{
	/*[DoNotSerialize]*/
	float yVelocity;

	float playerHeight;
	float playerWidth;
	float jumpVelocity;

	ControlState currState;

	PlayerComponent() {
		playerHeight = 0.6f;
		playerWidth = 0.2f;
		jumpVelocity = 3.0f;
		yVelocity = 0.0f;

		currState = CS_GROUNDED;
	}

	bool CheckWater(Vector3 pos);
	bool CheckLadder(Vector3 pos);

	void Update();
};

#endif

