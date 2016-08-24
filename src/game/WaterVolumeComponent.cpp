#include "WaterVolumeComponent.h"

#include "../physics/BoxCollider.h"

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
		return IsInsideBoxCollider(boxCol, pos);
	}
}

