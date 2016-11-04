#include "Material.h"
#include "Texture.h"
#include "GLExtInit.h"

#include "../core/Scene.h"

#include "../../ext/3dbasics/Vector4.h"

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

		case UT_TEXTURE2D: {
			glUniform1i(uniformLoc, uniformValues.Read<int>());
		} break;

		case UT_MATRIX4: {
			Mat4x4 val = uniformValues.Read<Mat4x4>();
			glUniformMatrix4fv(uniformLoc, 1, GL_TRUE, (float*)val.m);
		} break;

		case UT_VEC2: {
			Vector2 val = uniformValues.Read<Vector2>();
			glUniform2f(uniformLoc, val.x, val.y);
		} break;

		case UT_VEC3: {
			Vector3 val = uniformValues.Read<Vector3>();
			glUniform3fv(uniformLoc, 1, &val.x);
		} break;

		case UT_VEC4: {
			Vector4 val = uniformValues.Read<Vector4>();
			glUniform4fv(uniformLoc, 1, &val.x);
		} break;

		default:
			ASSERT_WARN("Invalid uniform of type %d", (int)type);
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

void Material::SetMatrix4Uniform(const char* name, const Mat4x4& val) {
	GLint loc = GetUniformLocation(name);

	uniformValues.Write<GLint>(loc);
	uniformValues.Write<UniformType>(UT_MATRIX4);
	uniformValues.Write<Mat4x4>(val);
}

void Material::SetVector4Uniform(const char* name, const Vector4& val) {
	GLint loc = GetUniformLocation(name);

	uniformValues.Write<GLint>(loc);
	uniformValues.Write<UniformType>(UT_VEC4);
	uniformValues.Write<Vector4>(val);
}

void Material::SetVector3Uniform(const char* name, const Vector3& val) {
	GLint loc = GetUniformLocation(name);

	uniformValues.Write<GLint>(loc);
	uniformValues.Write<UniformType>(UT_VEC3);
	uniformValues.Write<Vector3>(val);
}

void Material::SetVector2Uniform(const char* name, const Vector2& val) {
	GLint loc = GetUniformLocation(name);

	uniformValues.Write<GLint>(loc);
	uniformValues.Write<UniformType>(UT_VEC2);
	uniformValues.Write<Vector2>(val);
}

void Material::SetTextureUniform(const char* name, const Texture* tex) {
	GLint loc = GetUniformLocation(name);

	uniformValues.Write<GLint>(loc);
	uniformValues.Write<UniformType>(UT_TEXTURE2D);
	uniformValues.Write<GLint>(tex->textureObj);
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
