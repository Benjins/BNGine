#include "BulletComponent.h"

#include "../core/Scene.h"

void BulletComponent::OnCollision(Collision col) {
	IDHandle<Entity> otherEntity = GlobalScene->phys.boxCols.GetByIdNum(col.colId2)->entity;
	HealthComponent* health = FIND_COMPONENT_BY_ENTITY(HealthComponent, otherEntity);
	if (health && !hasDealtDamage) {
		health->TakeDamage(1.0f);
		hasDealtDamage = true;
	}

	GlobalScene->DestroyEntity(entity);
}

void BulletComponent::Update() {
	currentTime += GlobalScene->GetDeltaTime();

	if (!hasAddedForce) {
		if (RigidBody* rb = FIND_COMPONENT_BY_ENTITY(RigidBody, entity)) {
			Entity* ent = GlobalScene->entities.GetById(entity);
			Transform* trans = GlobalScene->transforms.GetById(ent->transform);

			// TODO: Timestep???
			rb->AddForceAtCentre(trans->Forward() * forceAtStart, 0.02f);
		}

		hasAddedForce = true;
	}
	else {
		// HACK: Clear out velocity
		if (RigidBody* rb = FIND_COMPONENT_BY_ENTITY(RigidBody, entity)) {
			rb->acceleration = Vector3(0, 0, 0);
		}
	}

	

	if (currentTime >= killTime) {
		GlobalScene->DestroyEntity(entity);
	}
}
