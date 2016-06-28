#include "BulletComponent.h"

#include "../core/Scene.h"

void BulletComponent::Update() {
	currentTime += GlobalScene->GetDeltaTime();

	Entity* ent = GlobalScene->entities.GetById(entity);
	Transform* trans = GlobalScene->transforms.GetById(ent->transform);

	trans->position = trans->position + trans->Forward() * GlobalScene->GetDeltaTime() * speed;

	if (currentTime >= killTime) {
		GlobalScene->DestroyEntity(entity);
	}
}
