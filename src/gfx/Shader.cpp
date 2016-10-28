#include "Shader.h"
#include "../../ext/CppUtils/strings.h"

#include "GLExtInit.h"

void Shader::CompileShader(const char* shaderText, GLenum _shaderType){
	shaderObj = glCreateShader(_shaderType);
	shaderType = _shaderType;

	const GLchar* progTexts[1];
	progTexts[0] = shaderText;
	GLint lengths[1];
	lengths[0] = StrLen(shaderText);

	glShaderSource(shaderObj, 1, progTexts, lengths);
	glCompileShader(shaderObj);

	GLint success;
	glGetShaderiv(shaderObj, GL_COMPILE_STATUS, &success);
	if (!success) {
		GLchar infoLog[1024];
		glGetShaderInfoLog(shaderObj, sizeof(infoLog), NULL, infoLog);
		ASSERT_WARN("Error compiling shader type %d: '%s'\n", _shaderType, infoLog);
	}
}
