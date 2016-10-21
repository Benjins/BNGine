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

void GuiDrawCall::ExecuteDraw() {
	ASSERT(pos.GetLength() == uvs.GetLength());

	GLuint posVbo;
	glGenBuffers(1, &posVbo);
	glBindBuffer(GL_ARRAY_BUFFER, posVbo);
	glBufferData(GL_ARRAY_BUFFER, pos.GetLength(), (float*)pos.readHead, GL_DYNAMIC_DRAW);

	GLuint uvVbo;
	glGenBuffers(1, &uvVbo);
	glBindBuffer(GL_ARRAY_BUFFER, uvVbo);
	glBufferData(GL_ARRAY_BUFFER, uvs.GetLength(), (float*)uvs.readHead, GL_DYNAMIC_DRAW);

	if ((int)texId != -1) {
		Texture* tex = GlobalScene->res.textures.GetById(texId);
		tex->Bind(GL_TEXTURE0);
	}

	Material* mat = GlobalScene->res.materials.GetById(matId);
	Program* prog = GlobalScene->res.programs.GetById(mat->programId);
	glUseProgram(prog->programObj);
	mat->UpdateUniforms();

	GLint posAttribLoc = glGetAttribLocation(prog->programObj, "pos");
	glEnableVertexAttribArray(posAttribLoc);
	glBindBuffer(GL_ARRAY_BUFFER, posVbo);
	glVertexAttribPointer(posAttribLoc, 2, GL_FLOAT, GL_FALSE, 0, 0);

	GLint uvAttribLoc = glGetAttribLocation(prog->programObj, "uv");
	glEnableVertexAttribArray(uvAttribLoc);
	glBindBuffer(GL_ARRAY_BUFFER, uvVbo);
	glVertexAttribPointer(uvAttribLoc, 2, GL_FLOAT, GL_FALSE, 0, 0);

	
	int indicesCount = (pos.GetLength() / sizeof(float) / 2);
	glDrawArrays(GL_TRIANGLES, 0,  indicesCount);

	glDisableVertexAttribArray(posAttribLoc);
	glDisableVertexAttribArray(uvAttribLoc);

	glDeleteBuffers(1, &posVbo);
	glDeleteBuffers(1, &uvVbo);
}

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

float GuiSystem::DrawTextLabel(const char* text, uint32 fontId, float scale, float x, float y, float w /*= 10000*/, float h /*= 10000*/){
	BitmapFont* font = GlobalScene->res.fonts.GetById(fontId);
	
	int textLen = StrLen(text);

	if (textLen == 0) {
		return 0;
	}

	float* posBuffer = (float*)malloc(textLen*12*sizeof(float));
	float* uvsBuffer = (float*)malloc(textLen*12*sizeof(float));

	int charsWritten = -1;
	float width = font->BakeAsciiToVertexData(text, x, y, w, h, posBuffer, uvsBuffer, &charsWritten);

	int dcIndex = -1;
	for (int i = 0; i < guiDrawCalls.count; i++) {
		if ((int)guiDrawCalls.Get(i).matId == guiTextMatId) {
			dcIndex = i;
			break;
		}
	}

	if (dcIndex == -1) {
		GuiDrawCall newDc;
		newDc.matId = guiTextMatId;
		newDc.texId = font->textureId;
		guiDrawCalls.PushBack(newDc);
		dcIndex = guiDrawCalls.count - 1;
	}

	guiDrawCalls.Get(dcIndex).pos.WriteArray(posBuffer, charsWritten * 12);
	guiDrawCalls.Get(dcIndex).uvs.WriteArray(uvsBuffer, charsWritten * 12);

	free(posBuffer);
	free(uvsBuffer);

	return width;
}

float GuiSystem::DrawUnicodeLabel(U32String text, uint32 fontId, float scale, float x, float y, float w /*= 10000*/, float h /*= 10000*/) {
	UniFont* font = GlobalScene->res.uniFonts.GetById(fontId);
	font->CacheGlyphs(text.start, text.length);

	int textLen = text.length;

	if (textLen == 0) {
		return 0;
	}
	int quadCount = font->GetQuadCountForText(text);
	float* posBuffer = (float*)malloc(quadCount * 12 * sizeof(float));
	float* uvsBuffer = (float*)malloc(quadCount * 12 * sizeof(float));
	
	int charsWritten = -1;
	int triCount = -1;
	float width = font->BakeU32ToVertexData(text, x, y, w, h, posBuffer, uvsBuffer, &charsWritten, &triCount);

	int dcIndex = -1;
	for (int i = 0; i < guiDrawCalls.count; i++) {
		if ((int)guiDrawCalls.Get(i).matId == guiTextMatId && (int)guiDrawCalls.Get(i).texId == font->textureId) {
			dcIndex = i;
			break;
		}
	}

	if (dcIndex == -1) {
		GuiDrawCall newDc;
		newDc.matId = guiTextMatId;
		newDc.texId = font->textureId;
		guiDrawCalls.PushBack(newDc);
		dcIndex = guiDrawCalls.count - 1;
	}

	guiDrawCalls.Get(dcIndex).pos.WriteArray(posBuffer, triCount * 6);
	guiDrawCalls.Get(dcIndex).uvs.WriteArray(uvsBuffer, triCount * 6);

	free(posBuffer);
	free(uvsBuffer);

	return width;
}


void GuiSystem::ColoredBox(float x, float y, float w, float h, const Vector4 col) {
	Material* mat = GlobalScene->res.materials.GetById(guiColMatId);
	Program* prog = GlobalScene->res.programs.GetById(mat->programId);
	mat->SetVector4Uniform("_col", col);

	glUseProgram(prog->programObj);
	mat->UpdateUniforms();

	Vector2 pos[4] = { { x, y },{ x + w, y },{ x, y - h },{ x + w, y - h } };

	glBegin(GL_TRIANGLE_STRIP);
	for (int i = 0; i < 4; i++) {
		glVertex2f(pos[i].x / GlobalScene->cam.widthPixels * 2 - 1, pos[i].y / GlobalScene->cam.heightPixels * 2 - 1);
	}

	glEnd();
}

String GuiSystem::TextInput(const String& textIn, uint32 fontId, float scale, float x, float y, float w) {
	ColoredBox(x, y + scale, w, scale, Vector4(0.4f, 0.4f, 0.4f, 0.85f));

	float textOffset = 0.0f;
	const char* textRenderStart = textInputState.prevEntry.string;
	if (textInputState.count == textInputState.activeIndex) {
		textRenderStart = &textInputState.prevEntry.string[textInputState.textOffset];
		BitmapFont* font = GlobalScene->res.fonts.GetById(fontId);
		float cursorX = font->GetCursorPos(textRenderStart, textInputState.cursorPos - textInputState.textOffset);

		if (cursorX > w) {
			while (cursorX > w && textInputState.textOffset < textInputState.prevEntry.GetLength() - 1) {
				textInputState.textOffset++;
				cursorX = font->GetCursorPos(textRenderStart, textInputState.cursorPos - textInputState.textOffset);
			}
		}
		else if (textInputState.cursorPos < textInputState.textOffset) {
			textInputState.textOffset = textInputState.cursorPos;
		}
	}

	const char* textToDraw = nullptr;
	if (textInputState.count == textInputState.activeIndex) {
		textToDraw = textInputState.prevEntry.string;
	}
	else {
		textToDraw = textIn.string;
	}

	if (textInputState.count == textInputState.activeIndex) {
		textToDraw = &textToDraw[textInputState.textOffset];
	}

	if (textInputState.count == textInputState.activeIndex) {
		DrawTextLabel(textToDraw, fontId, scale, x + textOffset, y, w, scale);
	}
	else {
		DrawTextLabel(textToDraw, fontId, scale, x + textOffset, y, w, scale);
	}

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
		static const int cursorWidth = 6;

		BitmapFont* font = GlobalScene->res.fonts.GetById(fontId);
		float cursorOffset = font->GetCursorPos(textRenderStart, textInputState.cursorPos - textInputState.textOffset);
		float curX = x + cursorOffset + textOffset;

		ColoredBox(curX, y + scale, cursorWidth, scale, Vector4(0.7f, 0.7f, 0.7f, 0.7f));

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

		if (GlobalScene->input.KeyIsPressed(KC_Minus)) {
			String newStr;
			if (GlobalScene->input.KeyIsDown(KC_Shift)) {
				textInputState.prevEntry = textInputState.prevEntry.Insert('_', textInputState.cursorPos);
				textInputState.cursorPos++;
				return textIn;
			}
			else {
				textInputState.prevEntry = textInputState.prevEntry.Insert('-', textInputState.cursorPos);
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

		// @HACK: We handle tab pressing in EndFrame(), but we need to return the
		// value in prevEntry and clear it, since we're being deselected.
		if (GlobalScene->input.KeyIsPressed(KC_Tab)) {
			String toRet = textInputState.prevEntry;
			textInputState.prevEntry.SetSize(0);
			return toRet;
		}

		if (GlobalScene->input.KeyIsPressed(KC_Enter)) {
			textInputState.activeIndex = -1;
			textInputState.cursorPos = 0;

			String toRet = textInputState.prevEntry;
			textInputState.prevEntry.SetSize(0);
			return toRet;
		}
	}

	return textIn;
}

void GuiSystem::Render() {
	for (int i = 0; i < guiDrawCalls.count; i++) {
		guiDrawCalls.Get(i).ExecuteDraw();
	}

	guiDrawCalls.Clear();
}

void GuiSystem::SelectTextInput(int index) {

}

void GuiSystem::DeSelectTextInput() {

}

bool GuiSystem::SimpleButton(float x, float y, float w, float h) {
	return false;
}

bool GuiSystem::TextButton(const char* text, uint32 fontId, float scale, float x, float y, float w, float h) {
	float cursorX = GlobalScene->input.cursorX;
	float cursorY = GlobalScene->cam.heightPixels - GlobalScene->input.cursorY;
	bool isCursorIn =  cursorX >= x && cursorX <= x + w
					&& cursorY >= y - h/2 && cursorY <= y + h/2;

	Vector4 col = Vector4(0.3f, 0.3f, 0.3f, 0.4f);
	if (isCursorIn) {
		if (GlobalScene->input.MouseButtonIsDown(PRIMARY)) {
			col = Vector4(0.5f, 0.5f, 0.5f, 0.4f);
		}
		else {
			col = Vector4(0.4f, 0.4f, 0.4f, 0.4f);
		}
	}

	ColoredBox(x, y + h/2, w, h, col);

	BitmapFont* font = GlobalScene->res.fonts.GetById(fontId);
	float textWidth = font->GetCursorPos(text, StrLen(text));
	DrawTextLabel(text, fontId, scale, x + w/2 - textWidth/2, y - scale/2);

	return isCursorIn && GlobalScene->input.MouseButtonIsReleased(PRIMARY);
}

int GuiSystem::StringPicker(const char** stringArr, int count, uint32 fontId, float scale, float x, float y, float w, float h) {
	ColoredBox(x, y, w, h, Vector4(0.7f, 0.7f, 0.7f, 0.7f));

	int index = -1;
	float currY = y - scale/2 - 1;
	for (int i = 0; i < count; i++) {
		if (TextButton(stringArr[i], fontId, scale, x + 2, currY, w - 4, scale + 2)) {
			index = i;
		}
		currY -= (scale + 4);
	}

	return index;
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

