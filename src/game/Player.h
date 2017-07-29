#ifndef PLAYER_H
#define PLAYER_H

#pragma once

#include "../core/Component.h"

struct Vector3;

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

	/*[DoNotSerialize]*/
	bool disablePlayerInput;

	float playerHeight;
	float playerWidth;
	float jumpVelocity;

	float currHealth;
	float maxHealth;

	float movementSpeed;

	ControlState currState;

	PlayerComponent() {
		playerHeight = 0.6f;
		playerWidth = 0.2f;
		jumpVelocity = 3.0f;
		yVelocity = 0.0f;

		movementSpeed = 3.0f;

		currHealth = 5;
		maxHealth = 5;

		currState = CS_GROUNDED;

		disablePlayerInput = false;
	}

	bool CheckWater(Vector3 pos);
	bool CheckLadder(Vector3 pos);

	void Start();
	void Update();
};

#endif

