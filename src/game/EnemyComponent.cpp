#include "EnemyComponent.h"
#include "../core/Scene.h"

void EnemyComponent::Update() {
	if (playerId.id == 0xFFFFFFFF) {
		playerId = GlobalScene->gameplay.players.vals[0].entity;
	}

	if (!hasCalculatedPatrolPoints) {
		patrolPoints.EnsureCapacity(GlobalScene->gameplay.patrolPoints.currentCount);
		for (int i = 0; i < GlobalScene->gameplay.patrolPoints.currentCount; i++) {
			patrolPoints.PushBack(GlobalScene->gameplay.patrolPoints.vals[i].entity);
		}

		ASSERT_MSG(patrolPoints.count > 0, "Must have patrol points if have an enemy in scene %d", GlobalScene->currentLevel)

		hasCalculatedPatrolPoints = true;
	}

	Transform* trans = GlobalScene->transforms.GetById(GlobalScene->entities.GetById(entity)->transform);

	HealthComponent* health = FIND_COMPONENT_BY_ENTITY(HealthComponent, entity);
	if (health->IsDead()) {
		GlobalScene->DestroyEntity(entity);
	}

	switch (currentState) {
		case ES_Patrol: {
			Entity* goalEnt = GlobalScene->entities.GetById(patrolPoints.data[patrolIndex]);
			Vector3 goal = GlobalScene->transforms.GetById(goalEnt->transform)->GetGlobalPosition();
			Vector3 toGoal = goal - trans->GetGlobalPosition();
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
