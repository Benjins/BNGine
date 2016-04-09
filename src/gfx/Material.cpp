#include "Material.h"
#include "../core/Scene.h"
#include "GLExtInit.h"

void Material::SetFloatUniform(const char* name, float val) {
	GLint loc = GetUniformLocation(name);

	glUniform1f(loc, val);
}

void Material::SeMatrix4Uniform(const char* name, Mat4x4 val) {
	GLint loc = GetUniformLocation(name);

	glUniformMatrix4fv(loc, 1, GL_TRUE, (float*)val.m);
}

GLint Material::GetUniformLocation(const char* name) {
	GLint loc;
	if (uniformCache.LookUp(name, &loc)) {
		return loc;
	}
	else {
		Program* prog = GlobalScene->gfx.programs.GetById(programId);
		loc = glGetUniformLocation(prog->programObj, name);
		uniformCache.Insert(name, loc);

		return loc;
	}
	
}