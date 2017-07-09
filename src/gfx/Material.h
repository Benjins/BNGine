#ifndef __MATERIAL_H
#define __MATERIAL_H

#pragma once

#include "../../ext/CppUtils/stringmap.h"
#include "../../ext/CppUtils/idbase.h"
#include "../../ext/CppUtils/memstream.h"

#include "../../ext/3dbasics/Mat4.h"

#include "GLWrap.h"

#define MAX_TEXTURES_PER_MATERIAL 14

struct Vector2;
struct Vector4;

struct Texture;
struct Program;
struct CubeMap;

struct Material : IDBase {
	IDHandle<Program> programId;
	IDHandle<Texture> texIds[MAX_TEXTURES_PER_MATERIAL];
	int texCount;

	IDHandle<CubeMap> cubeMap;

	StringMap<GLint> uniformCache;

	MemStream uniformValues;

	Material() : IDBase() {
		texCount = 0;
	}

	void AddTexture(IDHandle<Texture> texId) {
		texIds[texCount] = texId;
		texCount++;
	}

	void UpdateUniforms();

	GLint GetUniformLocation(const char* name);

	void SetFloatUniform(const char* name, float val);
	void SetIntUniform(const char* name, int val);
	void SetMatrix4ArrayUniform(const char* name, const Mat4x4* vals, int count);
	void SetMatrix4Uniform(const char* name, const Mat4x4& val);
	void SetVector4Uniform(const char* name, const Vector4& val);
	void SetVector3Uniform(const char* name, const Vector3& val);
	void SetVector2Uniform(const char* name, const Vector2& val);
	void SetTextureUniform(const char* name, const Texture* tex);
};

enum UniformType{
	UT_UNKNOWN,
	UT_INTEGER,
	UT_FLOAT,
	UT_TEXTURE2D,
	UT_MATRIX4,
	UT_MATRIX4_ARRAY,
	UT_VEC2,
	UT_VEC3,
	UT_VEC4,
	UT_CUBEMAP
};

#endif
