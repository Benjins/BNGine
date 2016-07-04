#include "EnemyComponent.h"
#include "../core/Scene.h"

void EnemyComponent::Update() {
	if (playerId == -1) {
		playerId = GlobalScene->gameplay.players.vals[0].entity;
	}

	Transform* playerTrans = GlobalScene->transforms.GetById(GlobalScene->entities.GetById(playerId)->transform);
	Transform* trans       = GlobalScene->transforms.GetById(GlobalScene->entities.GetById(entity)->transform);

	switch (currentState) {
		case ES_Patrol: {
			Vector3 toGoal = patrolPoints.data[patrolIndex] - trans->GetGlobalPosition();
			if (toGoal.Magnitude() < 0.2f){
				patrolIndex = (patrolIndex + 1) % patrolPoints.count;
				currentState = ES_Paused;
				timer = 0;
			}
			else {
				Vector3 moveVec = toGoal.Normalized() * GlobalScene->GetDeltaTime() * speed;
				trans->position = trans->position + moveVec;
			}
		} break;

		case ES_Paused: {
			timer += GlobalScene->GetDeltaTime();

			if (timer > pauseTime) {
				currentState = ES_Patrol;
				timer = 0;
			}
		} break;

		case ES_Chase: {

		} break;

		case ES_AttackWait: {

		} break;

		case ES_Attack: {

		} break;
	}
}
