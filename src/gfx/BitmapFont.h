#ifndef BITMAPFONT_H
#define BITMAPFONT_H

#pragma once

#include "../../ext/CppUtils/idbase.h"
#include "../../ext/CppUtils/vector.h"

struct CodepointInfo{
	int codepoint;
	
	float x,y;
	float w,h;
	
	float xOffset,yOffset;
	float xAdvance;
};

struct BitmapFont : IDBase {
	IDHandle<Texture> textureId;

	Vector<CodepointInfo> codepointListing;

	CodepointInfo* GetInfoForCodepoint(int codepoint);

	float BakeAsciiToVertexData(const char* text, float xStart, float yStart, float width, float height, float* outPosData, float* outUvData, int* outCharsBaked = nullptr);

	float GetCursorPos(const char* text, int cursorPos);
};


#endif
