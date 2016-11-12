#include "AssetFile.h"

#include "../core/Scene.h"

#include "../gfx/Material.h"
#include "../gfx/BitmapFont.h"

#include "../script/ScriptObject.h"

#include "../util/Serialization.h"
#include "../metagen/ComponentMeta.h"
#include "../metagen/MetaStruct.h"

#include "../../ext/CppUtils/filesys.h"
#include "../../ext/CppUtils/stringmap.h"
#include "../../ext/CppUtils/memstream.h"
#include "../../ext/CppUtils/vector.h"
#include "../../ext/CppUtils/xml.h"

#include "../../ext/CppUtils/macros.h"

#include "../../ext/3dbasics/Vector2.h"
#include "../../ext/3dbasics/Vector3.h"
#include "../../ext/3dbasics/Vector4.h"
#include "../../ext/3dbasics/Quaternion.h"

void PackAssetFile(const char* assetDirName, const char* packedFileName) {
	File assetDir;
	assetDir.Load(assetDirName);

	char fileId[] = "BNSA";

	FILE* assetFile = fopen(packedFileName, "wb");

	fwrite(fileId, 1, 4, assetFile);
	int version = ASSET_FILE_VERSION;
	fwrite(&version, 1, 4, assetFile);

	StringMap<int>& assetFileIds = GlobalScene->res.assetIdMap;
	
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

	Vector<File*> levelFiles;
	assetDir.FindFilesWithExt("lvl", &levelFiles);
	
	Vector<File*> ttfFiles;
	assetDir.FindFilesWithExt("ttf", &ttfFiles);

	Vector<File*> bitmapFontFiles;
	assetDir.FindFilesWithExt("fnt", &bitmapFontFiles);

	Vector<File*> uniFontFiles;
	assetDir.FindFilesWithExt("uft", &uniFontFiles);

	Vector<File*> prefabFiles;
	assetDir.FindFilesWithExt("bnp", &prefabFiles);

	Vector<File*> scriptFiles;
	assetDir.FindFilesWithExt("bnv", &scriptFiles);

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

	for (int i = 0; i < levelFiles.count; i++) {
		assetFileIds.Insert(levelFiles.data[i]->fileName, i);
	}

	for (int i = 0; i < bitmapFontFiles.count; i++) {
		assetFileIds.Insert(bitmapFontFiles.data[i]->fileName, i);
	}

	for (int i = 0; i < uniFontFiles.count; i++) {
		assetFileIds.Insert(uniFontFiles.data[i]->fileName, i);
	}

	for (int i = 0; i < prefabFiles.count; i++) {
		assetFileIds.Insert(prefabFiles.data[i]->fileName, i);
	}

	for (int i = 0; i < scriptFiles.count; i++) {
		assetFileIds.Insert(scriptFiles.data[i]->fileName, i);
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

	for (int i = 0; i < levelFiles.count; i++) {
		WriteLevelChunk(levelFiles.data[i]->fullName, assetFileIds, i, assetFile);
	}

	for (int i = 0; i < bitmapFontFiles.count; i++) {
		WriteBitmapFontChunk(bitmapFontFiles.data[i]->fullName, ttfFiles, i, assetFile);
	}

	for (int i = 0; i < uniFontFiles.count; i++) {
		WriteUniFontChunk(uniFontFiles.data[i]->fullName, ttfFiles, i, assetFile);
	}

	for (int i = 0; i < prefabFiles.count; i++) {
		WritePrefabChunk(prefabFiles.data[i]->fullName, assetFileIds, i, assetFile);
	}

	for (int i = 0; i < scriptFiles.count; i++) {
		WriteScriptChunk(scriptFiles.data[i]->fullName, assetFileIds, i, assetFile);
	}

	int bnsaNegated = ~*(int*)fileId;
	fwrite(&bnsaNegated, 1, 4, assetFile);

	fclose(assetFile);

	assetFileIds.Clear();

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
		ASSERT_WARN("Unkown uniform type name: '%s'", typeName);
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

	bool found = rootElem->attributes.LookUp("vs", &vShader);
	ASSERT(found);
	found = rootElem->attributes.LookUp("fs", &fShader);
	ASSERT(found);

	int vShaderId;
	int fShaderId;
	
	found = assetIds.LookUp(vShader, &vShaderId);
	ASSERT(found);
	found = assetIds.LookUp(fShader, &fShaderId);
	ASSERT(found);

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

			case UT_VEC2: {
				Vector2 val = ParseVector2(valString.string);
				fwrite(&val, 1, sizeof(Vector2), assetFileHandle);
			} break;

			case UT_VEC3: {
				Vector3 val = ParseVector3(valString.string);
				fwrite(&val, 1, sizeof(Vector3), assetFileHandle);
			} break;

			case UT_VEC4: {
				Vector4 val = ParseVector4(valString.string);
				fwrite(&val, 1, sizeof(Vector4), assetFileHandle);
			} break;

			default:
				break;
			}
		}
	}

	int chunkIdFlipped = ~*(int*)chunkId;
	fwrite(&chunkIdFlipped, 1, 4, assetFileHandle);
}

#define XML_PARSE_FLOAT_ATTR(xmlElem,varName,attrName) {String varName##_str; xmlElem->attributes.LookUp(attrName, &varName##_str); varName = (float)atof(varName##_str.string);}
#define XML_PARSE_SERIAL_ATTR(xmlElem,varName,attrName,parseFunc) {String varName##_str; xmlElem->attributes.LookUp(attrName, &varName##_str); varName = parseFunc(varName##_str.string);}

void WriteTransformData(XMLElement* transElem, FILE* assetFileHandle) {
	int id;
	XML_PARSE_SERIAL_ATTR(transElem, id, "id", Atoi);

	int parent;
	XML_PARSE_SERIAL_ATTR(transElem, parent, "parent", Atoi);

	Vector3 pos;
	Quaternion rot;
	Vector3 scale;

	XML_PARSE_SERIAL_ATTR(transElem, pos, "pos", ParseVector3);
	XML_PARSE_SERIAL_ATTR(transElem, rot, "rotation", ParseQuaternion);
	XML_PARSE_SERIAL_ATTR(transElem, scale, "scale", ParseVector3);

	fwrite(&id, 1, sizeof(int), assetFileHandle);
	fwrite(&parent, 1, sizeof(int), assetFileHandle);
	fwrite(&pos, 1, sizeof(Vector3), assetFileHandle);
	fwrite(&rot, 1, sizeof(Quaternion), assetFileHandle);
	fwrite(&scale, 1, sizeof(Vector3), assetFileHandle);
}

void WriteCameraSubChunk(XMLElement* entElem, FILE* assetFileHandle) {
	ASSERT(entElem->childrenIds.count == 1);
	XMLElement* transElem = entElem->doc->elements.GetById(entElem->childrenIds.data[0]);
	ASSERT(transElem->name == "Transform");

	char subChunkId[] = "CMRA";
	fwrite(subChunkId, 1, 4, assetFileHandle);
	
	float fov, nearClip, farClip;
	XML_PARSE_FLOAT_ATTR(entElem, fov, "fov");
	XML_PARSE_FLOAT_ATTR(entElem, nearClip, "nearClip");
	XML_PARSE_FLOAT_ATTR(entElem, farClip, "farClip");

	fwrite(&fov, 1, sizeof(float), assetFileHandle);
	fwrite(&nearClip, 1, sizeof(float), assetFileHandle);
	fwrite(&farClip, 1, sizeof(float), assetFileHandle);

	WriteTransformData(transElem, assetFileHandle);

	int subChunkIdFlipped = ~*(int*)subChunkId;
	fwrite(&subChunkIdFlipped, 1, sizeof(int), assetFileHandle);
}

void WriteEntitySubChunk(XMLElement* entElem, const StringMap<int>& assetIds, FILE* assetFileHandle) {
	char subChunkId[] = "ENTT";
	fwrite(subChunkId, 1, 4, assetFileHandle);

	XMLElement* transElem = entElem->GetChild("Transform");
	ASSERT(transElem != nullptr);

	int id;
	XML_PARSE_SERIAL_ATTR(entElem, id, "id", Atoi);
	fwrite(&id, 1, sizeof(int), assetFileHandle);

	WriteTransformData(transElem, assetFileHandle);

	int meshId = -1;
	int matId = -1;

	if (XMLElement* matElem = entElem->GetChild("Material")) {
		String matSrc;
		if (matElem->attributes.LookUp("src", &matSrc)) {
			assetIds.LookUp(matSrc, &matId);
		}
	}

	if (XMLElement* meshElem = entElem->GetChild("Mesh")) {
		String meshSrc;
		if (meshElem->attributes.LookUp("src", &meshSrc)) {
			assetIds.LookUp(meshSrc, &meshId);
		}
	}

	int customComponentCount = 0;
	MemStream customComponents;
	for (int i = 0; i < entElem->childrenIds.count; i++) {
		XMLElement* childElem = entElem->doc->elements.GetById(entElem->childrenIds.data[i]);
		String name = childElem->name;
		int metaDataIndex = FindStructByName(name.string);

		if (metaDataIndex != -1) {
			customComponentCount++;

			MetaStruct* metaData = componentMetaData[metaDataIndex];

			customComponents.Write(metaDataIndex);

			//TODO: We could re-use this buffer, or at least avoid freeing it every time
			Component* buffer = (Component*)malloc(metaData->size);
			(componentXMLDeserializeFuncs[metaDataIndex])(buffer, childElem);
			(componentMemSerializeFuncs[metaDataIndex])(buffer, &customComponents);
			free(buffer);
		}
	}

	fwrite(&meshId, 1, sizeof(int), assetFileHandle);
	fwrite(&matId, 1, sizeof(int), assetFileHandle);
	fwrite(&customComponentCount, 1, sizeof(int), assetFileHandle);
	fwrite(customComponents.readHead, 1, customComponents.GetLength(), assetFileHandle);

	int subChunkIdFlipped = ~*(int*)subChunkId;
	fwrite(&subChunkIdFlipped, 1, sizeof(int), assetFileHandle);
}

void WriteLevelChunk(const char* levelFileName, const StringMap<int>& assetIds, int id, FILE* assetFileHandle){
	XMLDoc lvlDoc;
	XMLError err = ParseXMLStringFromFile(levelFileName, &lvlDoc);
	ASSERT_MSG(err == XMLE_NONE, "Error %d when parsing level file: '%s'", (int)err, levelFileName);

	XMLElement* rootElem = lvlDoc.elements.GetById(0);
	ASSERT(rootElem != nullptr);
	ASSERT(rootElem->name == "Scene");
	char chunkId[] = "BNLV";
	fwrite(chunkId, 1, 4, assetFileHandle);
	fwrite(&id, 1, 4, assetFileHandle);

	XMLElement* camElem = rootElem->GetChild("Camera");
	WriteCameraSubChunk(camElem, assetFileHandle);

	int entCount = 0;

	for (int i = 0; i < rootElem->childrenIds.count; i++) {
		XMLElement* childElem = lvlDoc.elements.GetById(rootElem->childrenIds.data[i]);
		if (childElem->name == "Entity") {
			entCount++;
		}
	}

	fwrite(&entCount, 1, 4, assetFileHandle);

	for (int i = 0; i < rootElem->childrenIds.count; i++) {
		XMLElement* childElem = lvlDoc.elements.GetById(rootElem->childrenIds.data[i]);
		if (childElem->name == "Entity") {
			WriteEntitySubChunk(childElem, assetIds, assetFileHandle);
		}
	}

	int chunkIdFlipped = ~*(int*)chunkId;
	fwrite(&chunkIdFlipped, 1, 4, assetFileHandle);
}

#define STB_TRUETYPE_IMPLEMENTATION
#include "../../ext/stb/stb_truetype.h"

void GlyphBlit(unsigned char* dst, int dstWidth, int dstHeight, int dstX, int dstY, unsigned char* src, int srcWidth, int srcHeight){
	for(int j = 0; j < srcHeight; j++){
		for(int i = 0; i < srcWidth; i++){
			int srcIdx = j * srcWidth + i;
			int dstIdx = (dstY+j)*dstWidth + (dstX+i);
			
			dst[dstIdx] = src[srcIdx];
		}
	}
}

void WriteCodePoints(CodepointInfo* codepointData, int codepointCount, FILE* assetFileHandle){
	fwrite(&codepointCount, 1, sizeof(int), assetFileHandle);
	
	fwrite(codepointData, sizeof(CodepointInfo), codepointCount, assetFileHandle);
}

const char* FindFilePathByName(const Vector<File*>& files, const char* fileName) {
	for (int i = 0; i < files.count; i++) {
		if (StrEqual(files.Get(i)->fileName, fileName)) {
			return files.Get(i)->fullName;
		}
	}

	return nullptr;
}

void WriteBitmapFontChunk(const char* fontFileName, const Vector<File*>& ttfFiles, int id, FILE* assetFileHandle){
	XMLDoc fontDoc;
	XMLError err = ParseXMLStringFromFile(fontFileName, &fontDoc);
	ASSERT(err == XMLE_NONE);

	String ttfFileShortName, fontSize;
	fontDoc.elements.vals[0].attributes.LookUp("size", &fontSize);
	fontDoc.elements.vals[0].attributes.LookUp("src", &ttfFileShortName);

	int scale = Atoi(fontSize.string);

	const char* ttfFilePath = FindFilePathByName(ttfFiles, ttfFileShortName.string);

	ASSERT(ttfFilePath != nullptr);

	int fontFileLength = 0;
	unsigned char* fontFileBuffer = ReadBinaryFile(ttfFilePath, &fontFileLength);

	stbtt_fontinfo font;
	stbtt_InitFont(&font, fontFileBuffer, stbtt_GetFontOffsetForIndex(fontFileBuffer,0));

	int fontBakeWidth = 1024;
	int fontBakeHeight = 1024;

	unsigned char* fontBakeBuffer = (unsigned char*)malloc(fontBakeWidth*fontBakeHeight);
	memset(fontBakeBuffer, 0, fontBakeWidth*fontBakeHeight);

	CodepointInfo codepoints[256] = {};
	int codepointCount = 0;

	int ascent,descent,lineGap;
	stbtt_GetFontVMetrics(&font, &ascent, &descent, &lineGap);

	float pixelScale = stbtt_ScaleForPixelHeight(&font, scale);

	int currX = 0;
	int currY = 0;
	int maxRowY = 0;
	for(unsigned char c = 32; c < 128; c++){
		int cW,cH;
		unsigned char* cBmp = stbtt_GetCodepointBitmap(&font, 0, pixelScale, c, &cW, &cH, 0,0);

		if (currX + cW >= fontBakeWidth){
			currX = 0;
			currY += (maxRowY+1);
			maxRowY = 0;
		}

		GlyphBlit(fontBakeBuffer, fontBakeWidth, fontBakeHeight, currX, currY, cBmp, cW, cH);

		int advanceWidth=0, leftSideBearing=0;
		stbtt_GetCodepointHMetrics(&font, c, &advanceWidth, &leftSideBearing);

		int x0,y0,x1,y1;
		stbtt_GetCodepointBitmapBox(&font, c, pixelScale, pixelScale, &x0,&y0,&x1,&y1);

		codepoints[codepointCount].codepoint = c;
		codepoints[codepointCount].x = currX;
		codepoints[codepointCount].y = currY;
		codepoints[codepointCount].w = cW;
		codepoints[codepointCount].h = cH;

		codepoints[codepointCount].xOffset = x0;
		codepoints[codepointCount].yOffset = y0;
		codepoints[codepointCount].xAdvance = pixelScale * advanceWidth;
		codepointCount++;

		currX += (cW+1);
		maxRowY = BNS_MAX(maxRowY, cH);

		free(cBmp);
	}

	char chunkId[] = "BNBF";
	fwrite(chunkId, 1, 4, assetFileHandle);
	fwrite(&id, 1, 4, assetFileHandle);

	fwrite(&fontBakeWidth, 1, sizeof(int), assetFileHandle);
	fwrite(&fontBakeHeight, 1, sizeof(int), assetFileHandle);
	fwrite(fontBakeBuffer, 1, fontBakeWidth*fontBakeHeight, assetFileHandle);

	WriteCodePoints(codepoints, codepointCount, assetFileHandle);

	free(fontBakeBuffer);
	free(fontFileBuffer);

	int chunkIdFlipped = ~*(int*)chunkId;
	fwrite(&chunkIdFlipped, 1, 4, assetFileHandle);
}

void WritePrefabChunk(const char* prefabFileName, const StringMap<int>& assetIds, int id, FILE* assetFileHandle) {
	XMLDoc prefabDoc;
	XMLError err = ParseXMLStringFromFile(prefabFileName, &prefabDoc);
	ASSERT_MSG(err == XMLE_NONE, "Error %d when parsing prefab file: '%s'", (int)err, prefabFileName);

	XMLElement* rootElem = prefabDoc.elements.GetById(0);
	ASSERT(rootElem != nullptr);
	ASSERT(rootElem->name == "Prefab");
	char chunkId[] = "BNPF";
	fwrite(chunkId, 1, 4, assetFileHandle);
	fwrite(&id, 1, 4, assetFileHandle);

	WriteEntitySubChunk(rootElem, assetIds, assetFileHandle);

	int chunkIdFlipped = ~*(int*)chunkId;
	fwrite(&chunkIdFlipped, 1, 4, assetFileHandle);
}

void WriteScriptChunk(const char* scriptFileName, const StringMap<int>& assetIds, int id, FILE* assetFileHandle) {
	BNVParser parser;
	parser.ParseFile(scriptFileName);

	BNVM vm;
	parser.AddByteCode(vm);

	MemStream stream;
	vm.WriteByteCodeToMemStream(&stream);

	char chunkId[] = "BNVM";
	fwrite(chunkId, 1, 4, assetFileHandle);
	fwrite(&id, 1, 4, assetFileHandle);

	fwrite(stream.readHead, 1, stream.GetLength(), assetFileHandle);

	int chunkIdFlipped = ~*(int*)chunkId;
	fwrite(&chunkIdFlipped, 1, 4, assetFileHandle);
}

void WriteUniFontChunk(const char* fontFileName, const Vector<File*>& ttfFiles, int id, FILE* assetFileHandle) {
	XMLDoc fontDoc;
	XMLError err = ParseXMLStringFromFile(fontFileName, &fontDoc);
	ASSERT(err == XMLE_NONE);

	XMLElement* root = &fontDoc.elements.vals[0];

	String fontSizeStr;
	root->attributes.LookUp("size", &fontSizeStr);
	int fontSize = Atoi(fontSizeStr.string);

	String cacheSizeStr;
	root->attributes.LookUp("cacheSize", &cacheSizeStr);
	int cacheSize = Atoi(cacheSizeStr.string);

	int fontCount = 0;
	while (root->GetChild("Source", fontCount)) {
		fontCount++;
	}

	ASSERT(fontCount > 0);

	char chunkId[] = "BNUF";
	fwrite(chunkId, 1, 4, assetFileHandle);
	fwrite(&id, 1, 4, assetFileHandle);
	fwrite(&fontSize, 1, 4, assetFileHandle);
	fwrite(&cacheSize, 1, 4, assetFileHandle);
	fwrite(&fontCount, 1, 4, assetFileHandle);

	for (int i = 0; i < fontCount; i++) {
		XMLElement* elem = root->GetChild("Source", i);

		String ttfFileShortName;
		elem->attributes.LookUp("path", &ttfFileShortName);
		const char* ttfFilePath = FindFilePathByName(ttfFiles, ttfFileShortName.string);

		int fontFileLength = 0;
		unsigned char* fontFileBuffer = ReadBinaryFile(ttfFilePath, &fontFileLength);

		fwrite(&fontFileLength, 1, 4, assetFileHandle);
		fwrite(fontFileBuffer, 1, fontFileLength, assetFileHandle);

		free(fontFileBuffer);
	}

	int chunkIdFlipped = ~*(int*)chunkId;
	fwrite(&chunkIdFlipped, 1, 4, assetFileHandle);
}

