#include "UniFont.h"

#include "Texture.h"

#include "../core/Scene.h"
#include "../assets/AssetFile.h"

const UniFont::FontSpecialCase UniFont::fontFunctionSpecialCases[] = {
	{&UniFont::CacheGlyphHangul, &UniFont::BakeVertexDataHangul, &UniFont::GetCharWidthHangul, UBT_HangulSyllables, 3},
	{&UniFont::CacheGlyphArabic, &UniFont::BakeVertexDataArabic, &UniFont::GetCharWidthHangul, UBT_Arabic, 1}
};

unsigned int arabicUnicodeLookup[] = {
	0xFE8D,
	0xFE8F,
	0xFE93,
	0xFE95,
	0xFE99,
	0xFE9D,
	0xFEA1,
	0xFEA5,
	0xFEA9,
	0xFEAB,
	0xFEAD,
	0xFEAF,
	0xFEB1,
	0xFEB5,
	0xFEB9,
	0xFEBD,
	0xFEC1,
	0xFEC5,
	0xFEC9,
	0xFECD,
	0x0000,
	0x0000,
	0x0000,
	0x0000,
	0x0000,
	0x0000,
	0xFED1,
	0xFED5,
	0xFED9,
	0xFEDD,
	0xFEE1,
	0xFEE5,
	0xFEE9,
	0xFEED,
	0xFEEF,
	0xFEF1,
	0xFE81
};

enum ArabicLetterType {
	ALT_Isolated = 0,
	ALT_End      = 1,
	ALT_Beginning  = 2,
	ALT_Middle   = 3
};

ArabicLetterType DetermineArabicLetterType(const unsigned int* codepoints, int index, int len) {
	bool hasLetterBefore = (index > 0)       && (GetBlockTypeOfCodePoint(codepoints[index - 1]) == UBT_Arabic);
	bool hasLetterAfter  = (index < len - 1) && (GetBlockTypeOfCodePoint(codepoints[index + 1]) == UBT_Arabic);

	if (hasLetterAfter && hasLetterBefore) {
		return ALT_Middle;
	}
	else if (hasLetterAfter) {
		return ALT_Beginning;
	}
	else if (hasLetterBefore) {
		return ALT_End;
	}
	else {
		return ALT_Isolated;
	}
}

int GetArabicRenderCodepoint(const unsigned int* str, int index, int len) {
	int arabicLetterIndex = str[index] - 0x0627;
	ASSERT(arabicLetterIndex >= 0 && arabicLetterIndex < BNS_ARRAY_COUNT(arabicUnicodeLookup));
	ASSERT(arabicUnicodeLookup[arabicLetterIndex] != 0);
	ArabicLetterType type = DetermineArabicLetterType(str, index, len);
	return arabicUnicodeLookup[arabicLetterIndex] + (int)type;
}

CodepointInfo* UniFont::GetInfoForCodepoint(int codepoint) {
	for (int i = 0; i < codepointListing.count; i++) {
		if (codepointListing.data[i].codepoint == codepoint) {
			return &codepointListing.data[i];
		}
	}

	return nullptr;
}

void UniFont::CacheGlyphDefault(const unsigned int* str, int index, int len, int cellCols, int cellRows, Texture* tex, bool* outIsDirty) {
	int codePoint = str[index];
	int charSize = fontScale + 2;
	if (GetInfoForCodepoint(codePoint) == nullptr) {
		stbtt_fontinfo* font = nullptr;
		unsigned char* cBmp = nullptr;
		int cW = 0, cH = 0;
		for (int j = 0; j < fontInfos.count; j++) {
			float pixelScale = stbtt_ScaleForPixelHeight(&fontInfos.data[j], fontScale);
			cBmp = stbtt_GetCodepointBitmap(&fontInfos.data[j], 0, pixelScale, codePoint, &cW, &cH, 0, 0);
			if (cBmp) {
				font = &fontInfos.data[j];
				break;
			}
		}

		if (font != nullptr) {
			*outIsDirty = true;
			int ascent, descent, lineGap;
			stbtt_GetFontVMetrics(font, &ascent, &descent, &lineGap);

			float pixelScale = stbtt_ScaleForPixelHeight(font, fontScale);

			int cellY = cacheCursor / cellCols;
			int cellX = cacheCursor % cellCols;

			int startX = cellX * charSize;
			int startY = cellY * charSize;

			GlyphBlit(tex->texMem, tex->width, tex->height, startX, startY, cBmp, cW, cH);

			free(cBmp);

			int advanceWidth = 0, leftSideBearing = 0;
			stbtt_GetCodepointHMetrics(font, codePoint, &advanceWidth, &leftSideBearing);

			int x0, y0, x1, y1;
			stbtt_GetCodepointBitmapBox(font, codePoint, pixelScale, pixelScale, &x0, &y0, &x1, &y1);
			CodepointInfo pointInfo = {};
			pointInfo.codepoint = codePoint;
			pointInfo.x = startX;
			pointInfo.y = startY;
			pointInfo.w = cW;
			pointInfo.h = cH;
			pointInfo.xOffset = x0;
			pointInfo.yOffset = y0;
			pointInfo.xAdvance = pixelScale * advanceWidth;

			codepointListing.EnsureCapacity(cacheCursor + 1);
			codepointListing.count = BNS_MAX(codepointListing.count, cacheCursor + 1);
			codepointListing.data[cacheCursor] = pointInfo;

			cacheCursor = (cacheCursor + 1) % (cellCols * cellRows);
		}
	}
}

void DecomposeHangulToJamo(int hangul, int* jamos) {
	// codepoint = ((initial * 588) + (medial * 28) + final) + 44032
	int final = (hangul - 44032) % 28;
	int medial = ((hangul - 44032) / 28) % 21;
	int initial = (hangul - 44032) / 588;

	jamos[0] = initial + 0x1100;
	jamos[1] = medial + 0x1161;
	jamos[2] = final + 0x11A7;
}

void UniFont::CacheGlyphHangul(const unsigned int* str, int index, int len, int cellCols, int cellRows, Texture* tex, bool* outIsDirty) {
	int codePoint = str[index];
	int codePoints[3] = {};
	DecomposeHangulToJamo(codePoint, codePoints);

	int count = codePoints[2] == 0x11A7 ? 2 : 3;
	for (int i = 0; i < count; i++) {
		CacheGlyphDefault((unsigned int*)codePoints, i, count, cellCols, cellRows, tex, outIsDirty);
	}
}

void UniFont::CacheGlyphArabic(const unsigned int* str, int index, int len, int cellCols, int cellRows, Texture* tex, bool* outIsDirty) {
	unsigned int finalCodepoint = GetArabicRenderCodepoint(str, index, len);
	CacheGlyphDefault(&finalCodepoint, 0, 1, cellCols, cellRows, tex, outIsDirty);
}

void UniFont::CacheGlyphs(unsigned int* _codePoints, int count) {
	int charSize = fontScale + 2;
	Texture* tex = GlobalScene->res.textures.GetById(textureId);
	int cellCols = tex->width / charSize;
	int cellRows = tex->height / charSize;

	bool isDirty = false;
	for (int i = 0; i < count; i++) {
		int codePoint = _codePoints[i];
		UnicodeBlockType blockType = GetBlockTypeOfCodePoint(codePoint);
		bool foundSpecialCase = false;
		for (int j = 0; j < BNS_ARRAY_COUNT(fontFunctionSpecialCases); j++) {
			if (blockType == fontFunctionSpecialCases[j].block) {
				foundSpecialCase = true;
				CacheGlyphMemberFunc specialCase = fontFunctionSpecialCases[j].cacheGlyphMethod;
				(this->*specialCase)(_codePoints, i, count, cellCols, cellRows, tex, &isDirty);
			}
		}

		if (!foundSpecialCase) {
			CacheGlyphDefault(_codePoints, i, count, cellCols, cellRows, tex, &isDirty);
		}
	}

	if (isDirty) {
		tex->UploadToGraphicsDevice();
	}
}

struct Rect {
	Vector2 lowerLeft;
	Vector2 upperRight;
};

Rect ScaleRectToRect(Rect src, Rect bounds) {
	Vector2 oldScale = src.upperRight - src.lowerLeft;
	Vector2 boundsScale = bounds.lowerLeft - bounds.upperRight;

	float scaleFactor = BNS_MAX(oldScale.x / boundsScale.x, oldScale.y / boundsScale.y);
	Vector2 newScale = oldScale / scaleFactor;

	Vector2 newCentre = (bounds.lowerLeft + bounds.upperRight) / 2;
	Rect newRect = { newCentre - newScale / 2, newCentre + newScale / 2 };

	return newRect;
}

void GetJamoRects(const int* codePoints, int codepointCount, Rect charRect, Rect* rects) {
	if (codepointCount == 2) {
		float middleX = (charRect.lowerLeft.x + charRect.upperRight.x) / 2;
		rects[0] = charRect;
		rects[1] = charRect;

		rects[0].upperRight.x = middleX;
		rects[1].lowerLeft.x = middleX;
	}
	else if (codepointCount == 3) {
		float middleX = (charRect.lowerLeft.x + charRect.upperRight.x) / 2;
		float middleY = (charRect.lowerLeft.y + charRect.upperRight.y) / 2;
		
		for (int i = 0; i < 2; i++) {
			rects[i] = charRect;
			rects[i].lowerLeft.y = middleY;
		}

		rects[0].upperRight.x = middleX;
		rects[1].lowerLeft.x = middleX;

		rects[2] = charRect;
		rects[2].upperRight.y = middleY;
	}
	else {
		ASSERT_WARN("%s: Codepoint count is %d, needs to be 2 or 3.", __FUNCTION__, codepointCount);
	}
}

void UniFont::BakeVertexDataHangul(const unsigned int* str, int index, int len, float* x, float y, 
								   float width, float height, Texture* fontTexture, 
								   float* outPosData, float* outUvData, int* outIndex) {
	int c = str[index];
	int codePoints[3] = {};
	DecomposeHangulToJamo(c, codePoints);

	int codePointCount = (codePoints[2] == 0x11A7) ? 2 : 3;

	float xAdvance = 0;
	for (int c = 0; c < codePointCount; c++) {
		CodepointInfo* info = GetInfoForCodepoint(codePoints[c]);
		xAdvance = info->xAdvance;

		Vector2 scale = { 1, 1 };
		if (codePointCount == 2) {
			scale = Vector2(0.5f, 1);
		}
		else if (c == 2) {
			scale = Vector2(1, 0.5f);
		}
		else {
			scale = Vector2(0.5f, 0.5f);
		}

		float w = info->w, h = info->h;
		float xAddUv[6] = { 0, w, w, 0, 0, w };
		float yAddUv[6] = { 0, 0, h, 0, h, h };

		float posW = info->w * scale.x;
		float posH = info->h * scale.y;
		float xAddPos[6] = { 0, posW, posW, 0, 0, posW };
		float yAddPos[6] = { 0, 0, posH, 0, posH, posH };

		float jamoX = *x + info->xOffset;
		float jamoY = y - info->yOffset;
		for (int i = 0; i < 6; i++) {
			outPosData[*outIndex] = (jamoX + xAddPos[i]) / GlobalScene->cam.widthPixels * 2 - 1;
			outPosData[*outIndex + 1] = (jamoY - yAddPos[i]) / GlobalScene->cam.heightPixels * 2 - 1;

			outUvData[*outIndex] = (info->x + xAddUv[i]) / fontTexture->width;
			outUvData[*outIndex + 1] = (info->y + yAddUv[i]) / fontTexture->height;

			*outIndex += 2;
		}
	}

	*x += xAdvance;
}

void UniFont::BakeVertexDataArabic(const unsigned int* str, int index, int len, float* x, float y,
								   float width, float height, Texture* fontTexture,
								   float* outPosData, float* outUvData, int* outIndex) {
	unsigned int finalCodepoint = GetArabicRenderCodepoint(str, index, len);
	CodepointInfo* info = GetInfoForCodepoint(finalCodepoint);
	ASSERT(info != nullptr);

	float offsetHack = 100;// width - info->w;
	//*x += offsetHack;
	//*x -= info->xAdvance * 2;
	BakeVertexDataDefault(&finalCodepoint, 0, 1, x, y, width, height, fontTexture, outPosData, outUvData, outIndex);
	//*x -= offsetHack;
	
}

void UniFont::BakeVertexDataDefault(const unsigned int* str, int index, int len, float* x, float y, float width, float height, 
									Texture* fontTexture, float* outPosData, float* outUvData, int* outIndex) {
	int c = str[index];
	CodepointInfo* info = GetInfoForCodepoint(c);

	if (info == nullptr) {
		return;
	}

	float w = info->w, h = info->h;
	float xAdd[6] = { 0, w, w, 0, 0, w };
	float yAdd[6] = { 0, 0, h, 0, h, h };

	if (*x + w > width) {
		xAdd[1] = width - *x;
		xAdd[2] = xAdd[1];
		xAdd[5] = xAdd[1];
	}

	for (int i = 0; i < 6; i++) {
		outPosData[*outIndex] = (*x + info->xOffset + xAdd[i]) / GlobalScene->cam.widthPixels * 2 - 1;
		outPosData[*outIndex + 1] = (y - info->yOffset - yAdd[i]) / GlobalScene->cam.heightPixels * 2 - 1;

		outUvData[*outIndex] = (info->x + xAdd[i]) / fontTexture->width;
		outUvData[*outIndex + 1] = (info->y + yAdd[i]) / fontTexture->height;

		*outIndex += 2;
	}

	*x += info->xAdvance;
}

float UniFont::BakeU32ToVertexData(U32String string, float xStart, float yStart, float width, float height, float* outPosData, float* outUvData, int* outCharsBaked /*= nullptr*/, int* outTriCount /*= nullptr*/) {
	float x = xStart, y = yStart;

	Texture* fontTexture = GlobalScene->res.textures.GetById(textureId);

	int index = 0;
	for (int c = 0; c < string.length && x < xStart + width; c++) {
		int codePoint = string.start[c];
		UnicodeBlockType blockType = GetBlockTypeOfCodePoint(codePoint);
		bool foundSpecialCase = false;
		for (int j = 0; j < BNS_ARRAY_COUNT(fontFunctionSpecialCases); j++) {
			if (blockType == fontFunctionSpecialCases[j].block) {
				foundSpecialCase = true;
				BakeVertexDataMemberFunc specialCase = fontFunctionSpecialCases[j].bakeVertexMethod;
				(this->*specialCase)(string.start, c, string.length, &x, y, width - x, height, fontTexture, outPosData, outUvData, &index);

			}
		}

		if (!foundSpecialCase) {
			BakeVertexDataDefault(string.start, c, string.length, &x, y, width - x, height, fontTexture, outPosData, outUvData, &index);
		}
}

	if (outCharsBaked) {
		*outCharsBaked = string.length;
	}
	
	if (outTriCount) {
		*outTriCount = index / 6;
	}

	return x - xStart;
}

int UniFont::GetQuadCountForText(const U32String string) {
	int quadCount = 0;
	for (int i = 0; i < string.length; i++) {
		int codePoint = string.start[i];
		UnicodeBlockType block = GetBlockTypeOfCodePoint(codePoint);
		bool foundSpecialCase = false;
		for (int j = 0; j < BNS_ARRAY_COUNT(fontFunctionSpecialCases); j++) {
			if (block == fontFunctionSpecialCases[j].block) {
				quadCount += fontFunctionSpecialCases[j].numQuadsPerChar;
				foundSpecialCase = true;
				break;
			}
		}

		if (!foundSpecialCase) {
			CodepointInfo* info = GetInfoForCodepoint(codePoint);
			if (info != nullptr) {
				quadCount++;
			}
		}
	}

	return quadCount;
}

float UniFont::GetCharWidthHangul(const U32String, int index, int c) {
	int codePoints[3] = {};
	DecomposeHangulToJamo(c, codePoints);

	CodepointInfo* info = GetInfoForCodepoint(codePoints[0]);
	return info->xAdvance;
}

float UniFont::GetCursorPos(const U32String string, int cursorPos) {
	ASSERT(cursorPos <= string.length);
	float x = 0.0f;
	for (int i = 0; i < cursorPos; i++) {
		int codePoint = string.start[i];
		UnicodeBlockType block = GetBlockTypeOfCodePoint(codePoint);

		bool foundSpecialCase = false;
		for (int j = 0; j < BNS_ARRAY_COUNT(fontFunctionSpecialCases); j++) {
			if (block == fontFunctionSpecialCases[j].block) {
				GetCharWidthMemberFunc specialCase = fontFunctionSpecialCases[j].getCharWidthMethod;
				if (specialCase != nullptr) {
					x += (this->*specialCase)(string, i, codePoint);
					foundSpecialCase = true;
					break;
				}
			}
		}

		if (!foundSpecialCase) {
			CodepointInfo* info = GetInfoForCodepoint(codePoint);
			if (info != nullptr) {
				x += info->xAdvance;
			}
		}
	}

	return x;
}

void UniFont::AddFont(unsigned char* fontBuffer, int bufferSize) {
	stbtt_fontinfo fontInfo;
	int fontOffset = stbtt_GetFontOffsetForIndex(fontBuffer, 0);
	stbtt_InitFont(&fontInfo, fontBuffer, fontOffset);
	fontInfos.PushBack(fontInfo);
	fontBuffersInMem.PushBack(fontBuffer);
}

void UniFont::CleanUp() {
	for (int i = 0; i < fontBuffersInMem.count; i++) {
		free(fontBuffersInMem.Get(i));
	}

	fontInfos.Clear();
	fontBuffersInMem.Clear();
}

UniFont::~UniFont(){
	CleanUp();
}

