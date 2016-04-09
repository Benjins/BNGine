#include "Program.h"
#include "Shader.h"

#include "GLExtInit.h"
#include "../core/Scene.h"

void Program::CompileProgram() {
	programObj = glCreateProgram();

	Shader* vertProg = GlobalScene->gfx.shaders.GetById(vertShader);
	Shader* fragProg = GlobalScene->gfx.shaders.GetById(fragShader);

	ASSERT(vertProg->shaderType == GL_VERTEX_SHADER);
	ASSERT(fragProg->shaderType == GL_FRAGMENT_SHADER);

	glAttachShader(programObj, vertProg->shaderObj);
	glAttachShader(programObj, fragProg->shaderObj);

	glLinkProgram(programObj);

	GLint success;
	glGetProgramiv(programObj, GL_LINK_STATUS, &success);

	if (success == 0) {
		GLchar errorLog[1024];
		glGetProgramInfoLog(programObj, sizeof(errorLog), NULL, errorLog);
		printf("Error linking shader program: '%s'\n", errorLog);
	}

	glValidateProgram(programObj);
}