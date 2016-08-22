#include "UniFont.h"

#include "Texture.h"

#include "../core/Scene.h"
#include "../assets/AssetFile.h"

const UniFont::FontSpecialCase UniFont::cacheGlyphSpecialCases[] = {
	{&UniFont::CacheGlyphHangul, &UniFont::BakeVertexDataHangul, UBT_HangulSyllables, 3}
};

CodepointInfo* UniFont::GetInfoForCodepoint(int codepoint) {
	for (int i = 0; i < codepointListing.count; i++) {
		if (codepointListing.data[i].codepoint == codepoint) {
			return &codepointListing.data[i];
		}
	}

	return nullptr;
}

void UniFont::CacheGlyphDefault(int codePoint, int cellCols, int cellRows, Texture* tex, bool* outIsDirty) {
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

			int yOffset = ascent - descent + lineGap;

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

void UniFont::CacheGlyphHangul(int codePoint, int cellCols, int cellRows, Texture* tex, bool* outIsDirty) {
	int codePoints[3] = {};
	DecomposeHangulToJamo(codePoint, codePoints);

	int count = codePoints[2] == 0x11A7 ? 2 : 3;
	for (int i = 0; i < count; i++) {
		CacheGlyphDefault(codePoints[i], cellCols, cellRows, tex, outIsDirty);
	}
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
		for (int j = 0; j < BNS_ARRAY_COUNT(cacheGlyphSpecialCases); j++) {
			if (blockType == cacheGlyphSpecialCases[j].block) {
				foundSpecialCase = true;
				CacheGlyphMemberFunc specialCase = cacheGlyphSpecialCases[j].cacheGlyphMethod;
				(this->*specialCase)(codePoint, cellCols, cellRows, tex, &isDirty);
			}
		}

		if (!foundSpecialCase) {
			CacheGlyphDefault(codePoint, cellCols, cellRows, tex, &isDirty);
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

void UniFont::BakeVertexDataHangul(int c, float* x, float y, float width, float height, Texture* fontTexture, float* outPosData, float* outUvData, int* index) {
	int codePoints[3] = {};
	DecomposeHangulToJamo(c, codePoints);

	float scale = fontScale;
	float startX = *x;

	Rect jamoRects[3];
	Rect charRect = { Vector2(startX, y), Vector2(startX + fontScale, y + fontScale) };

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
			outPosData[*index] = (jamoX + xAddPos[i]) / GlobalScene->cam.widthPixels * 2 - 1;
			outPosData[*index + 1] = (jamoY - yAddPos[i]) / GlobalScene->cam.heightPixels * 2 - 1;

			outUvData[*index] = (info->x + xAddUv[i]) / fontTexture->width;
			outUvData[*index + 1] = (info->y + yAddUv[i]) / fontTexture->height;

			*index += 2;
		}
	}

	*x += xAdvance;
}


/*
for (int i = 0; i < 3; i++) {
BakeVertexDataDefault(codePoints[i], x, y, width, height, fontTexture, outPosData, outUvData, index);
}
*/

/*
for (int c = 0; c < 2; c++) {
CodepointInfo* info = GetInfoForCodepoint(codePoints[c]);

float w = info->w, h = info->h;
float xAddUv[6] = { 0, w, w, 0, 0, w };
float yAddUv[6] = { 0, 0, h, 0, h, h };

float xAddPos[6] = { 0, w / 2, w / 2 , 0, 0, w / 2 };
float yAddPos[6] = { 0, 0, h, 0, h, h };

for (int i = 0; i < 6; i++) {
outPosData[*index] = (*x + xAddPos[i]) / GlobalScene->cam.widthPixels * 2 - 1;
outPosData[*index + 1] = (y - yAddPos[i] + h) / GlobalScene->cam.heightPixels * 2 - 1;

outUvData[*index] = (info->x + xAddUv[i]) / fontTexture->width;
outUvData[*index + 1] = (info->y + yAddUv[i]) / fontTexture->height;

*index += 2;
}

*x += (info->w / 2);

if (c == 1) {
*x = startX + info->xAdvance;
}
}

for (int i = 0; i < 12; i++) {
outPosData[*index] = 0;
outUvData[*index] = 0;

(*index)++;
}
*/

void UniFont::BakeVertexDataDefault(int c, float* x, float y, float width, float height, Texture* fontTexture, float* outPosData, float* outUvData, int* index) {
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
		outPosData[*index] = (*x + info->xOffset + xAdd[i]) / GlobalScene->cam.widthPixels * 2 - 1;
		outPosData[*index + 1] = (y - info->yOffset - yAdd[i]) / GlobalScene->cam.heightPixels * 2 - 1;

		outUvData[*index] = (info->x + xAdd[i]) / fontTexture->width;
		outUvData[*index + 1] = (info->y + yAdd[i]) / fontTexture->height;

		*index += 2;
	}

	*x += info->xAdvance;
}

float UniFont::BakeU32ToVertexData(U32String string, float xStart, float yStart, float width, float height, float* outPosData, float* outUvData, int* outCharsBaked /*= nullptr*/) {
	float x = xStart, y = yStart;

	Texture* fontTexture = GlobalScene->res.textures.GetById(textureId);

	int index = 0;
	for (int c = 0; c < string.length && x < xStart + width; c++) {
		int codePoint = string.start[c];
		UnicodeBlockType blockType = GetBlockTypeOfCodePoint(codePoint);
		bool foundSpecialCase = false;
		for (int j = 0; j < BNS_ARRAY_COUNT(cacheGlyphSpecialCases); j++) {
			if (blockType == cacheGlyphSpecialCases[j].block) {
				foundSpecialCase = true;
				BakeVertexDataMemberFunc specialCase = cacheGlyphSpecialCases[j].bakeVertexMethod;
				(this->*specialCase)(codePoint, &x, y, width - x, height, fontTexture, outPosData, outUvData, &index);

			}
		}

		if (!foundSpecialCase) {
			BakeVertexDataDefault(codePoint, &x, y, width - x, height, fontTexture, outPosData, outUvData, &index);
		}
}

	if (outCharsBaked) {
		*outCharsBaked = string.length;
	}

	return x - xStart;
}

int UniFont::GetQuadCountForText(const U32String string) {
	int quadCount = 0;
	for (int i = 0; i < string.length; i++) {
		int codePoint = string.start[i];
		UnicodeBlockType block = GetBlockTypeOfCodePoint(codePoint);
		bool foundSpecialCase = false;
		for (int j = 0; j < BNS_ARRAY_COUNT(cacheGlyphSpecialCases); j++) {
			if (block == cacheGlyphSpecialCases[j].block) {
				quadCount += cacheGlyphSpecialCases[j].numQuadsPerChar;
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

float UniFont::GetCursorPos(const char* text, int cursorPos) {
	return 0;
}

void UniFont::AddFont(unsigned char* fontBuffer, int bufferSize) {
	stbtt_fontinfo fontInfo;
	fontInfos.PushBack(fontInfo);
	int fontOffset = stbtt_GetFontOffsetForIndex(fontBuffer, 0);
	stbtt_InitFont(&fontInfos.data[fontInfos.count-1], fontBuffer, fontOffset);
	fontBuffersInMem.PushBack(fontBuffer);
}

void UniFont::CleanUp() {
	for (int i = 0; i < fontBuffersInMem.count; i++) {
		free(fontBuffersInMem.Get(i));
	}

	fontInfos.Clear();
	fontBuffersInMem.Clear();
}

