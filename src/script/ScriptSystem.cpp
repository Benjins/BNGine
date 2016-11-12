#include "ScriptSystem.h"

#include "../core/Scene.h"

void ScriptSystem::Start() {
	for (int i = 0; i < scriptInstances.currentCount; i++) {
		scriptInstances.vals[i].OnLoad();
	}
}

void ScriptSystem::Update() {
	for (int i = 0; i < scriptInstances.currentCount; i++) {
		ScriptInstance* inst = &scriptInstances.vals[i];
		inst->UpdateScript();
	}
}

