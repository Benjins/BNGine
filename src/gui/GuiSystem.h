#ifndef GUISYSTEM_H
#define GUISYSTEM_H

#pragma once

#include "GuiButton.h"
#include "GuiCheckbox.h"
#include "GuiStringPicker.h"

#include "../../ext/CppUtils/idbase.h"
#include "../../ext/CppUtils/vector.h"
#include "../../ext/CppUtils/memstream.h"
#include "../../ext/CppUtils/strings.h"
#include "../../ext/CppUtils/unicode.h"

#include "../../ext/3dbasics/Mat4.h"

struct GuiDrawCall{
	uint32 matId;
	uint32 texId;
	
	MemStream uvs;
	MemStream pos;
	
	void ExecuteDraw();
};

enum GuiAlignment{
	GA_None = 0,
	GA_VertTop = 1,
	GA_VertCenter = 2,
	GA_VertBottom = 3,
	GA_VertMask = 3,
	
	GA_HorizTop = 4,
	GA_HorizCenter = 8,
	GA_HorizBottom = 12,
	GA_HorizMask = 12
};

struct GuiTextInputState {
	int count;
	int activeIndex;
	int cursorPos;
	int textOffset;
	String prevEntry;
};

struct GuiSystem{
	int guiImgMatId;
	int guiColMatId;
	int guiTextMatId;
	
	GuiTextInputState textInputState;

	Vector<GuiDrawCall> guiDrawCalls;
	
	Vector<Mat4x4> matrixStack;
protected:
	GuiAlignment currAlign;
public:

	GuiAlignment GetAlignment(){return currAlign;}
	void SetAlignment(GuiAlignment alignment);

	IDTracker<GuiRect> rects;
	IDTracker<GuiButton> buttons;
	IDTracker<GuiCheckbox> checkboxes;
	IDTracker<GuiStringPicker> stringPickers;

	Vector<GuiRect> cachedRects;

	GuiButton* AddButton(Vector2 pos, Vector2 size) {
		GuiButton* button = buttons.CreateAndAdd();
		GuiRect* rect = rects.CreateAndAdd();
		rect->position = pos;
		rect->size = size;
		button->rect = rect->id;

		return button;
	}

	GuiCheckbox* AddCheckbox(Vector2 pos, Vector2 size) {
		GuiCheckbox* check = checkboxes.CreateAndAdd();
		GuiRect* rect = rects.CreateAndAdd();
		rect->position = pos;
		rect->size = size;
		check->rect = rect->id;

		return check;
	}

	GuiStringPicker* AddStringPicker(Vector2 pos, Vector2 size) {
		GuiStringPicker* picker = stringPickers.CreateAndAdd();
		GuiRect* rect = rects.CreateAndAdd();
		rect->position = pos;
		rect->size = size;
		picker->rect = rect->id;

		return picker;
	}
	
	GuiSystem(){
		textInputState.activeIndex = -1;
		textInputState.count = 0;
		textInputState.cursorPos = 0;
		textInputState.textOffset = 0;
	}
	
	void Init();
	void ShutDown();
	
	void PushMatrix();
	void LoadIdentity();
	void Translate(float x, float y);
	void PopMatrix();

	void Render();

	void ColoredBox(float x, float y, float w, float h, const Vector4 col);

	void DrawImage(int textureId, float x, float y, float w, float h);
	// Returns width of drawn text
	float DrawTextLabel(const char* text, uint32 fontId, float scale, float x, float y, float w = 10000, float h = 10000);
	
	float DrawUnicodeLabel(U32String text, uint32 fontId, float scale, float x, float y, float w = 10000, float h = 10000);

	void DrawContent(const GuiContent& content, GuiRect rect);

	String TextInput(const String& textIn, uint32 fontId, float scale, float x, float y, float w);
	
	bool SimpleButton(float x, float y, float w, float h);
	bool TextButton(const char* text, uint32 fontId, float scale, float x, float y, float w, float h, 
					Vector4 backCol = Vector4(0.3f, 0.3f, 0.3f, 0.3f),
					Vector4 hoverCol = Vector4(0.4f, 0.4f, 0.4f, 0.4f),
					Vector4 pressedCol = Vector4(0.5f, 0.5f, 0.5f, 0.5f));

	int StringPicker(const char** stringArr, int count, uint32 fontId, float scale, float x, float y, float w, float h);

	void SelectTextInput(int index);
	void DeSelectTextInput();

	void EndFrame();
};

Vector2 GetContentNaturalSize(const GuiContent& content);

#endif
