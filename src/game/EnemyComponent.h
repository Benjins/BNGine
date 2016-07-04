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

struct EnemyComponent : Component {
	EnemyState currentState;

	/*[DoNotSerialize]*/
	float timer;

	/*[DoNotSerialize]*/
	int playerId;

	/*[DoNotSerialize]*/
	int patrolIndex;

	float speed;
	float pauseTime;

	Vector<Vector3> patrolPoints;

	EnemyComponent() {
		currentState = ES_Paused;
		pauseTime = 0.3f;
		timer = 0;
		playerId = -1;
		patrolIndex = 0;
		speed = 2.0f;

		patrolPoints.PushBack(Vector3(2,  0,  2));
		patrolPoints.PushBack(Vector3(2,  0, -2));
		patrolPoints.PushBack(Vector3(-2, 0,  2));
		patrolPoints.PushBack(Vector3(-2, 0, -2));
	}

	void Update();
};

#endif
