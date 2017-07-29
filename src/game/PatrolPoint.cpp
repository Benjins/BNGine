#include "PatrolPoint.h"

#include "../gui/Debug3DGUI.h"
#include "../core/Scene.h"

bool shouldDisplayPatrolPoints = true;

void PatrolPointComponent::EditorGui(bool isSelected) {
	if (GlobalScene->input.KeyIsReleased('L')) {
		shouldDisplayPatrolPoints = !shouldDisplayPatrolPoints;
	}

	if (shouldDisplayPatrolPoints) {
		Entity* ent = GlobalScene->entities.GetById(entity);
		Transform* trans = GlobalScene->transforms.GetById(ent->transform);

		Vector3 position = Vector3(0, 0, 0);
		Vector3 size = Vector3(0.2f, 0.2f, 0.2f);
		DebugDrawWireCube(position, size, trans);
	}
}
