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

float GuiSystem::DrawTextLabel(const char* text, uint32 fontId, float scale, float x, float y){
	BitmapFont* font = GlobalScene->res.fonts.GetById(fontId);
	
	int textLen = StrLen(text);

	if (textLen == 0) {
		return 0;
	}

	float* posBuffer = (float*)malloc(textLen*12*sizeof(float));
	float* uvsBuffer = (float*)malloc(textLen*12*sizeof(float));

	float width = font->BakeAsciiToVertexData(text, x, y, posBuffer, uvsBuffer);

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

	return width;
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

	float textOffset = 0.0f;

	if (textInputState.count == textInputState.activeIndex) {
		BitmapFont* font = GlobalScene->res.fonts.GetById(fontId);
		float cursorX = font->GetCursorPos(textInputState.prevEntry.string, textInputState.cursorPos);

		if (cursorX > w) {
			// Will be negative
			textOffset = (w - cursorX);
		}
	}

	glScissor((int)x, (int)y, (int)w, (int)scale);
	if (textInputState.count == textInputState.activeIndex) {
		DrawTextLabel(textInputState.prevEntry.string, fontId, scale, x + textOffset, y);
	}
	else {
		DrawTextLabel(textIn.string, fontId, scale, x + textOffset, y);
	}
	glScissor((int)GlobalScene->cam.xOffset, (int)GlobalScene->cam.yOffset, (int)GlobalScene->cam.widthPixels, (int)GlobalScene->cam.heightPixels);

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

				String toRet = textInputState.prevEntry;
				textInputState.prevEntry.SetSize(0);
				return toRet;
			}
		}
		else {
			if (cursorIn) {
				textInputState.activeIndex = textInputState.count - 1;
				textInputState.prevEntry = textIn;
			}
		}
	}

	if (textInputState.count == textInputState.activeIndex + 1) {
		mat->SetVector4Uniform("_col", Vector4(0.7f, 0.7f, 0.7f, 0.7f));
		glUseProgram(prog->programObj);
		mat->UpdateUniforms();
		BitmapFont* font = GlobalScene->res.fonts.GetById(fontId);
		float cursorOffset = font->GetCursorPos(textInputState.prevEntry.string, textInputState.cursorPos);

		static const int cursorWidth = 6;
		float curX = x + cursorOffset + textOffset;

		Vector2 cursorPos[4] = { { curX, y },{ curX + cursorWidth, y },{ curX, y + scale },{ curX + cursorWidth, y + scale } };

		glBegin(GL_TRIANGLE_STRIP);
		for (int i = 0; i < 4; i++) {
			glVertex2f(cursorPos[i].x / GlobalScene->cam.widthPixels * 2 - 1, cursorPos[i].y / GlobalScene->cam.heightPixels * 2 - 1);
		}

		glEnd();

		for (unsigned char c = 'A'; c <= 'Z'; c++) {
			if (GlobalScene->input.KeyIsPressed(c)) {
				if (!GlobalScene->input.KeyIsDown(KC_Shift)) {
					c += 32;
				}

				textInputState.prevEntry = textInputState.prevEntry.Insert(c, textInputState.cursorPos);
				textInputState.cursorPos++;
				return textIn;
			}
		}

		static const char* shiftString = ")!@#$%^&*(";

		for (unsigned char c = '0'; c <= '9'; c++) {
			if (GlobalScene->input.KeyIsPressed(c)) {
				if (GlobalScene->input.KeyIsDown(KC_Shift)) {
					c = shiftString[c - '0'];
				}

				textInputState.prevEntry = textInputState.prevEntry.Insert(c, textInputState.cursorPos);
				textInputState.cursorPos++;
				return textIn;
			}
		}

		if (GlobalScene->input.KeyIsPressed(KC_Space)) {
			textInputState.prevEntry = textInputState.prevEntry.Insert(' ', textInputState.cursorPos);
			textInputState.cursorPos++;
			return textIn;
		}
		if (GlobalScene->input.KeyIsPressed(KC_Period)) {
			String newStr;
			if (GlobalScene->input.KeyIsDown(KC_Shift)) {
				textInputState.prevEntry = textInputState.prevEntry.Insert('>', textInputState.cursorPos);
				textInputState.cursorPos++;
				return textIn;
			}
			else {
				textInputState.prevEntry = textInputState.prevEntry.Insert('.', textInputState.cursorPos);
				textInputState.cursorPos++;
				return textIn;
			}
			
			textInputState.cursorPos++;
			return newStr;
		}

		if (GlobalScene->input.KeyIsPressed(KC_BackSpace) && textInputState.cursorPos > 0) {
			textInputState.prevEntry = textInputState.prevEntry.Remove(textInputState.cursorPos - 1);
			textInputState.cursorPos--;
			return textIn;
		}

		if (GlobalScene->input.KeyIsPressed(KC_RightArrow)) {
			textInputState.cursorPos++;
		}
		if (GlobalScene->input.KeyIsPressed(KC_LeftArrow)) {
			textInputState.cursorPos--;
		}

		if (GlobalScene->input.KeyIsPressed(KC_UpArrow) 
			|| textInputState.cursorPos < 0) {
			textInputState.cursorPos = 0;
		}

		if (GlobalScene->input.KeyIsPressed(KC_DownArrow)
			|| textInputState.cursorPos > textInputState.prevEntry.GetLength()) {
			textInputState.cursorPos = textInputState.prevEntry.GetLength();
		}

		if (GlobalScene->input.KeyIsPressed(KC_Enter)) {
			textInputState.activeIndex = -1;
			textInputState.cursorPos = 0;

			String toRet = textInputState.prevEntry;
			textInputState.prevEntry.SetSize(0);
			return toRet;
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
	if (textInputState.activeIndex >= 0) {
		if (GlobalScene->input.KeyIsPressed(KC_Tab)) {
			textInputState.cursorPos = 0;
			if (GlobalScene->input.KeyIsDown(KC_Shift)) {
				textInputState.activeIndex--;
				if (textInputState.activeIndex < 0) {
					textInputState.activeIndex += textInputState.count;
				}
			}
			else {
				textInputState.activeIndex++;
				textInputState.activeIndex = textInputState.activeIndex % textInputState.count;
			}
		}
	}

	if (textInputState.count != 0) {
		if (textInputState.activeIndex > 0) {
			textInputState.activeIndex = textInputState.activeIndex % textInputState.count;
		}
		textInputState.count = 0;
	}
}

