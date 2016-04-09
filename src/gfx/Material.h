#ifndef __MATERIAL_H
#define __MATERIAL_H

#pragma once

#include "../../ext/CppUtils/stringmap.h"
#include "../../ext/CppUtils/idbase.h"

#include "../../ext/3dbasics/Mat4.h"

#include "GLWrap.h"

struct Material : IDBase {
	char name[64];
	uint32 programId;

	StringMap<GLint> uniformCache;

	GLint GetUniformLocation(const char* name);

	void SetFloatUniform(const char* name, float val);
	void SeMatrix4Uniform(const char* name, Mat4x4 val);
};


#endif
