#include "RigidBody.h"

#include "../core/Scene.h"



Vector3 RigidBody::GetCentreOfMass() {
	if (!centreOfMassIsCached) {
		BoxCollider* col = FIND_COMPONENT_BY_ENTITY(BoxCollider, entity);
		cachedCentreOfMass = col->position;
	}

	return cachedCentreOfMass;
}

