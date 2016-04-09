#ifndef PROGRAM_H
#define PROGRAM_H

#pragma once

#include "GLWrap.h"

#include "../../ext/CppUtils/idbase.h"

struct Program : IDBase {
	uint32 vertShader;
	uint32 fragShader;

	GLuint programObj;

	void CompileProgram();
};

#endif