#include "LadderComponent.h"

#include "../physics/BoxCollider.h"

#include "../core/Scene.h"

void LadderComponent::Update() {
	if (collider.id == 0xFFFFFFFF) {
		BoxCollider* col = FIND_COMPONENT_BY_ENTITY(BoxCollider, entity);
		collider = IDHandle<BoxCollider>(col->id);
	}
}

bool LadderComponent::IsInside(Vector3 pos) {
	if (collider.id == 0xFFFFFFFF) {
		return false;
	}
	else {
		BoxCollider* boxCol = GlobalScene->phys.boxCols.GetById(collider);
		return IsInsideBoxCollider(boxCol, pos);
	}
}

