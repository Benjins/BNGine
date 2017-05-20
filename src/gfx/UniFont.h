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

struct Texture;

struct UniFont : IDBase{
	IDHandle<Texture> textureId;

	Vector<CodepointInfo> codepointListing;

	CodepointInfo* GetInfoForCodepoint(int codepoint);

	Vector<stbtt_fontinfo> fontInfos;
	Vector<void*> fontBuffersInMem;

	int cacheCursor;
	int fontScale;

	typedef float (UniFont::* GetCharWidthMemberFunc)(const U32String, int, int);
	typedef void (UniFont::* CacheGlyphMemberFunc)(const unsigned int*, int, int, int, Texture*, bool*);
	typedef void (UniFont::* BakeVertexDataMemberFunc)
		(const unsigned int*, int, float*, float, float, float, Texture*, float*, float*, int*);

	//int codePoint, float x, float y, float width, float height, float* outPosData, float* outUvDat

	struct FontSpecialCase {
		CacheGlyphMemberFunc cacheGlyphMethod;
		BakeVertexDataMemberFunc bakeVertexMethod;
		GetCharWidthMemberFunc getCharWidthMethod;
		UnicodeBlockType block;
		int numQuadsPerChar;
	};

	static const FontSpecialCase fontFunctionSpecialCases[];

	void AddFont(unsigned char* fontBuffer, int bufferSize);

	void CacheGlyphDefault(const unsigned int* str, int index, int cellCols, int cellRows, Texture* tex, bool* outIsDirty);
	void CacheGlyphHangul(const unsigned int* str, int index, int cellCols, int cellRows, Texture* tex, bool* outIsDirty);
	void CacheGlyphArabic(const unsigned int* str, int index, int cellCols, int cellRows, Texture* tex, bool* outIsDirty);
	void CacheGlyphs(unsigned int* codePoints, int count);

	void BakeVertexDataDefault(const unsigned int* str, int index, float* x, float y, float width, float height, Texture* fontTexture, float* outPosData, float* outUvData, int* outIndex);
	void BakeVertexDataHangul(const unsigned int* str, int index, float* x, float y, float width, float height, Texture* fontTexture, float* outPosData, float* outUvData, int* outIndex);
	void BakeVertexDataArabic(const unsigned int* str, int index, float* x, float y, float width, float height, Texture* fontTexture, float* outPosData, float* outUvData, int* outIndex);
	float BakeU32ToVertexData(U32String string, float xStart, float yStart, float width, float height, float* outPosData, float* outUvData, int* outCharsBaked = nullptr, int* outTriCount = nullptr);

	int GetQuadCountForText(const U32String string);

	float GetCharWidthHangul(const U32String, int index, int c);
	float GetCursorPos(const U32String string, int cursorPos);

	void CleanUp();

	~UniFont();
};

#endif
