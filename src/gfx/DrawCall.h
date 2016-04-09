#ifndef DRAWCALL_H
#define DRAWCALL_H

#pragma once

#include "../../ext/CppUtils/idbase.h"

struct DrawCall : IDBase {
	uint32 meshId;
	uint32 matId;
	uint32 entId;
};

void ExecuteDrawCalls(DrawCall* calls, int count);

#endif
