#ifndef UNIFONT_H
#define UNIFONT_H

#pragma once	

#include "BitmapFont.h"

#include "../../ext/CppUtils/unicode.h"

// Because of unity build, we must take precautions
#if defined(STB_TRUETYPE_IMPLEMENTATION)
#undef STB_TRUETYPE_IMPLEMENTATION
#endif
#include "../../ext/stb/stb_truetype.h"

struct UniFont : IDBase{
	int textureId;

	Vector<CodepointInfo> codepointListing;

	CodepointInfo* GetInfoForCodepoint(int codepoint);

	Vector<stbtt_fontinfo> fontInfos;
	Vector<void*> fontBuffersInMem;

	int cacheCursor;
	int fontScale;

	void AddFont(unsigned char* fontBuffer, int bufferSize);

	void CacheGlyphs(unsigned int* codePoints, int count);

	float BakeU32ToVertexData(U32String string, float xStart, float yStart, float width, float height, float* outPosData, float* outUvData, int* outCharsBaked = nullptr);
	float GetCursorPos(const char* text, int cursorPos);

	void CleanUp();
};

#endif
