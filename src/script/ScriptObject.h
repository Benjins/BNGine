#ifndef SCRIPTOBJECT_H
#define SCRIPTOBJECT_H

#pragma once

#include "../../ext/CppUtils/bnvparser.h"
#include "../../ext/CppUtils/bnvm.h"
#include "../../ext/CppUtils/idbase.h"

struct ScriptObject : IDBase {
	BNVM vm;
};

#endif
