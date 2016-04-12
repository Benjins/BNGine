#include "GfxResManager.h"
#include "../core/Scene.h"
#include "../assets/AssetFile.h"

#include "../../ext/CppUtils/filesys.h"
#include "../../ext/CppUtils/assert.h"

GfxResManager::GfxResManager() 
	: programs(20), shaders(30), materials(15), meshes(30), drawCalls(40){

}

void GfxResManager::LoadAssetFile(const char* fileName) {
	typedef unsigned char byte;

	int fileLength = 0;
	byte* assetFile = ReadBinaryFile(fileName, &fileLength);

	char fileIdx[] = "BNSA";

	ASSERT_MSG(assetFile != nullptr, "Could not load asset file '%s'", fileName);

	byte* fileCursor = assetFile;

	ASSERT(memcmp(fileCursor, fileIdx, 4) == 0);
	fileCursor += 4;
	int version = *(int*)fileCursor;
	fileCursor += 4;

	int assetNameCount = *(int*)fileCursor;
	fileCursor += 4;

	assetIdMap.EnsureCapacity(assetNameCount);
	for (int i = 0; i < assetNameCount; i++) {
		int nameLength = *(int*)fileCursor;
		fileCursor += 4;

		int assetId = *(int*)fileCursor;
		fileCursor += 4;

		char* assetName = (char*)malloc(nameLength);
		MemCpy(assetName, fileCursor, nameLength);
		assetName[nameLength] = '\0';

		assetIdMap.Insert(assetName, assetId);

		fileCursor += nameLength;

		free(assetName);
	}

	while (fileCursor - assetFile < fileLength - 4) {
		char* chunkId = (char*)fileCursor;
		fileCursor += 4;
		int assetId = *(int*)fileCursor;
		fileCursor += 4;
		int chunkLength = *(int*)fileCursor;
		fileCursor += 4;

		if (memcmp(chunkId, "BNMD", 4) == 0) {
			Mesh* mesh = meshes.AddWithId(assetId);
			LoadMeshFromChunk(fileCursor, chunkLength, mesh);
		}
		else if (memcmp(chunkId, "BNVS", 4) == 0) {
			Shader* shader = shaders.AddWithId(assetId);
			LoadVShaderFromChunk(fileCursor, chunkLength, shader);
		}
		else if (memcmp(chunkId, "BNFS", 4) == 0) {
			Shader* shader = shaders.AddWithId(assetId);
			LoadFShaderFromChunk(fileCursor, chunkLength, shader);
		}
		else {
			ASSERT_WARN("Unkown chunk id: '%.*s'", 4, chunkId);
		}

		fileCursor += chunkLength;

		int chunkFooter = *(int*)fileCursor;
		fileCursor += 4;
		int expectedChunkFooter = ~*(int*)chunkId;

		ASSERT(chunkFooter == expectedChunkFooter);
	}

	int chunkFooter = *(int*)fileCursor;
	int expectedChunkFooter = ~*(int*)fileIdx;
	ASSERT(chunkFooter == expectedChunkFooter);
}

void GfxResManager::LoadMeshFromChunk(unsigned char* meshChunk, int chunkLength, Mesh* outMesh) {
	unsigned char* meshCursor = meshChunk;
	int flags = *(int*)meshCursor;
	meshCursor += 4;

	int posCount = *(int*)meshCursor;
	meshCursor += 4;

	outMesh->vertices.EnsureCapacity(posCount);
	MemCpy(outMesh->vertices.data, meshCursor, posCount * sizeof(Vector3));
	outMesh->vertices.count = posCount;
	meshCursor += posCount * sizeof(Vector3);
	
	int uvCount = *(int*)meshCursor;
	meshCursor += 4;
	meshCursor += uvCount * sizeof(Vector2);

	int indexCount = *(int*)meshCursor;
	meshCursor += 4;

	Vector<int> indices;
	indices.EnsureCapacity(indexCount);
	MemCpy(indices.data, meshCursor, indexCount * sizeof(int));
	indices.count = indexCount;

	int skipCount = (flags & MCF_UVS) ? 2 : 1;
	int faceCount = indexCount / skipCount / 3;
	outMesh->faces.EnsureCapacity(faceCount);
	outMesh->faces.count = faceCount;

	int faceIndex = 0;
	for (int i = 0; i < faceCount; i++) {
		for (int j = 0; j < 3; j++) {
			outMesh->faces.data[i].indices[j] = indices.data[faceIndex];
			faceIndex += skipCount;
		}
	}

	outMesh->UploadToGfxDevice();
}

void GfxResManager::LoadVShaderFromChunk(unsigned char* shaderChunk, int chunkLength, Shader* outShader) {
	outShader->CompileShader((char*)shaderChunk, GL_VERTEX_SHADER);
}

void GfxResManager::LoadFShaderFromChunk(unsigned char* shaderChunk, int chunkLength, Shader* outShader) {
	outShader->CompileShader((char*)shaderChunk, GL_FRAGMENT_SHADER);
}

void GfxResManager::Render() {
	glViewport(0, 0, (int)GlobalScene->cam.widthPixels, (int)GlobalScene->cam.heightPixels);

	//TODO: Camera matrix

	ExecuteDrawCalls(drawCalls.vals, drawCalls.currentCount);
}