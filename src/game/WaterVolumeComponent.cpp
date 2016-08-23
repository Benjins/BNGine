#include "WaterVolumeComponent.h"

#include "../core/Scene.h"

void WaterVolumeComponent::Update() {
	if (collider == -1) {
		BoxCollider* col = FIND_COMPONENT_BY_ENTITY(BoxCollider, entity);
		if (col != nullptr) {
			collider = col->id;
		}
	}
}

bool WaterVolumeComponent::IsInside(Vector3 pos) {
	if (collider == -1) {
		return false;
	}
	else {
		BoxCollider* boxCol = GlobalScene->phys.boxCols.GetById(collider);
		Entity* ent = GlobalScene->entities.GetById(entity);
		Transform* trans = GlobalScene->transforms.GetById(ent->transform);

		Vector3 localPos = trans->GetGlobaltoLocalMatrix().MultiplyAsPosition(pos);

		Vector3 boxColMin = boxCol->position - boxCol->size;
		Vector3 boxColMax = boxCol->position + boxCol->size;

		return WithinVolume(localPos, boxColMin, boxColMax);
	}
}

