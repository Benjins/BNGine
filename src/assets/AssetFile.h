#ifndef ASSETSYSTEM_H
#define ASSETSYSTEM_H

#include <stdio.h>

#pragma once

/*
Asset files:

*.bna

BNSA    4 bytes
Version 4 bytes

header:
list of origin file names and their asset id's

the file name is the file name and extension, not including directory.  
Collisions are possible if two files have the same name but are in different directories. 

number of files 4 bytes
for each file:
asset id
string length inlcuding null terminator 4 bytes
string including null terminator (n bytes)

- BNSA negated signals EOF

Chunks:
Chunk type
Asset id
Chunk data
Chunk footer (bitwise negation of type)

Chunk types:
- mdf file (model)
- texture
- shader
- material

*/

#define ASSET_FILE_VERSION 100

enum ModelChunkFlags {
	MCF_NONE = 0,
	MCF_POSITIONS = (1 << 0),
	MCF_UVS = (1 << 1)
};

enum AssetType {
	AT_VSHADER,
	AT_FSHADER,
	AT_MODEL
};

template<typename T>
struct StringMap;

template<typename T>
struct Vector;

struct File;

void PackAssetFile(const char* assetDir, const char* packedFileName);

void WriteMeshChunk(const char* meshFileName, int id, FILE* assetFileHandle);
void WriteVShaderChunk(const char* shaderFileName, int id, FILE* assetFileHandle);
void WriteFShaderChunk(const char* shaderFileName, int id, FILE* assetFileHandle);
void WriteTextureChunk(const char* textureFileName, int id, FILE* assetFileHandle);
void WriteMaterialChunk(const char* materialFileName, const StringMap<int>& assetIds, int id, FILE* assetFileHandle);
void WriteLevelChunk(const char* levelFileName, const StringMap<int>& assetIds, int id, FILE* assetFileHandle);
void WriteBitmapFontChunk(const char* fontFileName, const Vector<File*>& ttfFiles, int id, FILE* assetFileHandle);
void WriteUniFontChunk(const char* fontFileName, const Vector<File*>& ttfFiles, int id, FILE* assetFileHandle);
void WritePrefabChunk(const char* prefabFileName, const StringMap<int>& assetIds, int id, FILE* assetFileHandle);
void WriteScriptChunk(const char* scriptFileName, const StringMap<int>& assetIds, int id, FILE* assetFileHandle);

void WriteAssetNameIdMap(const StringMap<int>& map, FILE* assetFileHandle);

void GlyphBlit(unsigned char* dst, int dstWidth, int dstHeight, int dstX, int dstY, unsigned char* src, int srcWidth, int srcHeight);

#endif
