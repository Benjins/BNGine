#include "ResourceManager.h"
#include "../core/Scene.h"
#include "../assets/AssetFile.h"

#include "../../ext/CppUtils/filesys.h"
#include "../../ext/CppUtils/assert.h"
#include "../../ext/CppUtils/memstream.h"

ResourceManager::ResourceManager() 
	: programs(20), shaders(30), materials(15), meshes(30), drawCalls(40), textures(20), levels(10){

}

void ResourceManager::LoadAssetFile(const char* fileName) {
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

		char* assetName = (char*)malloc(nameLength+1);
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
		else if (memcmp(chunkId, "BNMT", 4) == 0) {
			Material* material = materials.AddWithId(assetId);
			LoadMaterialFromChunk(fileBufferStream, material);
		}
		else if (memcmp(chunkId, "BNLV", 4) == 0) {
			Level* level = levels.AddWithId(assetId);
			LoadLevelFromChunk(fileBufferStream, level);
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

void ResourceManager::LoadMeshFromChunk(MemStream& stream, Mesh* outMesh) {
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

void ResourceManager::LoadVShaderFromChunk(MemStream& stream, Shader* outShader) {
	outShader->CompileShader(stream.ReadStringInPlace(), GL_VERTEX_SHADER);
}

void ResourceManager::LoadFShaderFromChunk(MemStream& stream, Shader* outShader) {
	outShader->CompileShader(stream.ReadStringInPlace(), GL_FRAGMENT_SHADER);
}

void ResourceManager::LoadTextureFromChunk(MemStream& stream, Texture* outTexture) {
	int width = stream.Read<int>();
	int height = stream.Read<int>();

	outTexture->width = width;
	outTexture->height = height;
	outTexture->texMem = (unsigned char*) malloc(width * height * 3);
	stream.ReadArray<unsigned char>(outTexture->texMem, width * height * 3);

	outTexture->textureType = GL_TEXTURE_2D;
	outTexture->UploadToGraphicsDevice();
}

void ResourceManager::LoadMaterialFromChunk(MemStream& stream, Material* outMat) {
	int vShaderId = stream.Read<int>();
	int fShaderId = stream.Read<int>();

	Program* prog = nullptr;
	for (int i = 0; i < programs.currentCount; i++) {
		if (programs.vals[i].vertShader == vShaderId && programs.vals[i].fragShader == fShaderId) {
			prog = &programs.vals[i];
			break;
		}
	}

	if (prog == nullptr) {
		prog = programs.CreateAndAdd();
		prog->vertShader = vShaderId;
		prog->fragShader = fShaderId;
		prog->CompileProgram();
	}

	outMat->programId = prog->id;

	int uniformCount = stream.Read<int>();

	for (int i = 0; i < uniformCount; i++) {
		int strLength = stream.Read<int>();
		char* uniformName = stream.ReadStringInPlace();

		UniformType uniformType = stream.Read<UniformType>();
		if (uniformType == UT_TEXTURE2D) {
			int texId = stream.Read<uint32>();
			outMat->AddTexture(texId);

			outMat->SetIntUniform(uniformName, outMat->texCount - 1);
		}
		else if (uniformType == UT_INTEGER) {
			int val = stream.Read<int>();
			outMat->SetIntUniform(uniformName, val);
		}
		else if (uniformType == UT_FLOAT) {
			float val = stream.Read<float>();
			outMat->SetFloatUniform(uniformName, val);
		}
	}
}

void ResourceManager::LoadTransform(MemStream& stream, Transform* outTrans) {
	outTrans->id = stream.Read<int>();
	outTrans->parent= stream.Read<int>();
	outTrans->position = stream.Read<Vector3>();
	outTrans->rotation = stream.Read<Quaternion>();
	outTrans->scale	= stream.Read<Vector3>();
}

void ResourceManager::LoadLevelFromChunk(MemStream& stream, Level* outLevel) {
	{
		char camChunkId[4];
		stream.ReadArray<char>(camChunkId, 4);
		ASSERT(memcmp(camChunkId, "CMRA", 4) == 0);

		outLevel->cam.fov = stream.Read<float>();
		outLevel->cam.nearClip = stream.Read<float>();
		outLevel->cam.farClip = stream.Read<float>();

		Transform trans;
		LoadTransform(stream, &trans);
		outLevel->transforms.PushBack(trans);

		outLevel->cam.transform = trans.id;

		ASSERT(stream.Read<int>() == ~*(int*)camChunkId);
	}

	int entCount = stream.Read<int>();

	for (int i = 0; i < entCount; i++) {
		char enttChunkId[4];
		stream.ReadArray<char>(enttChunkId, 4);
		ASSERT(memcmp(enttChunkId, "ENTT", 4) == 0);

		Entity ent;
		ent.id = stream.Read<int>();

		Transform entTrans;
		LoadTransform(stream, &entTrans);
		entTrans.entity = ent.id;

		outLevel->transforms.PushBack(entTrans);

		ent.transform = entTrans.id;

		outLevel->entities.PushBack(ent);

		outLevel->meshIds.PushBack(stream.Read<int>());
		outLevel->matIds.PushBack(stream.Read<int>());

		ASSERT(stream.Read<int>() == ~*(int*)enttChunkId);
	}
}

void ResourceManager::Render() {
	glViewport(0, 0, (int)GlobalScene->cam.widthPixels, (int)GlobalScene->cam.heightPixels);

	ExecuteDrawCalls(drawCalls.vals, drawCalls.currentCount);
}
