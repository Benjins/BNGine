#ifndef SCRIPTINSTANCE_H
#define SCRIPTINSTANCE_H

#pragma once

#include "ScriptObject.h"

#include "../core/Component.h"

struct ScriptInstance : Component {
	BNVMInstance inst;

	/*[SerializeFromId("src", "bnv")]*/
	IDHandle<ScriptObject> vmId;

	void OnLoad();

	void UpdateScript();
};

#endif
