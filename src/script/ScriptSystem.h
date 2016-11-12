#ifndef SCRIPTSYSTEM_H
#define SCRIPTSYSTEM_H

#pragma once

#include "ScriptInstance.h"

struct ScriptSystem {
	IDTracker<ScriptInstance> scriptInstances;

	void Start();
	void Update();
};

#endif
