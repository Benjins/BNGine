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
"#version 120\n"
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

	if (texId.id != 0xFFFFFFFF) {
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
		prog->vertShader = IDHandle<Shader>(guiTextVShader->id);
		prog->fragShader = IDHandle<Shader>(guiTextFShader->id);
		prog->CompileProgram();

		Material* guiTextMat = GlobalScene->res.materials.CreateAndAdd();
		guiTextMat->programId = IDHandle<Program>(prog->id);

		guiTextMat->SetIntUniform("_mainTex", 0);

		guiTextMatId = IDHandle<Material>(guiTextMat->id);
	}

	{
		Shader* guiColVShader = GlobalScene->res.shaders.CreateAndAdd();
		guiColVShader->CompileShader(guiColVShaderText, GL_VERTEX_SHADER);

		Shader* guiColFShader = GlobalScene->res.shaders.CreateAndAdd();
		guiColFShader->CompileShader(guiColFShaderText, GL_FRAGMENT_SHADER);

		Program* prog = GlobalScene->res.programs.CreateAndAdd();
		prog->vertShader = IDHandle<Shader>(guiColVShader->id);
		prog->fragShader = IDHandle<Shader>(guiColFShader->id);
		prog->CompileProgram();

		Material* guiColMat = GlobalScene->res.materials.CreateAndAdd();
		guiColMat->programId = IDHandle<Program>(prog->id);

		guiColMatId = IDHandle<Material>(guiColMat->id);
	}
}

void GuiSystem::ShutDown(){
	
	{
		Material* mat = GlobalScene->res.materials.GetById(guiTextMatId);
		Program* prog = GlobalScene->res.programs.GetById(mat->programId);
		
		glDeleteProgram(prog->programObj);
		
		Shader* vs = GlobalScene->res.shaders.GetById(prog->vertShader);
		Shader* fs = GlobalScene->res.shaders.GetById(prog->fragShader);
		
		glDeleteShader(vs->shaderObj);
		glDeleteShader(fs->shaderObj);
	}
	
	{
		Material* mat = GlobalScene->res.materials.GetById(guiColMatId);
		Program* prog = GlobalScene->res.programs.GetById(mat->programId);
		
		glDeleteProgram(prog->programObj);
		
		Shader* vs = GlobalScene->res.shaders.GetById(prog->vertShader);
		Shader* fs = GlobalScene->res.shaders.GetById(prog->fragShader);
		
		glDeleteShader(vs->shaderObj);
		glDeleteShader(fs->shaderObj);
	}
}

// TODO: Text wrapping? Fit to width?
Vector2 GetContentNaturalSize(const GuiContent& content) {
	switch (content.type) {

	case GCT_None: {
		// Do nothing
		// TODO: Should we assert here?
		return Vector2();
	} break;

	case GCT_Ascii: {
		BitmapFont* font = GlobalScene->res.fonts.GetById(content.bmpFontId);
		float width = font->GetCursorPos(content.asciiStr.string, StrLen(content.asciiStr.string));
		float height = content.textScale;

		return Vector2(width, height);
	} break;

	case GCT_Unicode: {
		UniFont* font = GlobalScene->res.uniFonts.GetById(content.uniFontId);
		float width = font->GetCursorPos(content.unicodeStr, content.unicodeStr.length);
		float height = content.textScale;

		return Vector2(width, height);
	} break;

	case GCT_Texture: {
		// TODO: Get the width/height of texture
		return Vector2();
	} break;

	case GCT_Color: {
		// TODO: This doesn't really apply here?
		return Vector2();
	} break;

	case GCT_Count: {
		ASSERT_WARN("Calling '%s' on GuiContent with GCT_Count type.", __FUNCTION__);
		return Vector2();
	} break;

	}

	ASSERT_WARN("Calling '%s' on GuiContent with GCT_Count type.", __FUNCTION__);
	return Vector2();
}

void GuiSystem::DrawContent(const GuiContent& content, GuiRect rect) {
	switch (content.type) {

	case GCT_None: {
		// Draw nothing
		// TODO: Should we assert here?
	} break;

	case GCT_Ascii: {
		DrawTextLabel(content.asciiStr.string, content.bmpFontId, content.textScale, rect.x, rect.y, rect.width, rect.height);
	} break;

	case GCT_Unicode: {
		DrawUnicodeLabel(content.unicodeStr, content.uniFontId, content.textScale, rect.x, rect.y, rect.width, rect.height);
	} break;

	case GCT_Texture: {
		// TODO:
	} break;

	case GCT_Color: {
		ColoredBox(rect.x, rect.y, rect.width, rect.height, content.color);
	} break;

	case GCT_Count: {
		ASSERT_WARN("%s", "Trying to draw GuiContent with GCT_Count type.");
	} break;

	}
}

float GuiSystem::DrawTextLabel(const char* text, IDHandle<BitmapFont> fontId, float scale, float x, float y, float w /*= 10000*/, float h /*= 10000*/){
	BitmapFont* font = GlobalScene->res.fonts.GetById(fontId);
	
	int textLen = StrLen(text);

	if (textLen == 0) {
		return 0;
	}

	// TODO: Avoid this malloc
	float* posBuffer = (float*)malloc(textLen*12*sizeof(float));
	float* uvsBuffer = (float*)malloc(textLen*12*sizeof(float));

	int charsWritten = -1;
	float width = font->BakeAsciiToVertexData(text, x, y, w, h, posBuffer, uvsBuffer, &charsWritten);

	int dcIndex = -1;
	for (int i = 0; i < guiDrawCalls.count; i++) {
		if (guiDrawCalls.Get(i).matId == guiTextMatId && guiDrawCalls.Get(i).texId == font->textureId) {
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

float GuiSystem::DrawUnicodeLabel(U32String text, IDHandle<UniFont> fontId, float scale, float x, float y, float w /*= 10000*/, float h /*= 10000*/) {
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
		if (guiDrawCalls.Get(i).matId == guiTextMatId && guiDrawCalls.Get(i).texId == font->textureId) {
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

	Vector2 pos[4] = { { x, y },{ x + w, y },{ x, y + h },{ x + w, y + h } };

	glBegin(GL_TRIANGLE_STRIP);
	for (int i = 0; i < 4; i++) {
		glVertex2f(pos[i].x / GlobalScene->cam.widthPixels * 2 - 1, pos[i].y / GlobalScene->cam.heightPixels * 2 - 1);
	}

	glEnd();
}

// TODO: make this an IDHandle<BitmapFont> ?
String GuiSystem::TextInput(const String& textIn, uint32 fontId, float scale, float x, float y, float w) {
	ColoredBox(x, y, w, scale, Vector4(0.4f, 0.4f, 0.4f, 0.85f));

	float textOffset = 0.0f;
	const char* textRenderStart = textInputState.prevEntry.string;
	if (textInputState.count == textInputState.activeIndex) {
		textRenderStart = &textInputState.prevEntry.string[textInputState.textOffset];
		BitmapFont* font = GlobalScene->res.fonts.GetByIdNum(fontId);
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

	DrawTextLabel(textToDraw, IDHandle<BitmapFont>(fontId), scale, x + textOffset, y, w, scale);

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

		BitmapFont* font = GlobalScene->res.fonts.GetByIdNum(fontId);
		float cursorOffset = font->GetCursorPos(textRenderStart, textInputState.cursorPos - textInputState.textOffset);
		float curX = x + cursorOffset + textOffset;

		ColoredBox(curX, y, cursorWidth, scale, Vector4(0.7f, 0.7f, 0.7f, 0.7f));

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
	cachedRects.EnsureCapacity(rects.currentCount);
	cachedRects.count = rects.currentCount;

	for (int i = 0; i < rects.currentCount; i++) {
		if (rects.vals[i].isDirty) {
			cachedRects.data[i] = rects.vals[i].GetFinalRect();
			rects.vals[i].isDirty = false;
		}
	}

	for (int i = 0; i < buttons.currentCount; i++) {
		Vector2 natSize = GetContentNaturalSize(buttons.vals[i].content);

		GuiRect* rect = rects.GetById(buttons.vals[i].rect);
		int rectIdx = rect - rects.vals;

		GuiRect finalRect = cachedRects.data[rectIdx];
		Vector2 centeredPos = finalRect.position + (finalRect.size - natSize) / 2;

		GuiRect contentRect = {};
		contentRect.position = centeredPos;
		contentRect.size = natSize;

		DrawContent(buttons.vals[i].content, contentRect);

		GuiContent colorBox;
		colorBox.type = GCT_Color;
		colorBox.color = Vector4(0.7f, 0.7f, 0.7f, (buttons.vals[i].state == GBS_Down ? 0.4f : 0.3f));
		DrawContent(colorBox, finalRect);
	}

	for (int i = 0; i < checkboxes.currentCount; i++) {
		GuiRect* rect = rects.GetById(checkboxes.vals[i].rect);
		int rectIdx = rect - rects.vals;

		GuiRect finalRect = cachedRects.data[rectIdx];

		GuiContent colorBox;
		colorBox.type = GCT_Color;
		colorBox.color = Vector4(0.7f, 0.7f, 0.7f, 0.8f);
		DrawContent(colorBox, finalRect);

		if (checkboxes.vals[i].isChecked){
			finalRect.position = finalRect.position + finalRect.size / 8;
			finalRect.size = finalRect.size * 0.75f;
			colorBox.color = Vector4(0.5f, 0.5f, 0.5f, 0.9f);
			DrawContent(colorBox, finalRect);
		}
	}

	for (int i = 0; i < stringPickers.currentCount; i++) {
		GuiStringPicker* picker = &stringPickers.vals[i];
		GuiRect* rect = rects.GetById(picker->rect);
		int rectIdx = rect - rects.vals;

		GuiRect finalRect = cachedRects.data[rectIdx];

		if (picker->options == GSPO_SingleChoice) {
			picker->choice = StringPicker(
				(const char**)picker->choices.data, 
				picker->choices.count, 0, 12, 
				finalRect.x, finalRect.y + finalRect.height, 
				finalRect.width, finalRect.height);
		}
		else {
			ColoredBox(finalRect.x, finalRect.y, finalRect.width, finalRect.height, Vector4(0.7f, 0.7f, 0.7f, 0.7f));

			bool changed = false;
			float scale = 12;
			float currY = finalRect.y + finalRect.height - scale / 2 + 1;
			for (int j = 0; j < picker->choices.count; j++) {
				Vector4 backCol = (picker->choice & (1 << j)) ? Vector4(0.5f, 0.8f, 0.8f, 0.5f) : Vector4(0.3f, 0.3f, 0.3f, 0.3f);
				Vector4 hoverCol = (picker->choice & (1 << j)) ? Vector4(0.5f, 0.7f, 0.7f, 0.4f) : Vector4(0.4f, 0.4f, 0.4f, 0.4f);
				if (j == picker->clearIndex || j == picker->allIndex) {
					backCol = Vector4(0.3f, 0.3f, 0.3f, 0.3f);
					hoverCol = Vector4(0.4f, 0.4f, 0.4f, 0.4f);
				}

				if (TextButton(picker->choices.data[j].string, 0, scale, finalRect.x + 2, currY, finalRect.width - 4, scale + 2, backCol, hoverCol)) {
					if (j == picker->clearIndex) {
						changed = (picker->choice != 0);
						picker->choice = 0;
					}
					else if (j == picker->allIndex) {
						changed = (picker->choice != -1);
						picker->choice = -1;
					}
					else {
						picker->choice ^= (j << i);
						changed = true;
					}
				}
				currY -= (scale + 4);
			}

			if (changed) {
				ExecuteAction(picker->onSelect);
			}
		}
	}

	for (int i = 0; i < guiFormStack.currentCount; i++) {
		DoGuiFormData(this, &guiFormStack.vals[i], GlobalScene->cam.widthPixels, GlobalScene->cam.heightPixels);
	}

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

float GuiSystem::SimpleSlider(float val, float x, float y, float width, float height) {
	float trueVal = BNS_MIN(1, BNS_MAX(0, val));

	const float sliderWidth = 15;
	float trueWidth = width - sliderWidth;
	ASSERT(trueWidth > 0);

	float sliderMiddleX = trueWidth * trueVal;
	float sliderBegin = x + sliderMiddleX - sliderWidth / 2;

	ColoredBox(x, y, width, height, Vector4(0.8f, 0.8f, 0.8f, 0.2f));
	ColoredBox(sliderBegin, y, sliderWidth, height, Vector4(0.8f, 0.8f, 0.8f, 0.8f));

	float mouseX = GlobalScene->input.cursorX;
	float mouseY = GlobalScene->cam.heightPixels - GlobalScene->input.cursorY;

	if (GlobalScene->input.MouseButtonIsDown(MouseButton::PRIMARY)) {
		if (mouseX >= sliderBegin && mouseX < sliderBegin + sliderWidth
		 && mouseY >= y && mouseY <= y + height) {
			float newSliderMiddleX = sliderMiddleX + GlobalScene->input.cursorDeltaX;
			float newVal = newSliderMiddleX / trueWidth;
			float newTrueVal = BNS_MIN(1, BNS_MAX(0, newVal));

			return newTrueVal;
		}
		// In case user moves mouse really fast, try to compensate
		// by checking if we were over slider last frame
		// TODO: Allow mouse to go outside slider, but still affect it as long as button stays down
		else {
			mouseX -= GlobalScene->input.cursorDeltaX;
			mouseY -= GlobalScene->input.cursorDeltaY;
			if (mouseX >= sliderBegin && mouseX < sliderBegin + sliderWidth
			 && mouseY >= y && mouseY <= y + height) {
				float newSliderMiddleX = sliderMiddleX + GlobalScene->input.cursorDeltaX;
				float newVal = newSliderMiddleX / trueWidth;
				float newTrueVal = BNS_MIN(1, BNS_MAX(0, newVal));

				return newTrueVal;
			}
		}
	}

	return trueVal;
}

bool GuiSystem::TextButton(const char* text, uint32 fontId, float scale, float x, float y, float w, float h,
	Vector4 backCol /*= Vector4(0.4f, 0.4f, 0.4f, 0.4f)*/, 
	Vector4 hoverCol /*= Vector4(0.4f, 0.4f, 0.4f, 0.4f)*/,
	Vector4 pressedCol /*= Vector4(0.4f, 0.4f, 0.4f, 0.4f)*/) {
	float cursorX = GlobalScene->input.cursorX;
	float cursorY = GlobalScene->cam.heightPixels - GlobalScene->input.cursorY;
	bool isCursorIn =  cursorX >= x && cursorX <= x + w
					&& cursorY >= y - h/2 && cursorY <= y + h/2;

	Vector4 col = backCol;
	if (isCursorIn) {
		if (GlobalScene->input.MouseButtonIsDown(PRIMARY)) {
			col = pressedCol;
		}
		else {
			col = hoverCol;
		}
	}

	ColoredBox(x, y - h/2, w, h, col);

	BitmapFont* font = GlobalScene->res.fonts.GetByIdNum(fontId);
	float textWidth = font->GetCursorPos(text, StrLen(text));
	DrawTextLabel(text, IDHandle<BitmapFont>(fontId), scale, x + w/2 - textWidth/2, y - scale/2);

	return isCursorIn && GlobalScene->input.MouseButtonIsReleased(PRIMARY);
}

int GuiSystem::StringPicker(const char** stringArr, int count, uint32 fontId, float scale, float x, float y, float w, float h) {
	ColoredBox(x, y - h, w, h, Vector4(0.7f, 0.7f, 0.7f, 0.7f));

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
	Vector2 mousePos = Vector2(GlobalScene->input.cursorX, GlobalScene->cam.heightPixels - GlobalScene->input.cursorY);
	bool mouseDown = GlobalScene->input.MouseButtonIsDown(MouseButton::PRIMARY);
	bool mouseReleased = GlobalScene->input.MouseButtonIsReleased(MouseButton::PRIMARY);
	bool mousePressed = GlobalScene->input.MouseButtonIsPressed(MouseButton::PRIMARY);
	for (int i = 0; i < buttons.currentCount; i++) {
		GuiRect* buttonRect = rects.GetById(buttons.vals[i].rect);
		GuiRect finalRect = cachedRects.data[buttonRect - rects.vals];

		bool insideButton = finalRect.ContainsPoint(mousePos);

		switch (buttons.vals[i].state) {
		case GBS_Off: {
			if (mouseDown && insideButton) {
				buttons.vals[i].state = GBS_Down;
			}
		} break;

		case GBS_Down: {
			if (!mouseDown) {
				if (insideButton) {
					ExecuteAction(buttons.vals[i].onClick);
				}

				buttons.vals[i].state = GBS_Off;
			}
		} break;
		}
	}

	for (int i = 0; i < checkboxes.currentCount; i++) {
		GuiRect* checkboxRect = rects.GetById(checkboxes.vals[i].rect);
		GuiRect finalRect = cachedRects.data[checkboxRect - rects.vals];
		bool insideCheckbox = finalRect.ContainsPoint(mousePos);

		if (insideCheckbox && mouseReleased) {
			checkboxes.vals[i].isChecked = !checkboxes.vals[i].isChecked;
			ExecuteAction(checkboxes.vals[i].onChange);
		}
	}

	for (int i = 0; i < stringPickers.currentCount; i++) {
		GuiStringPicker* picker = &stringPickers.vals[i];
		GuiRect* rect = rects.GetById(picker->rect);
		int rectIdx = rect - rects.vals;

		GuiRect finalRect = cachedRects.data[rectIdx];

		bool insidePicker = finalRect.ContainsPoint(mousePos);
		if (picker->witnessedMouseDown && !insidePicker && mouseReleased) {
			stringPickers.RemoveByIdNum(picker->id);
		}
		else if (!insidePicker && mousePressed) {
			picker->witnessedMouseDown = true;
		}
		else if (picker->choice >= 0 && picker->options == GSPO_SingleChoice) {
			ExecuteAction(picker->onSelect);
			stringPickers.RemoveByIdNum(picker->id);
		}
	}

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

