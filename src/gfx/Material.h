#ifndef __MATERIAL_H
#define __MATERIAL_H

#pragma once

#include "../../ext/CppUtils/stringmap.h"
#include "../../ext/CppUtils/idbase.h"

#include "../../ext/3dbasics/Mat4.h"

#include "GLWrap.h"

#define MAX_TEXTURES_PER_MATERIAL 14

struct Material : IDBase {
	char name[64];

	int texIds[MAX_TEXTURES_PER_MATERIAL];
	uint32 programId;
	int texCount;
	
	StringMap<GLint> uniformCache;

	Material() : IDBase() {
		texCount = 0;
	}

	void AddTexture(int texId) {
		texIds[texCount] = texId;
		texCount++;
	}

	GLint GetUniformLocation(const char* name);

	void SetFloatUniform(const char* name, float val);
	void SetIntUniform(const char* name, int val);
	void SeMatrix4Uniform(const char* name, Mat4x4 val);
};


#endif
