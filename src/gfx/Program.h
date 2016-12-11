#ifndef PROGRAM_H
#define PROGRAM_H

#pragma once

#include "GLWrap.h"

#include "../../ext/CppUtils/idbase.h"

struct Program : IDBase {
	IDHandle<Shader> vertShader;
	IDHandle<Shader> fragShader;

	GLuint programObj;

	void CompileProgram();
};

#endif