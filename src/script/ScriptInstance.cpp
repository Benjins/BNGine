#include "ScriptInstance.h"
#include "../core/Scene.h"


void ScriptInstance::OnLoad() {
	inst.vm = &GlobalScene->res.scripts.GetById(vmId)->vm;
	inst.vm->InitNewInst(&inst);
	inst.SetGlobalVariableValue<int>("entity", entity.id);

	int dummy;
	if (inst.vm->functionPointers.LookUp("start", &dummy)) {
		inst.Execute("start");
	}
}

void ScriptInstance::UpdateScript() {
	inst.vm = &GlobalScene->res.scripts.GetById(vmId)->vm;

	inst.Execute("update");
}