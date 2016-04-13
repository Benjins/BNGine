#include "GfxResManager.h"
#include "../core/Scene.h"
#include "../assets/AssetFile.h"

#include "../../ext/CppUtils/filesys.h"
#include "../../ext/CppUtils/assert.h"
#include "../../ext/CppUtils/memstream.h"

GfxResManager::GfxResManager() 
	: programs(20), shaders(30), materials(15), meshes(30), drawCalls(40), textures(20){

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
		else if (memcmp(chunkId, "BNTX", 4) == 0) {
			Texture* texture = textures.AddWithId(assetId);
			LoadTextureFromChunk(fileBufferStream, texture);
		}
		else {
			ASSERT_WARN("Unkown chunk id: '%.*s'", 4, chunkId);
		}

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

	outMesh->positions.EnsureCapacity(posCount);
	stream.ReadArray<Vector3>(outMesh->positions.data, posCount);
	outMesh->positions.count = posCount;
	
	int uvCount = stream.Read<int>();
	
	outMesh->uvs.EnsureCapacity(uvCount);
	stream.ReadArray<Vector2>(outMesh->uvs.data, uvCount);
	outMesh->uvs.count = uvCount;

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
			outMesh->faces.data[i].posIndices[j] = indices.data[faceIndex];
			faceIndex++;

			if (flags & MCF_UVS) {
				outMesh->faces.data[i].uvIndices[j] = indices.data[faceIndex];
				faceIndex++;
			}
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

void GfxResManager::LoadTextureFromChunk(MemStream& stream, Texture* outTexture) {
	int width = stream.Read<int>();
	int height = stream.Read<int>();

	outTexture->width = width;
	outTexture->height = height;
	outTexture->texMem = (unsigned char*) malloc(width * height * 3);
	stream.ReadArray<unsigned char>(outTexture->texMem, width * height * 3);

	outTexture->textureType = GL_TEXTURE_2D;
	outTexture->UploadToGraphicsDevice();
}

void GfxResManager::Render() {
	glViewport(0, 0, (int)GlobalScene->cam.widthPixels, (int)GlobalScene->cam.heightPixels);

	ExecuteDrawCalls(drawCalls.vals, drawCalls.currentCount);
}