#include "BitmapFont.h"

#include "../core/Scene.h"

CodepointInfo* BitmapFont::GetInfoForCodepoint(int codepoint){
	for(int i = 0; i < codepointListing.count; i++){
		if(codepointListing.data[i].codepoint == codepoint){
			return &codepointListing.data[i];
		}
	}
	
	return nullptr;
}

void BitmapFont::BakeAsciiToVertexData(const char* text, float xStart, float yStart, float* outPosData, float* outUvData){
	const char* cursor = text;
	int index = 0;
	
	float x = xStart, y = yStart;

	Texture* fontTexture = GlobalScene->res.textures.GetById(textureId);

	while(*cursor){
		CodepointInfo* info = GetInfoForCodepoint(*cursor);
		
		float w = info->w, h = info->h;
		float xAdd[6] = {0, w, w, 0, 0, w};
		float yAdd[6] = {0, 0, h, 0, h, h};
		
		for(int i = 0; i < 6; i++){
			outPosData[index] = (x + info->xOffset + xAdd[i]) / GlobalScene->cam.widthPixels * 2 - 1;
			outPosData[index+1] = (y - info->yOffset - yAdd[i]) / GlobalScene->cam.heightPixels * 2 - 1;
			
			outUvData[index] = (info->x + xAdd[i]) / fontTexture->width;
			outUvData[index+1] = (info->y + yAdd[i]) / fontTexture->height;
			
			index += 2;
		}
		
		x += info->xAdvance;
		
		cursor++;
	}
}

float BitmapFont::GetCursorPos(const char* text, int cursorPos) {
	float x = 0;
	for (int i = 0; i < cursorPos; i++) {
		ASSERT(text[i] != '\0');

		CodepointInfo* info = GetInfoForCodepoint(text[i]);
		x += info->xAdvance;
	}

	return x;
}
