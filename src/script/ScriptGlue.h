#include "ScriptObject.h"

#include "../../ext/3dbasics/Vector3.h"

#include "../core/Scene.h"

/*[ScriptFunc]*/
void SetLocalPosition(int entId, Vector3 pos) {
	Entity* ent = GlobalScene->entities.GetById(entId);
	Transform* trans = GlobalScene->transforms.GetById(ent->transform);
	trans->position = pos;
}

/*[ScriptFunc]*/
Vector3 GetLocalPosition(int entId) {
	Entity* ent = GlobalScene->entities.GetById(entId);
	Transform* trans = GlobalScene->transforms.GetById(ent->transform);
	return trans->position;
}

/*[ScriptFunc]*/
float GetDeltaTime() {
	return (float)GlobalScene->GetDeltaTime();
}


