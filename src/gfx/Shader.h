#ifndef SHADER_H
#define SHADER_H

#pragma once

#include "GLWrap.h"

#include "../../ext/CppUtils/idbase.h"

struct Shader : IDBase{
	GLuint shaderObj;
	GLenum shaderType;

	void CompileShader(const char* pShaderText, GLenum _shaderType);
};

#endif