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

struct BitmapFont : IDBase{
	int textureId;
	
	Vector<CodepointInfo> codepointListing;
	
	CodepointInfo* GetInfoForCodepoint(int codepoint);
	
	void BakeAsciiToVertexData(const char* text, float xStart, float yStart, float* outPosData, float* outUvData);
};


#endif
