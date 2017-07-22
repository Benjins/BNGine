#include "ConfigVariable.h"

#include "Scene.h"

_ConfigVarInit::_ConfigVarInit(int* iVal, const char* name) {
	ConfigVariable var;
	var.name.Append(name);
	var.value = iVal;
	globalConfigTable.vars.PushBack(var);
}
_ConfigVarInit::_ConfigVarInit(float* fVal, const char* name) {
	ConfigVariable var;
	var.name.Append(name);
	var.value = fVal;
	globalConfigTable.vars.PushBack(var);
}
_ConfigVarInit::_ConfigVarInit(bool* bVal, const char* name) {
	ConfigVariable var;
	var.name.Append(name);
	var.value = bVal;
	globalConfigTable.vars.PushBack(var);
}
_ConfigVarInit::_ConfigVarInit(String* sVal, const char* name) {
	ConfigVariable var;
	var.name.Append(name);
	var.value = sVal;
	globalConfigTable.vars.PushBack(var);
}
_ConfigVarInit::_ConfigVarInit(Vector2* v2Val, const char* name) {
	ConfigVariable var;
	var.name.Append(name);
	var.value = v2Val;
	globalConfigTable.vars.PushBack(var);
}
_ConfigVarInit::_ConfigVarInit(Vector3* v3Val, const char* name) {
	ConfigVariable var;
	var.name.Append(name);
	var.value = v3Val;
	globalConfigTable.vars.PushBack(var);
}
_ConfigVarInit::_ConfigVarInit(Vector4* cVal, const char* name) {
	ConfigVariable var;
	var.name.Append(name);
	var.value = cVal;
	globalConfigTable.vars.PushBack(var);
}

