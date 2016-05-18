#ifndef GUISYSTEM_H
#define GUISYSTEM_H

#pragma once

#include "../../ext/CppUtils/idbase.h"
#include "../../ext/CppUtils/vector.h"
#include "../../ext/CppUtils/memstream.h"
#include "../../ext/CppUtils/strings.h"

#include "../../ext/3dbasics/Mat4.h"

struct GuiDrawCall{
	uint32 matId;
	
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
};

struct GuiSystem{
	int guiImgMatId;
	int guiColMatId;
	int guiTextMatId;
	
	GuiTextInputState textInputState;
	
	Vector<Mat4x4> matrixStack;
protected:
	GuiAlignment currAlign;
public:

	GuiAlignment GetAlignment(){return currAlign;}
	void SetAlignment(GuiAlignment alignment);
	
	GuiSystem(){
		textInputState.activeIndex = -1;
		textInputState.count = 0;
		textInputState.cursorPos = 0;
	}
	
	void Init();
	
	void PushMatrix();
	void LoadIdentity();
	void Translate(float x, float y);
	void PopMatrix();
	
	void DrawImage(int textureId, float x, float y, float w, float h);
	void DrawTextLabel(const char* text, uint32 fontId, float scale, float x, float y);
	void DrawTextWrap(const char* text, float scale, uint32 fontId, float x, float y, float w, float h);
	
	String TextInput(const String& textIn, uint32 fontId, float scale, float x, float y, float w);
	
	bool SimpleButton(float x, float y, float w, float h);
	bool TextButton(float x, float y, float w, float h);

	void EndFrame();
};


//TODO
/*
 - Gui::TextLabel 
 - Gui::TextButton
 - Gui::TextField
 - Gui::CheckBox


*/

#endif
