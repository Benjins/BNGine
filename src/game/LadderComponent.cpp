#include "LadderComponent.h"

#include "../physics/BoxCollider.h"

#include "../core/Scene.h"

void LadderComponent::Update() {
	if (collider == -1) {
		for (int i = 0; i < GlobalScene->phys.boxCols.currentCount; i++) {
			BoxCollider* boxCol = &GlobalScene->phys.boxCols.vals[i];
			if (boxCol->entity == entity
			 && boxCol->isTrigger) {
				collider = boxCol->id;
				break;
			}
		}
	}
}

bool LadderComponent::IsInside(Vector3 pos) {
	if (collider == -1) {
		return false;
	}
	else {
		BoxCollider* boxCol = GlobalScene->phys.boxCols.GetById(collider);
		return IsInsideBoxCollider(boxCol, pos);
	}
}

