#ifndef GUICONTENT_H
#define GUICONTENT_H

#pragma once

#include "../../ext/CppUtils/unicode.h"
#include "../../ext/CppUtils/strings.h"
#include "../../ext/CppUtils/idbase.h"

#include "../../ext/3dbasics/Vector4.h"

enum GuiContentType {
	GCT_None = -1,
	GCT_Ascii,
	GCT_Unicode,
	GCT_Texture,
	GCT_Color,
	GCT_Count
};

struct GuiContent {
	GuiContentType type;

	union {
		struct {
			union {
				struct {
					String asciiStr;
					uint32 bmpFontId;
				};
				struct {
					U32String unicodeStr;
					uint32 uniFontId;
				};
			};
			float textScale;
		};
		Vector4 color;
		uint32 texId;
	};

	GuiContent() {
		type = GCT_None;
	}

	GuiContent(const GuiContent& orig) {
		SetType(orig.type);

		if (type == GCT_Ascii) {
			asciiStr = orig.asciiStr;
			bmpFontId = orig.bmpFontId;
			textScale = orig.textScale;
		}
		else if (type == GCT_Unicode) {
			unicodeStr = orig.unicodeStr;
			uniFontId = orig.uniFontId;
			textScale = orig.textScale;
		}
		else if (type == GCT_Texture) {
			texId = orig.texId;
		}
		else if (type == GCT_Color) {
			color = orig.color;
		}
	}

	void Create() {
		if (type == GCT_Ascii) {
			new (&asciiStr) String();
		}
	}

	void SetType(GuiContentType newType) {
		Destroy();
		type = newType;
		Create();
	}

	void Destroy() {
		if (type == GCT_Ascii) {
			asciiStr.Release();
		}
	}

	~GuiContent() {
		Destroy();
	}
};


#endif