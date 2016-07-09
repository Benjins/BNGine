#include "BulletComponent.h"

#include "../core/Scene.h"

void BulletComponent::OnCollision(Collision col) {
	uint32 otherEntity = GlobalScene->phys.boxCols.GetById(col.colId2)->entity;
	HealthComponent* health = FIND_COMPONENT_BY_ENTITY(HealthComponent, otherEntity);
	if (health && !hasDealtDamage) {
		health->TakeDamage(1.0f);
		hasDealtDamage = true;
	}

	GlobalScene->DestroyEntity(entity);
}

void BulletComponent::Update() {
	currentTime += GlobalScene->GetDeltaTime();

	Entity* ent = GlobalScene->entities.GetById(entity);
	Transform* trans = GlobalScene->transforms.GetById(ent->transform);

	trans->position = trans->position + trans->Forward() * GlobalScene->GetDeltaTime() * speed;

	if (currentTime >= killTime) {
		GlobalScene->DestroyEntity(entity);
	}
}
