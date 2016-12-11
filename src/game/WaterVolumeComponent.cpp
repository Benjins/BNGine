#include "WaterVolumeComponent.h"

#include "../physics/BoxCollider.h"

#include "../core/Scene.h"

void WaterVolumeComponent::Update() {
	if (collider.id == 0xFFFFFFFF) {
		BoxCollider* col = FIND_COMPONENT_BY_ENTITY(BoxCollider, entity);
		if (col != nullptr) {
			collider = IDHandle<BoxCollider>(col->id);
		}
	}
}

bool WaterVolumeComponent::IsInside(Vector3 pos) {
	if (collider.id == 0xFFFFFFFF) {
		return false;
	}
	else {
		BoxCollider* boxCol = GlobalScene->phys.boxCols.GetById(collider);
		return IsInsideBoxCollider(boxCol, pos);
	}
}

