#include "EnemyComponent.h"
#include "../core/Scene.h"

void EnemyComponent::Update() {
	if (playerId == -1) {
		playerId = GlobalScene->gameplay.players.vals[0].entity;
	}

	Transform* playerTrans = GlobalScene->transforms.GetById(GlobalScene->entities.GetById(playerId)->transform);
	Transform* trans       = GlobalScene->transforms.GetById(GlobalScene->entities.GetById(entity)->transform);

	for (int i = 0; i < GlobalScene->phys.collisions.count; i++) {
		Collision col = GlobalScene->phys.collisions.data[i];
		BoxCollider* col1 = GlobalScene->phys.boxCols.GetById(col.colId1);
		BoxCollider* col2 = GlobalScene->phys.boxCols.GetById(col.colId2);

		int colEntity = -1;
		if (col1->entity == entity) {
			colEntity = col2->entity;
		}
		else if (col2->entity == entity) {
			colEntity = col1->entity;
		}

		bool collideWithBullet = false;
		if (colEntity != -1) {
			for (int i = 0; i < GlobalScene->gameplay.bullets.currentCount; i++) {
				if (GlobalScene->gameplay.bullets.vals[i].entity == col2->entity) {
					collideWithBullet = true;
					break;
				}
			}
		}

		if (collideWithBullet) {
			for (int i = 0; i < GlobalScene->gameplay.healthComps.currentCount; i++) {
				HealthComponent* health = &GlobalScene->gameplay.healthComps.vals[i];
				if (health->entity == entity) {
					health->TakeDamage(1.0f);

					if (health->IsDead()) {
						GlobalScene->DestroyEntity(entity);
					}

					break;
				}
			}
		}
	}

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
