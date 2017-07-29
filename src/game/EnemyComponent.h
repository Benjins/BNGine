#ifndef ENEMYCOMPONENT_H
#define ENEMYCOMPONENT_H

#pragma once

#include "../core/Component.h"

#include "../../ext/3dbasics/Vector3.h"

#include "../../ext/CppUtils/vector.h"

enum EnemyState {
	ES_Patrol,
	ES_Paused,
	ES_Chase,
	ES_AttackWait,
	ES_Attack
};

struct Collision;

struct EnemyComponent : Component {
	EnemyState currentState;

	/*[DoNotSerialize]*/
	float timer;

	/*[DoNotSerialize]*/
	IDHandle<Entity> playerId;

	/*[DoNotSerialize]*/
	int patrolIndex;

	/*[DoNotSerialize]*/
	bool damageThisFrame;

	float speed;
	float pauseTime;

	Vector<IDHandle<Entity>> patrolPoints;

	/*[DoNotSerialize]*/
	bool hasCalculatedPatrolPoints;

	EnemyComponent() {
		currentState = ES_Paused;
		pauseTime = 0.3f;
		timer = 0;
		patrolIndex = 0;
		speed = 2.0f;
		damageThisFrame = false;
		hasCalculatedPatrolPoints = false;
	}

	void Update();
};

#endif
