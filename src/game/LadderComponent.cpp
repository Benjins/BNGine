#include "LadderComponent.h"

#include "../physics/BoxCollider.h"

#include "../core/Scene.h"

void LadderComponent::Update() {
	if (collider.id == 0xFFFFFFFF) {
		int idx = 0;
		BoxCollider* col = nullptr;
		while (true) {
			BoxCollider* thisCol = FIND_COMPONENT_BY_ENTITY_AND_INDEX(BoxCollider, entity, idx);
			if (thisCol == nullptr) {
				break;
			}
			else if (thisCol->isTrigger) {
				col = thisCol;
			}

			idx++;
		}

		ASSERT(col != nullptr);

		collider = IDHandle<BoxCollider>((col == nullptr) ? -1 : col->id);
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

