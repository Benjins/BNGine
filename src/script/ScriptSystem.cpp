#include "ScriptSystem.h"

void ScriptSystem::Start() {
	for (int i = 0; i < scriptInstances.currentCount; i++) {
		ScriptInstance* inst = &scriptInstances.vals[i];

		inst->inst.vm = &GlobalScene->res.scripts.GetById(inst->vmId)->vm;

		// Huh.
		inst->inst.vm->InitNewInst(&inst->inst);

		inst->inst.SetGlobalVariableValue<int>("entity", inst->entity);
		int dummy;
		if (inst->inst.vm->functionPointers.LookUp("start", &dummy)) {
			inst->inst.Execute("start");
		}
	}
}

void ScriptSystem::Update() {
	for (int i = 0; i < scriptInstances.currentCount; i++) {
		ScriptInstance* inst = &scriptInstances.vals[i];
		inst->UpdateScript();
	}
}

