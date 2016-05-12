#include "GuiSystem.h"

#include "../core/Scene.h"

#include "../gfx/Shader.h"
#include "../gfx/Program.h"
#include "../gfx/Material.h"
#include "../gfx/GLExtInit.h"

#include "../../ext/CppUtils/strings.h"

const char* guiTextVShaderText = 
"#version 120\n"
"attribute vec2 pos;\n"
"attribute vec2 uv;\n"
"\n"
"varying vec2 outUv;\n"
"\n"
"void main(){\n"
"	vec4 outPos = vec4(pos, 0, 1);\n"
"	outUv = uv;\n"
"	gl_Position = outPos;\n"
"}\n";

const char* guiTextFShaderText =
"varying vec2 outUv;\n"
"uniform sampler2D _mainTex;\n"
"\n"
"void main(){\n"
"	float sample = texture2D(_mainTex, outUv).r;\n"
"	gl_FragColor = vec4(sample,sample,sample,sample);\n"
"}\n";

void GuiSystem::Init(){
	Shader* guiTextVShader = GlobalScene->res.shaders.CreateAndAdd();
	guiTextVShader->CompileShader(guiTextVShaderText, GL_VERTEX_SHADER);
	
	Shader* guiTextFShader = GlobalScene->res.shaders.CreateAndAdd();
	guiTextFShader->CompileShader(guiTextFShaderText, GL_FRAGMENT_SHADER);
	
	Program* prog = GlobalScene->res.programs.CreateAndAdd();
	prog->vertShader = guiTextVShader->id;
	prog->fragShader = guiTextFShader->id;
	prog->CompileProgram();

	Material* guiTextMat = GlobalScene->res.materials.CreateAndAdd();
	guiTextMat->programId = prog->id;
	
	guiTextMat->SetIntUniform("_mainTex", 0);
	
	guiTextMatId = guiTextMat->id;
}

void GuiSystem::DrawTextLabel(const char* text, uint32 fontId, float scale, float x, float y){
	BitmapFont* font = GlobalScene->res.fonts.GetById(fontId);
	
	int textLen = StrLen(text);
	
	float* posBuffer = (float*)malloc(textLen*12*sizeof(float));
	float* uvsBuffer = (float*)malloc(textLen*12*sizeof(float));
	
	font->BakeAsciiToVertexData(text, x, y, posBuffer, uvsBuffer);

	Texture* tex = GlobalScene->res.textures.GetById(font->textureId);
	tex->Bind(GL_TEXTURE0);

	Material* mat = GlobalScene->res.materials.GetById(guiTextMatId);
	Program* prog = GlobalScene->res.programs.GetById(mat->programId);
	glUseProgram(prog->programObj);
	mat->UpdateUniforms();

	GLuint posVbo;
	glGenBuffers(1, &posVbo);
	glBindBuffer(GL_ARRAY_BUFFER, posVbo);
	glBufferData(GL_ARRAY_BUFFER, textLen*12*sizeof(float), posBuffer, GL_DYNAMIC_DRAW);
	
	GLuint uvVbo;
	glGenBuffers(1, &uvVbo);
	glBindBuffer(GL_ARRAY_BUFFER, uvVbo);
	glBufferData(GL_ARRAY_BUFFER, textLen*12 * sizeof(float), uvsBuffer, GL_DYNAMIC_DRAW);
	
	GLint posAttribLoc = glGetAttribLocation(prog->programObj, "pos");
	glEnableVertexAttribArray(posAttribLoc);
	glBindBuffer(GL_ARRAY_BUFFER, posVbo);
	glVertexAttribPointer(posAttribLoc, 2, GL_FLOAT, GL_FALSE, 0, 0);

	GLint uvAttribLoc = glGetAttribLocation(prog->programObj, "uv");
	glEnableVertexAttribArray(uvAttribLoc);
	glBindBuffer(GL_ARRAY_BUFFER, uvVbo);
	glVertexAttribPointer(uvAttribLoc, 2, GL_FLOAT, GL_FALSE, 0, 0);

	glDrawArrays(GL_TRIANGLES, 0, textLen*6);

	glDisableVertexAttribArray(posAttribLoc);
	glDisableVertexAttribArray(uvAttribLoc);
	
	glDeleteBuffers(1, &posVbo);
	glDeleteBuffers(1, &uvVbo);
}



