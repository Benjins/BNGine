#include "Prefab.h"
#include "../core/Scene.h"

#include "../metagen/ComponentMeta.h"

Entity* Prefab::Instantiate(Vector3 position, Quaternion rotation /*= QUAT_IDENTITY*/) {
	Entity* toAdd = GlobalScene->AddVisibleEntity(matId, meshId);

	Transform* trans = GlobalScene->transforms.GetById(toAdd->transform);
	trans->scale = transform.scale;
	trans->position = position;
	trans->rotation = rotation;

	void* customComponentsOldReadHead = customComponents.readHead;
	while (customComponents.GetLength() > 0) {
		uint32 compId = customComponents.Read<uint32>();
		ASSERT(compId >= 0 && compId < CCT_Count);

		Component* comp = addComponentFuncs[compId]();
		comp->entity = toAdd->id;

		componentMemDeserializeFuncs[compId](comp, &customComponents);
	}

	customComponents.readHead = customComponentsOldReadHead;

	return toAdd;
}
