#include "UniFont.h"

#include "../core/Scene.h"
#include "../assets/AssetFile.h"

CodepointInfo* UniFont::GetInfoForCodepoint(int codepoint) {
	for (int i = 0; i < codepointListing.count; i++) {
		if (codepointListing.data[i].codepoint == codepoint) {
			return &codepointListing.data[i];
		}
	}

	return nullptr;
}

void UniFont::CacheGlyphs(unsigned int* codePoints, int count) {
	int charSize = fontScale + 2;

	Texture* tex = GlobalScene->res.textures.GetById(textureId);
	int cellCols = tex->width / charSize;
	int cellRows = tex->height / charSize;
	bool isDirty = false;
	for (int i = 0; i < count; i++) {
		if (GetInfoForCodepoint(codePoints[i]) == nullptr) {
			stbtt_fontinfo* font = nullptr;
			unsigned char* cBmp = nullptr;
			int cW=0, cH=0;
			for (int j = 0; j < fontInfos.count; j++) {
				float pixelScale = stbtt_ScaleForPixelHeight(&fontInfos.data[j], fontScale);
				cBmp = stbtt_GetCodepointBitmap(&fontInfos.data[j], 0, pixelScale, codePoints[i], &cW, &cH, 0, 0);
				if (cBmp) {
					font = &fontInfos.data[j];
					break;
				}
			}

			if (font != nullptr) {
				isDirty = true;
				int ascent, descent, lineGap;
				stbtt_GetFontVMetrics(font, &ascent, &descent, &lineGap);

				int yOffset = ascent - descent + lineGap;

				float pixelScale = stbtt_ScaleForPixelHeight(font, fontScale);

				int cellY = cacheCursor / cellCols;
				int cellX = cacheCursor % cellCols;

				int startX = cellX * charSize;
				int startY = cellY * charSize;

				GlyphBlit(tex->texMem, tex->width, tex->height, startX, startY, cBmp, cW, cH);
				
				/*
				unsigned char* bmpCursor = cBmp;
				for (int j = 0; j < cH; j++){
					unsigned char* texMemCursor = &tex->texMem[(startX + ((startY+j)*tex->width)) * 3];
					for (int i = 0; i < cW; i++) {
						*texMemCursor++ = *bmpCursor++;
					}
				}*/

				free(cBmp);

				int advanceWidth = 0, leftSideBearing = 0;
				stbtt_GetCodepointHMetrics(font, codePoints[i], &advanceWidth, &leftSideBearing);

				int x0, y0, x1, y1;
				stbtt_GetCodepointBitmapBox(font, codePoints[i], pixelScale, pixelScale, &x0, &y0, &x1, &y1);
				CodepointInfo pointInfo = {};
				pointInfo.codepoint = codePoints[i];
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

	if (isDirty) {
		tex->UploadToGraphicsDevice();
	}
}

float UniFont::BakeU32ToVertexData(U32String string, float xStart, float yStart, float width, float height, float* outPosData, float* outUvData, int* outCharsBaked /*= nullptr*/) {
	int index = 0;

	float x = xStart, y = yStart;

	Texture* fontTexture = GlobalScene->res.textures.GetById(textureId);

	for (int c = 0; c < string.length && x < xStart + width; c++) {
		CodepointInfo* info = GetInfoForCodepoint(string.start[c]);

		if (info == nullptr) {
			continue;
		}

		float w = info->w, h = info->h;
		float xAdd[6] = { 0, w, w, 0, 0, w };
		float yAdd[6] = { 0, 0, h, 0, h, h };

		if (x + w > xStart + width) {
			xAdd[1] = (xStart + width) - x;
			xAdd[2] = xAdd[1];
			xAdd[5] = xAdd[1];
		}

		for (int i = 0; i < 6; i++) {
			outPosData[index] = (x + info->xOffset + xAdd[i]) / GlobalScene->cam.widthPixels * 2 - 1;
			outPosData[index + 1] = (y - info->yOffset - yAdd[i]) / GlobalScene->cam.heightPixels * 2 - 1;

			outUvData[index] = (info->x + xAdd[i]) / fontTexture->width;
			outUvData[index + 1] = (info->y + yAdd[i]) / fontTexture->height;

			index += 2;
		}

		x += info->xAdvance;
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

