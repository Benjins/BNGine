#include "GuiSystem.h"

#include "../core/Scene.h"

#include "../gfx/Shader.h"
#include "../gfx/Program.h"
#include "../gfx/Material.h"
#include "../gfx/GLExtInit.h"

#include "../../ext/CppUtils/strings.h"

#include "../../ext/3dbasics/Vector4.h"

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
"	gl_FragColor = vec4(1.0,1.0,1.0,sample);\n"
"}\n";

const char* guiColVShaderText =
"#version 120\n"
"attribute vec2 pos;\n"
"\n"
"void main(){\n"
"	vec4 outPos = vec4(pos, 0, 1);\n"
"	gl_Position = outPos;\n"
"}\n";

const char* guiColFShaderText =
"#version 120\n"
"uniform vec4 _col;\n"
"\n"
"void main(){\n"
"	gl_FragColor = _col;\n"
"}\n";

void GuiSystem::Init(){
	{
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

	{
		Shader* guiColVShader = GlobalScene->res.shaders.CreateAndAdd();
		guiColVShader->CompileShader(guiColVShaderText, GL_VERTEX_SHADER);

		Shader* guiColFShader = GlobalScene->res.shaders.CreateAndAdd();
		guiColFShader->CompileShader(guiColFShaderText, GL_FRAGMENT_SHADER);

		Program* prog = GlobalScene->res.programs.CreateAndAdd();
		prog->vertShader = guiColVShader->id;
		prog->fragShader = guiColFShader->id;
		prog->CompileProgram();

		Material* guiColMat = GlobalScene->res.materials.CreateAndAdd();
		guiColMat->programId = prog->id;

		guiColMatId = guiColMat->id;
	}
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

	free(posBuffer);
	free(uvsBuffer);
}

String GuiSystem::TextInput(const String& textIn, uint32 fontId, float scale, float x, float y, float w) {
	Material* mat = GlobalScene->res.materials.GetById(guiColMatId);
	Program* prog = GlobalScene->res.programs.GetById(mat->programId);
	mat->SetVector4Uniform("_col", Vector4(0.4f, 0.4f, 0.4f, 0.85f));

	glUseProgram(prog->programObj);
	mat->UpdateUniforms();

	Vector2 pos[4] = { {x, y}, {x + w, y}, {x, y + scale}, {x + w, y + scale} };

	glBegin(GL_TRIANGLE_STRIP);
	for (int i = 0; i < 4; i++) {
		glVertex2f(pos[i].x / GlobalScene->cam.widthPixels * 2 - 1, pos[i].y / GlobalScene->cam.heightPixels * 2 - 1);
	}

	glEnd();

	DrawTextLabel(textIn.string, fontId, scale, x, y);

	textInputState.count++;

	float mouseX = GlobalScene->input.cursorX;
	float mouseY = GlobalScene->cam.heightPixels - GlobalScene->input.cursorY;

	bool cursorIn = (mouseX >= x) && (mouseX <= x + w)
				 && (mouseY >= y) && (mouseY <= y + scale);

	if (GlobalScene->input.MouseButtonIsPressed(PRIMARY)) {
		if (textInputState.count == textInputState.activeIndex + 1) {
			if (!cursorIn) {
				textInputState.activeIndex = -1;
				textInputState.cursorPos = 0;
			}
		}
		else {
			if (cursorIn) {
				textInputState.activeIndex = textInputState.count - 1;
			}
		}
	}

	if (textInputState.count == textInputState.activeIndex + 1) {
		mat->SetVector4Uniform("_col", Vector4(0.7f, 0.7f, 0.7f, 0.7f));
		glUseProgram(prog->programObj);
		mat->UpdateUniforms();
		BitmapFont* font = GlobalScene->res.fonts.GetById(fontId);
		float cursorOffset = font->GetCursorPos(textIn.string, textInputState.cursorPos);

		static const int cursorWidth = 6;
		float curX = x + cursorOffset;

		Vector2 cursorPos[4] = { { curX, y },{ curX + cursorWidth, y },{ curX, y + scale },{ curX + cursorWidth, y + scale } };

		glBegin(GL_TRIANGLE_STRIP);
		for (int i = 0; i < 4; i++) {
			glVertex2f(cursorPos[i].x / GlobalScene->cam.widthPixels * 2 - 1, cursorPos[i].y / GlobalScene->cam.heightPixels * 2 - 1);
		}

		glEnd();

		for (unsigned char c = 32; c < 127; c++) {
			if (GlobalScene->input.KeyIsPressed(c)) {
				String newStr = textIn.Insert(c, textInputState.cursorPos);
				textInputState.cursorPos++;
				return newStr;
			}
		}

		if (GlobalScene->input.KeyIsPressed('\b') && textInputState.cursorPos > 1) {
			String newStr = textIn.Remove(textInputState.cursorPos - 1);
			textInputState.cursorPos--;
			return newStr;
		}

		if (GlobalScene->input.KeyIsPressed(0x87)) {
			textInputState.cursorPos++;
		}
		if (GlobalScene->input.KeyIsPressed(0x85)) {
			textInputState.cursorPos++;
		}

		if (GlobalScene->input.KeyIsPressed(0x18) 
			|| textInputState.cursorPos < 0) {
			textInputState.cursorPos = 0;
		}

		if (GlobalScene->input.KeyIsPressed(0x19) 
			|| textInputState.cursorPos > textIn.GetLength()) {
			textInputState.cursorPos = textIn.GetLength();
		}
	}

	/*
	Up: 0x18
	Down: 0x19
	Right: 0x1A
	Left: 0x1B
	*/

	return textIn;
}

bool GuiSystem::SimpleButton(float x, float y, float w, float h) {
	return false;
}

bool GuiSystem::TextButton(float x, float y, float w, float h) {
	return false;
}

void GuiSystem::EndFrame() {
	textInputState.count = 0;
}

