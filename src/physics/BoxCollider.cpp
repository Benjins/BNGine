#include "BoxCollider.h"

#include "../core/Scene.h"

#include "../gui/Debug3DGUI.h"

bool WithinVolume(Vector3 pos, Vector3 min, Vector3 max) {
	return min.x < pos.x && pos.x < max.x
		&& min.y < pos.y && pos.y < max.y
		&& min.z < pos.z && pos.z < max.z;
}

bool IsInsideBoxCollider(const BoxCollider* col, Vector3 pos){
	Entity* ent = GlobalScene->entities.GetById(col->entity);
	Transform* trans = GlobalScene->transforms.GetById(ent->transform);

	Vector3 localPos = trans->GetGlobaltoLocalMatrix().MultiplyAsPosition(pos);

	Vector3 boxMin = col->position - col->size;
	Vector3 boxMax = col->position + col->size;

	return WithinVolume(localPos, boxMin, boxMax);
}

void BoxCollider::EditorGui() {
	Entity* ent = GlobalScene->entities.GetById(entity);
	Transform* trans = GlobalScene->transforms.GetById(ent->transform);

	DebugDrawWireCube(position, size, trans);
}

