#include "AssetFile.h"

#include "../gfx/Material.h"

#include "../../ext/CppUtils/filesys.h"
#include "../../ext/CppUtils/stringmap.h"
#include "../../ext/CppUtils/memstream.h"
#include "../../ext/CppUtils/xml.h"

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

	Vector<File*> materialFiles;
	assetDir.FindFilesWithExt("mat", &materialFiles);

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

	for (int i = 0; i < materialFiles.count; i++) {
		assetFileIds.Insert(materialFiles.data[i]->fileName, i);
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

	for (int i = 0; i < materialFiles.count; i++) {
		WriteMaterialChunk(materialFiles.data[i]->fullName, assetFileIds, i, assetFile);
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
		
		size_t lineKeyLen = strcspn(fileCursor, whitespace);
		char* lineKey = fileCursor;

		fileCursor += lineKeyLen;
		fileCursor += strspn(fileCursor, whitespace);

		if (strncmp(lineKey, "v", lineKeyLen) == 0) {
			Vector3 pos;
			for (int i = 0; i < 3; i++) {
				fileCursor += strcspn(fileCursor, numbers);
				((float*)&pos)[i] = (float)atof(fileCursor);
				fileCursor += strspn(fileCursor, numbers);
			}

			flags |= MCF_POSITIONS;
			positions.PushBack(pos);
		}
		else if (strncmp(lineKey, "vt", lineKeyLen) == 0) {
			Vector2 uv;
			for (int i = 0; i < 2; i++) {
				fileCursor += strcspn(fileCursor, numbers);
				((float*)&uv)[i] = (float)atof(fileCursor);
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

UniformType ParseUniformType(const char* typeName) {
	if (typeName == nullptr) {
		return UT_UNKNOWN;
	}

	if (StrEqual(typeName, "tex2D")) {
		return UT_TEXTURE2D;
	}
	else if (StrEqual(typeName, "int")) {
		return UT_INTEGER;
	}
	else if (StrEqual(typeName, "float")) {
		return UT_FLOAT;
	}
	else if (StrEqual(typeName, "mat4")) {
		return UT_MATRIX4;
	}
	else if (StrEqual(typeName, "vec2")) {
		return UT_VEC2;
	}
	else if (StrEqual(typeName, "vec3")) {
		return UT_VEC3;
	}
	else if (StrEqual(typeName, "vec4")) {
		return UT_VEC4;
	}
	else {
		return UT_UNKNOWN;
	}
}

void WriteMaterialChunk(const char* materialFileName, const StringMap<int>& assetIds, int id, FILE* assetFileHandle) {
	XMLDoc matDoc;
	XMLError err = ParseXMLStringFromFile(materialFileName, &matDoc);
	ASSERT_MSG(err == XMLE_NONE, "Error %d when parsing material file: '%s'", (int)err, materialFileName);

	XMLElement* rootElem = matDoc.elements.GetById(0);
	ASSERT(rootElem != nullptr);
	ASSERT(rootElem->name == "Material");

	String vShader;
	String fShader;

	ASSERT(rootElem->attributes.LookUp("vs", &vShader));
	ASSERT(rootElem->attributes.LookUp("fs", &fShader));

	int vShaderId;
	int fShaderId;
	ASSERT(assetIds.LookUp(vShader, &vShaderId));
	ASSERT(assetIds.LookUp(fShader, &fShaderId));

	int uniformCount = 0;
	for (int i = 0; i < rootElem->childrenIds.count; i++) {
		XMLElement* childElem = matDoc.elements.GetById(rootElem->childrenIds.Get(i));
		if (childElem->name == "Uniform") {
			uniformCount++;
		}
	}

	char chunkId[] = "BNMT";
	fwrite(chunkId, 1, 4, assetFileHandle);
	fwrite(&id, 1, 4, assetFileHandle);

	fwrite(&vShaderId, 1, sizeof(int), assetFileHandle);
	fwrite(&fShaderId, 1, sizeof(int), assetFileHandle);
	fwrite(&uniformCount, 1, sizeof(int), assetFileHandle);

	for (int i = 0; i < rootElem->childrenIds.count; i++) {
		XMLElement* childElem = matDoc.elements.GetById(rootElem->childrenIds.Get(i));
		if (childElem->name == "Uniform") {
			String uniformType;
			childElem->attributes.LookUp("type", &uniformType);
			UniformType type = ParseUniformType(uniformType.string);

			String uniformName;
			childElem->attributes.LookUp("name", &uniformName);

			if (type == UT_UNKNOWN || uniformName.string == nullptr) {
				continue;
			}

			int nameLength = uniformName.GetLength() + 1; // Include null byte
			fwrite(&nameLength, 1, sizeof(int), assetFileHandle);

			fwrite(uniformName.string, 1, nameLength, assetFileHandle);

			fwrite(&type, 1, sizeof(UniformType), assetFileHandle);

			String valString;
			childElem->attributes.LookUp("value", &valString);

			switch (type) {

			case UT_FLOAT: {
				float val = (float)atof(valString.string);
				fwrite(&val, 1, sizeof(float), assetFileHandle);
			} break;

			case UT_INTEGER: {
				int val = Atoi(valString.string);
				fwrite(&val, 1, sizeof(int), assetFileHandle);
			} break;

			case UT_TEXTURE2D: {
				int texId;
				assetIds.LookUp(valString.string, &texId);
				fwrite(&texId, 1, sizeof(int), assetFileHandle);
			} break;

			default:
				break;
			}
		}
	}

	int chunkIdFlipped = ~*(int*)chunkId;
	fwrite(&chunkIdFlipped, 1, 4, assetFileHandle);
}