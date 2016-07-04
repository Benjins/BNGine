#include "BulletComponent.h"

#include "../core/Scene.h"

void BulletComponent::Update() {
	currentTime += GlobalScene->GetDeltaTime();

	Entity* ent = GlobalScene->entities.GetById(entity);
	Transform* trans = GlobalScene->transforms.GetById(ent->transform);

	trans->position = trans->position + trans->Forward() * GlobalScene->GetDeltaTime() * speed;

	int colId = -1;
	for (int i = 0; i < GlobalScene->phys.boxCols.currentCount; i++) {
		BoxCollider col = GlobalScene->phys.boxCols.vals[i];
		if (col.entity == entity) {
			colId = col.id;
			break;
		}
	}

	for (int i = 0; i < GlobalScene->phys.collisions.count; i++) {
		Collision col = GlobalScene->phys.collisions.Get(i);
		if ((col.colType1 == CT_BOX && col.colId1 == colId) 
		 || (col.colType2 == CT_BOX && col.colId2 == colId)) {
			GlobalScene->DestroyEntity(entity);
			break;
		}
	}

	if (currentTime >= killTime) {
		GlobalScene->DestroyEntity(entity);
	}
}
