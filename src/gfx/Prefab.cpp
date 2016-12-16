#include "Prefab.h"
#include "../core/Scene.h"

#include "../metagen/ComponentMeta.h"

void Prefab::InstantiateIntoEntityPtr(Entity* toAdd, Vector3 position, Quaternion rotation) {
	Transform* trans = GlobalScene->transforms.GetById(toAdd->transform);
	trans->scale = transform.scale;
	trans->position = position;
	trans->rotation = rotation;

	void* customComponentsOldReadHead = customComponents.readHead;
	while (customComponents.GetLength() > 0) {
		uint32 compId = customComponents.Read<uint32>();
		ASSERT(compId >= 0 && compId < CCT_Count);

		Component* comp = addComponentFuncs[compId]();
		comp->entity = IDHandle<Entity>(toAdd->id);

		componentMemDeserializeFuncs[compId](comp, &customComponents);
	}

	customComponents.readHead = customComponentsOldReadHead;

	PrefabInstanceComponent* instComp = GlobalScene->gameplay.prefabInsts.CreateAndAdd();
	instComp->prefab = GET_PTR_HANDLE(this);
}

Entity* Prefab::Instantiate(Vector3 position, Quaternion rotation /*= QUAT_IDENTITY*/) {
	Entity* toAdd = GlobalScene->AddVisibleEntity(matId, meshId);

	InstantiateIntoEntityPtr(toAdd, position, rotation);

	return toAdd;
}

Entity* Prefab::InstantiateWithIdAndTransId(uint32 id, uint32 transId, Vector3 position, Quaternion rotation /*= QUAT_IDENTITY*/) {
	Entity* toAdd;
	if (matId.id != -1 && meshId.id != -1) {
		toAdd = GlobalScene->AddVisibleEntityWithIdAndTransId(id,transId, matId, meshId);
	}
	else {
		toAdd = GlobalScene->AddEntityWithIdAndTrandId(id, transId);
	}

	InstantiateIntoEntityPtr(toAdd, position, rotation);

	return toAdd;
}

Entity* Prefab::InstantiateWithId(uint32 id, Vector3 position, Quaternion rotation /*= QUAT_IDENTITY*/) {
	Entity* toAdd;
	if (matId.id != -1 && meshId.id != -1) {
		toAdd = GlobalScene->AddVisibleEntityWithId(id, matId, meshId);
	}
	else {
		toAdd = GlobalScene->AddEntityWithId(id);
	}

	InstantiateIntoEntityPtr(toAdd, position, rotation);

	return toAdd;
}
