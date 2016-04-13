#include "GfxResManager.h"
#include "../core/Scene.h"
#include "../assets/AssetFile.h"

#include "../../ext/CppUtils/filesys.h"
#include "../../ext/CppUtils/assert.h"
#include "../../ext/CppUtils/memstream.h"

GfxResManager::GfxResManager() 
	: programs(20), shaders(30), materials(15), meshes(30), drawCalls(40){

}

void GfxResManager::LoadAssetFile(const char* fileName) {
	typedef unsigned char byte;

	char fileIdx[] = "BNSA";

	MemStream fileBufferStream;
	fileBufferStream.ReadInFromFile(fileName);

	ASSERT_MSG(fileBufferStream.readHead != nullptr, "Could not load asset file '%s'", fileName);

	char fileHead[4];
	fileBufferStream.ReadArray<char>(fileHead, 4);
	ASSERT(memcmp(fileHead, fileIdx, 4) == 0);

	int version = fileBufferStream.Read<int>();

	int assetNameCount = fileBufferStream.Read<int>();

	assetIdMap.EnsureCapacity(assetNameCount);
	for (int i = 0; i < assetNameCount; i++) {
		int nameLength = fileBufferStream.Read<int>();

		int assetId = fileBufferStream.Read<int>();

		char* assetName = (char*)malloc(nameLength);
		fileBufferStream.ReadArray<char>(assetName, nameLength);
		assetName[nameLength] = '\0';

		assetIdMap.Insert(assetName, assetId);

		free(assetName);
	}

	while (fileBufferStream.GetLength() > 4) {
		char chunkId[4];
		fileBufferStream.ReadArray<char>(chunkId, 4);

		int assetId = fileBufferStream.Read<int>();

		int chunkLength = fileBufferStream.Read<int>();

		void* oldReadHead = fileBufferStream.readHead;

		if (memcmp(chunkId, "BNMD", 4) == 0) {
			Mesh* mesh = meshes.AddWithId(assetId);
			LoadMeshFromChunk(fileBufferStream, mesh);
		}
		else if (memcmp(chunkId, "BNVS", 4) == 0) {
			Shader* shader = shaders.AddWithId(assetId);
			LoadVShaderFromChunk(fileBufferStream, shader);
		}
		else if (memcmp(chunkId, "BNFS", 4) == 0) {
			Shader* shader = shaders.AddWithId(assetId);
			LoadFShaderFromChunk(fileBufferStream, shader);
		}
		else {
			ASSERT_WARN("Unkown chunk id: '%.*s'", 4, chunkId);
		}

		ASSERT(VOID_PTR_DIST(fileBufferStream.readHead, oldReadHead) == chunkLength);

		int chunkFooter = fileBufferStream.Read<int>();
		int expectedChunkFooter = ~*(int*)chunkId;

		ASSERT(chunkFooter == expectedChunkFooter);
	}

	int chunkFooter = fileBufferStream.Read<int>();
	int expectedChunkFooter = ~*(int*)fileIdx;
	ASSERT(chunkFooter == expectedChunkFooter);
}

void GfxResManager::LoadMeshFromChunk(MemStream& stream, Mesh* outMesh) {
	int flags = stream.Read<int>();

	int posCount = stream.Read<int>();

	outMesh->vertices.EnsureCapacity(posCount);
	stream.ReadArray<Vertex>(outMesh->vertices.data, posCount);
	outMesh->vertices.count = posCount;
	
	int uvCount = stream.Read<int>();
	stream.readHead = VOID_PTR_ADD(stream.readHead, uvCount * sizeof(Vector2));

	int indexCount = stream.Read<int>();

	Vector<int> indices;
	indices.EnsureCapacity(indexCount);
	stream.ReadArray<int>(indices.data, indexCount);
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

void GfxResManager::LoadVShaderFromChunk(MemStream& stream, Shader* outShader) {
	outShader->CompileShader(stream.ReadStringInPlace(), GL_VERTEX_SHADER);
}

void GfxResManager::LoadFShaderFromChunk(MemStream& stream, Shader* outShader) {
	outShader->CompileShader(stream.ReadStringInPlace(), GL_FRAGMENT_SHADER);
}

void GfxResManager::Render() {
	glViewport(0, 0, (int)GlobalScene->cam.widthPixels, (int)GlobalScene->cam.heightPixels);

	//TODO: Camera matrix

	ExecuteDrawCalls(drawCalls.vals, drawCalls.currentCount);
}