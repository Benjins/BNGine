#include "AssetFile.h"
#include "../../ext/CppUtils/filesys.h"
#include "../../ext/CppUtils/stringmap.h"
#include "../../ext/CppUtils/memstream.h"

#include "../../ext/3dbasics/Vector3.h"
#include "../../ext/3dbasics/Vector2.h"

void PackAssetFile(const char* assetDirName, const char* packedFileName) {
	File assetDir;
	assetDir.Load(assetDirName);

	char fileId[] = "BNSA";

	FILE* assetFile = fopen(packedFileName, "wb");

	fwrite(fileId, 1, 4, assetFile);
	int version = ASSET_FILE_VERSION;
	fwrite(&version, 1, 4, assetFile);

	StringMap<int> assetFileIds;
	
	Vector<File*> meshFiles;
	assetDir.FindFilesWithExt("obj", &meshFiles);

	Vector<File*> vShaderFiles;
	assetDir.FindFilesWithExt("vs", &vShaderFiles);

	Vector<File*> fShaderFiles;
	assetDir.FindFilesWithExt("fs", &fShaderFiles);

	Vector<File*> textureFiles;
	assetDir.FindFilesWithExt("bmp", &textureFiles);

	for (int i = 0; i < meshFiles.count; i++) {
		assetFileIds.Insert(meshFiles.data[i]->fileName, i);
	}

	for (int i = 0; i < vShaderFiles.count; i++) {
		assetFileIds.Insert(vShaderFiles.data[i]->fileName, i);
	}

	for (int i = 0; i < fShaderFiles.count; i++) {
		assetFileIds.Insert(fShaderFiles.data[i]->fileName, vShaderFiles.count + i);
	}

	for (int i = 0; i < textureFiles.count; i++) {
		assetFileIds.Insert(textureFiles.data[i]->fileName, i);
	}

	WriteAssetNameIdMap(assetFileIds, assetFile);

	for (int i = 0; i < meshFiles.count; i++) {
		WriteMeshChunk(meshFiles.data[i]->fullName, i, assetFile);
	}

	for (int i = 0; i < vShaderFiles.count; i++) {
		WriteVShaderChunk(vShaderFiles.data[i]->fullName, i, assetFile);
	}

	for (int i = 0; i < fShaderFiles.count; i++) {
		WriteFShaderChunk(fShaderFiles.data[i]->fullName, vShaderFiles.count + i, assetFile);
	}

	for (int i = 0; i < textureFiles.count; i++) {
		WriteTextureChunk(textureFiles.data[i]->fullName, i, assetFile);
	}

	int bnsaNegated = ~*(int*)fileId;
	fwrite(&bnsaNegated, 1, 4, assetFile);

	fclose(assetFile);

	assetDir.Unload();
}

void WriteAssetNameIdMap(const StringMap<int>& map, FILE* assetFileHandle) {
	int nameCount = map.count;
	fwrite(&nameCount, 1, sizeof(int), assetFileHandle);

	for (int i = 0; i < nameCount; i++) {
		int nameLength = map.names[i].GetLength() + 1;
		fwrite(&nameLength, 1, sizeof(int), assetFileHandle);
		fwrite(&map.values[i], 1, sizeof(int), assetFileHandle);
		fwrite(map.names[i].string, 1, nameLength, assetFileHandle);
	}
}

void WriteMeshChunk(const char* modelFileName, int id, FILE* assetFileHandle) {
	int modelFileSize = 0;
	char* modelFileText = ReadTextFile(modelFileName, &modelFileSize);
	ASSERT_MSG(modelFileText != nullptr, "Could not open model file '%s'", modelFileName);

	char* fileCursor = modelFileText;

	int flags = MCF_NONE;

	Vector<Vector3> positions;
	Vector<Vector2> uvs;
	Vector<int> indices;

	static const char* whitespace = " \t\n\r";
	static const char* numbers = "0123456789-.";

	while (fileCursor != nullptr && fileCursor - modelFileText < modelFileSize) {
		fileCursor += strspn(fileCursor, whitespace);
		
		int lineKeyLen = strcspn(fileCursor, whitespace);
		char* lineKey = fileCursor;

		fileCursor += lineKeyLen;
		fileCursor += strspn(fileCursor, whitespace);

		if (strncmp(lineKey, "v", lineKeyLen) == 0) {
			Vector3 pos;
			for (int i = 0; i < 3; i++) {
				fileCursor += strcspn(fileCursor, numbers);
				((float*)&pos)[i] = atof(fileCursor);
				fileCursor += strspn(fileCursor, numbers);
			}

			flags |= MCF_POSITIONS;
			positions.PushBack(pos);
		}
		else if (strncmp(lineKey, "vt", lineKeyLen) == 0) {
			Vector2 uv;
			for (int i = 0; i < 2; i++) {
				fileCursor += strcspn(fileCursor, numbers);
				((float*)&uv)[i] = atof(fileCursor);
				fileCursor += strspn(fileCursor, numbers);
			}

			flags |= MCF_UVS;
			uvs.PushBack(uv);
		}
		else if (strncmp(lineKey, "f", lineKeyLen) == 0) {
			for (int i = 0; i < 3; i++) {
				fileCursor += strcspn(fileCursor, numbers);
				int posIdx = Atoi(fileCursor) - 1;
				fileCursor += strspn(fileCursor, numbers);

				indices.PushBack(posIdx);

				if (flags & MCF_UVS) {
					fileCursor += strcspn(fileCursor, numbers);
					int uvIdx = Atoi(fileCursor) - 1;
					fileCursor += strspn(fileCursor, numbers);

					indices.PushBack(uvIdx);
				}
			}
		}

		fileCursor = strstr(fileCursor, "\n");
	}

	free(modelFileText);

	int dataSize = 4 + 4 + positions.count * sizeof(Vector3) + 4 + uvs.count * sizeof(Vector2) + 4 + indices.count * sizeof(int);

	char chunkId[] = "BNMD";
	fwrite(chunkId, 1, 4, assetFileHandle);
	fwrite(&id, 1, 4, assetFileHandle);
	fwrite(&flags, 1, 4, assetFileHandle);

	fwrite(&positions.count, 1, 4, assetFileHandle);
	fwrite(positions.data, sizeof(Vector3), positions.count, assetFileHandle);

	fwrite(&uvs.count, 1, 4, assetFileHandle);
	fwrite(uvs.data, sizeof(Vector2), uvs.count, assetFileHandle);

	fwrite(&indices.count, 1, 4, assetFileHandle);
	fwrite(indices.data, sizeof(int), indices.count, assetFileHandle);

	int chunkIdFlipped = ~*(int*)chunkId;
	fwrite(&chunkIdFlipped, 1, 4, assetFileHandle);
}

void WriteVShaderChunk(const char* shaderFileName, int id, FILE* assetFileHandle){
	int shaderFileSize = 0;
	char* shaderFileText = ReadTextFile(shaderFileName, &shaderFileSize);
	shaderFileSize++; //Get the null byte too

	char chunkId[] = "BNVS";
	fwrite(chunkId, 1, 4, assetFileHandle);
	fwrite(&id, 1, 4, assetFileHandle);
	fwrite(shaderFileText, 1, shaderFileSize, assetFileHandle);

	free(shaderFileText);

	int chunkIdFlipped = ~*(int*)chunkId;
	fwrite(&chunkIdFlipped, 1, 4, assetFileHandle);
}

void WriteFShaderChunk(const char* shaderFileName, int id, FILE* assetFileHandle){
	int shaderFileSize = 0;
	char* shaderFileText = ReadTextFile(shaderFileName, &shaderFileSize);
	shaderFileSize++; //Get the null byte too

	char chunkId[] = "BNFS";
	fwrite(chunkId, 1, 4, assetFileHandle);
	fwrite(&id, 1, 4, assetFileHandle);
	fwrite(shaderFileText, 1, shaderFileSize, assetFileHandle);

	free(shaderFileText);

	int chunkIdFlipped = ~*(int*)chunkId;
	fwrite(&chunkIdFlipped, 1, 4, assetFileHandle);
}

#if defined(_MSC_VER)

#pragma pack(1)
struct BitMapHeader {
#else
typedef struct __attribute((packed))__{
#endif
	short fileTag;
	int fileSize;
	short reservedA;
	short reservedB;
	int imageDataOffset;

	int headerSize;
	int imageWidth;
	int imageHeight;
	short numColorPlanes;
	short bitDepth;
	int compressionMethod;
	int imageDataSize;
	int horizontalResolution;
	int verticalResolution;
	int numPaletteColors;
	int numImportantColors;
#if defined(_MSC_VER)
};
#pragma pack()
#else
} BitMapHeader;
#endif

void WriteTextureChunk(const char* textureFileName, int id, FILE* assetFileHandle) {
	MemStream stream;
	stream.ReadInFromFile(textureFileName);

	BitMapHeader hdr;
	stream.ReadArray(&hdr, 1);

	//Jump to the image data
	stream.readHead = VOID_PTR_ADD(stream.base, hdr.imageDataOffset);

	char chunkId[] = "BNTX";
	fwrite(chunkId, 1, 4, assetFileHandle);
	fwrite(&id, 1, 4, assetFileHandle);

	fwrite(&hdr.imageWidth, 1, sizeof(int), assetFileHandle);
	fwrite(&hdr.imageHeight, 1, sizeof(int), assetFileHandle);
	fwrite(stream.readHead, 3, hdr.imageWidth * hdr.imageHeight, assetFileHandle);	

	int chunkIdFlipped = ~*(int*)chunkId;
	fwrite(&chunkIdFlipped, 1, 4, assetFileHandle);
}

