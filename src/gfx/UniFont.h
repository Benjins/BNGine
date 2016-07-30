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

	typedef void (UniFont::* CacheGlyphMemberFunc)(int, int, int, Texture*, bool*);
	typedef void (UniFont::* BakeVertexDataMemberFunc)
		(int, float*, float, float, float, Texture*, float*, float*, int*);

	//int codePoint, float x, float y, float width, float height, float* outPosData, float* outUvDat

	struct FontSpecialCase {
		CacheGlyphMemberFunc cacheGlyphMethod;
		BakeVertexDataMemberFunc bakeVertexMethod;
		UnicodeBlockType block;
		int numQuadsPerChar;
	};

	static const FontSpecialCase cacheGlyphSpecialCases[];

	void AddFont(unsigned char* fontBuffer, int bufferSize);

	void CacheGlyphDefault(int codePoint, int cellCols, int cellRows, Texture* tex, bool* outIsDirty);
	void CacheGlyphHangul(int codePoint, int cellCols, int cellRows, Texture* tex, bool* outIsDirty);
	void CacheGlyphs(unsigned int* codePoints, int count);

	void BakeVertexDataDefault(int c, float* x, float y, float width, float height, Texture* fontTexture, float* outPosData, float* outUvData, int* index);
	void BakeVertexDataHangul(int c, float* x, float y, float width, float height, Texture* fontTexture, float* outPosData, float* outUvData, int* index);
	float BakeU32ToVertexData(U32String string, float xStart, float yStart, float width, float height, float* outPosData, float* outUvData, int* outCharsBaked = nullptr);
	
	int GetQuadCountForText(const U32String string);

	float GetCursorPos(const char* text, int cursorPos);

	void CleanUp();
};

#endif
