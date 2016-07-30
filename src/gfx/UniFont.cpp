#include "UniFont.h"

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

void UniFont::CacheGlyphHangul(int codePoint, int cellCols, int cellRows, Texture* tex, bool* outIsDirty) {

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

void UniFont::BakeVertexDataHangul(int c, float* x, float y, float width, float height, Texture* fontTexture, float* outPosData, float* outUvData, int* index) {
	// TODO: stub
}

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

