#include "ScriptInstance.h"
#include "../core/Scene.h"

void ScriptInstance::UpdateScript() {
	inst.vm = &GlobalScene->res.scripts.GetById(vmId)->vm;

	inst.Execute("update");
}