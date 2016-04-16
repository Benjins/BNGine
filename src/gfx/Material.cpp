#include "Material.h"
#include "../core/Scene.h"
#include "GLExtInit.h"

void Material::UpdateUniforms() {
	while (uniformValues.GetLength() > 0) {
		GLint uniformLoc = uniformValues.Read<GLint>();
		UniformType type = uniformValues.Read<UniformType>();
		switch (type) {
		
		case UT_FLOAT: {
			glUniform1f(uniformLoc, uniformValues.Read<float>());
		} break;

		case UT_INTEGER: {
			glUniform1i(uniformLoc, uniformValues.Read<int>());
		} break;

		case UT_MATRIX4: {
			Mat4x4 val = uniformValues.Read<Mat4x4>();
			glUniformMatrix4fv(uniformLoc, 1, GL_TRUE, (float*)val.m);
		} break;

		default:
			break;
		}
	}

	uniformValues.readHead = uniformValues.base;
	uniformValues.writeHead = uniformValues.base;
}

void Material::SetFloatUniform(const char* name, float val) {
	GLint loc = GetUniformLocation(name);

	uniformValues.Write<GLint>(loc);
	uniformValues.Write<UniformType>(UT_FLOAT);
	uniformValues.Write<float>(val);
}

void Material::SetIntUniform(const char* name, int val) {
	GLint loc = GetUniformLocation(name);

	uniformValues.Write<GLint>(loc);
	uniformValues.Write<UniformType>(UT_INTEGER);
	uniformValues.Write<int>(val);
}

void Material::SeMatrix4Uniform(const char* name, Mat4x4 val) {
	GLint loc = GetUniformLocation(name);

	uniformValues.Write<GLint>(loc);
	uniformValues.Write<UniformType>(UT_MATRIX4);
	uniformValues.Write<Mat4x4>(val);
}

GLint Material::GetUniformLocation(const char* name) {
	GLint loc;
	if (uniformCache.LookUp(name, &loc)) {
		return loc;
	}
	else {
		Program* prog = GlobalScene->res.programs.GetById(programId);
		loc = glGetUniformLocation(prog->programObj, name);
		uniformCache.Insert(name, loc);

		return loc;
	}
	
}