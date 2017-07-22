#include "EnemyComponent.h"
#include "../core/Scene.h"

CONFIG_VAR(bool, enemydoDownCast, "enemy_floor", false);

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

	const float enemyHeight = 1.5f;

	float floorHeight = -10.0f;

	// TODO: Copied from player code, pull out to common function?
	if (enemydoDownCast) {
		RaycastHit downCast = GlobalScene->phys.Raycast(trans->GetGlobalPosition() + Vector3(0, 0.5f, 0), Y_AXIS * -1);

		if (downCast.wasHit) {
			floorHeight = downCast.globalPos.y;
		}
	}

	switch (currentState) {
		case ES_Patrol: {
			Entity* goalEnt = GlobalScene->entities.GetById(patrolPoints.data[patrolIndex]);
			Vector3 goal = GlobalScene->transforms.GetById(goalEnt->transform)->GetGlobalPosition();
			Vector3 toGoal = goal - trans->GetGlobalPosition();
			if (Vector2(toGoal.x, toGoal.z).Magnitude() < 0.2f){
				patrolIndex = (patrolIndex + 1) % patrolPoints.count;
				currentState = ES_Paused;
				timer = 0;
			}
			else {
				Vector3 moveVec = toGoal.Normalized() * GlobalScene->GetDeltaTime() * speed;
				trans->position = trans->position + moveVec;
				if (enemydoDownCast) {
					trans->position.y = floorHeight;
				}
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
