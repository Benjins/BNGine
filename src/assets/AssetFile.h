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

number of files 4 bytes
for each file:
asset id
string length inlcuding null terminator 4 bytes
string including null terminator (n bytes)

- BNSA negated signals EOF

Chunks:
Chunk type
Asset id
Chunk length
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

void PackAssetFile(const char* assetDir, const char* packedFileName);

void WriteMeshChunk(const char* meshFileName, int id, FILE* assetFileHandle);
void WriteVShaderChunk(const char* shaderFileName, int id, FILE* assetFileHandle);
void WriteFShaderChunk(const char* shaderFileName, int id, FILE* assetFileHandle);

#endif
