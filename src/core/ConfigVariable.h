#ifndef CONFIG_VARIABLE_H
#define CONFIG_VARIABLE_H

#pragma once

#include "../../ext/CppUtils/vector.h"
#include "../../ext/CppUtils/strings.h"
#include "../../ext/CppUtils/disc_union.h"

#include "../../ext/3dbasics/Vector2.h"
#include "../../ext/3dbasics/Vector3.h"
#include "../../ext/3dbasics/Vector4.h"

typedef int* ConfigInt;
typedef float* ConfigFloat;
typedef bool* ConfigBool;
typedef String* ConfigString;
typedef Vector2* ConfigVec2;
typedef Vector3* ConfigVec3;
typedef Vector4* ConfigColor;

#define DISC_MAC(mac)  \
	mac(ConfigInt)     \
	mac(ConfigFloat)   \
	mac(ConfigBool)    \
	mac(ConfigString)  \
	mac(ConfigVec2)    \
	mac(ConfigVec3)    \
	mac(ConfigColor)

DEFINE_DISCRIMINATED_UNION(ConfigValue, DISC_MAC)

#undef DISC_MAC

#define CONFIG_VAR_NAME_MAX_LENGTH 48

struct ConfigVariable {
	StringStackBuffer<CONFIG_VAR_NAME_MAX_LENGTH> name;
	ConfigValue value;
};

struct ConfigVarTable {
	Vector<ConfigVariable> vars;

	bool VarExists(const String& varName) {
		BNS_VEC_FOREACH(vars) {
			if (varName == ptr->name.buffer) {
				return true;
			}
		}

		return false;
	}

	void SetFloat(const String& varName, float val) {
		BNS_VEC_FOREACH(vars) {
			if (varName == ptr->name.buffer) {
				ASSERT(ptr->value.IsConfigFloat());
				*ptr->value.AsConfigFloat() = val;
			}
		}
	}
};

struct _ConfigVarInit {
	_ConfigVarInit(int* iVal, const char* name);
	_ConfigVarInit(float* fVal, const char* name);
	_ConfigVarInit(bool* bVal, const char* name);
	_ConfigVarInit(String* sVal, const char* name);
	_ConfigVarInit(Vector2* v2Val, const char* name);
	_ConfigVarInit(Vector3* v3Val, const char* name);
	_ConfigVarInit(Vector4* cVal, const char* name);
};

#define CONFIG_FLOAT(type, var, name, defaultVal) \
	type var = defaultVal; \
	_ConfigVarInit BNS_GLUE_TOKS(_config_var_, __COUNTER__) (&(var), name)


#endif
