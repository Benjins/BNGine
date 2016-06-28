#include "ResourceManager.h"
#include "../core/Scene.h"
#include "../assets/AssetFile.h"
#include "../metagen/ComponentMeta.h"
#include "../metagen/MetaStruct.h"
#include "../util/Serialization.h"
#include "../util/LevelLoading.h"

#include "../../ext/CppUtils/filesys.h"
#include "../../ext/CppUtils/assert.h"
#include "../../ext/CppUtils/memstream.h"
#include "../../ext/CppUtils/xml.h"

ResourceManager::ResourceManager() 
	: programs(20), shaders(30), materials(15), meshes(30), drawCalls(40), textures(20), levels(10){

}

void ResourceManager::Reset() {
	assetIdMap = StringMap<int>();

	fonts.Reset();
	levels.Reset();
	materials.Reset();
	meshes.Reset();
	programs.Reset();
	shaders.Reset();
	textures.Reset();
}

void ResourceManager::LoadAssetFile(const char* fileName) {
	typedef unsigned char byte;

	Reset();

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
		else if (memcmp(chunkId, "BNBF", 4) == 0) {
			BitmapFont* font = fonts.AddWithId(assetId);
			LoadBitmapFontFromChunk(fileBufferStream, font);
		}
		else if (memcmp(chunkId, "BNPF", 4) == 0) {
			Prefab* prefab = prefabs.AddWithId(assetId);
			LoadPrefabFromChunk(fileBufferStream, prefab);
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
	
	free(outTexture->texMem);
	outTexture->texMem = nullptr;
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

		int customComponentCount = stream.Read<int>();
		for (int j = 0; j < customComponentCount; j++) {
			int id = stream.Read<int>();
			ASSERT(id >= 0 && id < CCT_Count);
			Component* toAdd = (addComponentToLevelFuncs[id])(outLevel);
			(componentMemDeserializeFuncs[id])(toAdd, &stream);
			toAdd->entity = ent.id;
		}

		ASSERT(stream.Read<int>() == ~*(int*)enttChunkId);
	}
}

void ResourceManager::LoadBitmapFontFromChunk(MemStream& stream, BitmapFont* outFont){
	int bakeWidth = stream.Read<int>();
	int bakeHeight = stream.Read<int>();
	
	unsigned char* bakeTex = (unsigned char*)malloc(bakeWidth*bakeHeight);
	stream.ReadArray<unsigned char>(bakeTex, bakeWidth*bakeHeight);
	
	Texture* fontTex = textures.CreateAndAdd();
	fontTex->width = bakeWidth;
	fontTex->height = bakeHeight;
	fontTex->texMem = bakeTex;

	fontTex->textureType = GL_TEXTURE_2D;
	fontTex->internalColourFormat = GL_RED;
	fontTex->externalColourFormat = GL_RED;
	fontTex->UploadToGraphicsDevice();
	
	outFont->textureId = fontTex->id;
	
	int codepointCount = stream.Read<int>();
	
	outFont->codepointListing.EnsureCapacity(codepointCount);
	outFont->codepointListing.count = codepointCount;
	stream.ReadArray<CodepointInfo>(outFont->codepointListing.data, codepointCount); 
}

void ResourceManager::LoadPrefabFromChunk(MemStream& stream, Prefab* outPrefab) {
	char enttChunkId[4];
	stream.ReadArray<char>(enttChunkId, 4);
	ASSERT(memcmp(enttChunkId, "ENTT", 4) == 0);

	// We don't care aobut the entity id that was serialised, we have the prefab's id already
	stream.Read<int>();

	LoadTransform(stream, &outPrefab->transform);

	outPrefab->meshId = stream.Read<int>();
	outPrefab->matId = stream.Read<int>();

	int customComponentCount = stream.Read<int>();
	for (int j = 0; j < customComponentCount; j++) {
		int id = stream.Read<int>();
		ASSERT(id >= 0 && id < CCT_Count);

		outPrefab->customComponents.Write(id);

		int size = componentMetaData[id]->size;
		Component* comp = (Component*)malloc(size);

		(componentMemDeserializeFuncs[id])(comp, &stream);
		(componentMemSerializeFuncs[id])(comp, &outPrefab->customComponents);

		free(comp);
	}

	ASSERT(stream.Read<int>() == ~*(int*)enttChunkId);

}

void XMLSerializeTransform(XMLElement* elem, const Transform* trans) {
	elem->name = STATIC_TO_SUBSTRING("Transform");
	//<Transform id='3' pos='0,0,0' rotation='1,0,0,0' scale='1,1,1' parent='2'/>
	elem->attributes.Insert("id", Itoa(trans->id));
	elem->attributes.Insert("pos", EncodeVector3(trans->position));
	elem->attributes.Insert("rotation", EncodeQuaternion(trans->rotation));
	elem->attributes.Insert("scale", EncodeVector3(trans->scale));
	elem->attributes.Insert("parent", Itoa(trans->parent));
}

void ResourceManager::SaveLevelToFile(const Level* lvl, const char* fileName) {
	XMLDoc doc;
	XMLElement* rootElem = doc.AddElement();
	rootElem->name = STATIC_TO_SUBSTRING("Scene");
	uint32 rootId = rootElem->id;

	XMLElement* cameraElem = doc.AddElement();
	uint32 camElemId = cameraElem->id;
	cameraElem->name = STATIC_TO_SUBSTRING("Camera");
	cameraElem->attributes.Insert("fov", Itoa(lvl->cam.fov));
	cameraElem->attributes.Insert("nearClip", Ftoa(lvl->cam.nearClip));
	cameraElem->attributes.Insert("farClip", Ftoa(lvl->cam.farClip));

	doc.elements.GetById(rootId)->childrenIds.PushBack(cameraElem->id);

	Transform* camTrans = GlobalScene->transforms.GetById(lvl->cam.transform);
	ASSERT(camTrans != nullptr);

	XMLElement* camTransElem = doc.AddElement();
	XMLSerializeTransform(camTransElem, camTrans);

	// It may have re-allocated.  Maybe I didn't think this through...
	doc.elements.GetById(camElemId)->childrenIds.PushBack(camTransElem->id);

	for (int i = 0; i < lvl->entities.count; i++) {
		Entity* ent = &lvl->entities.data[i];

		XMLElement* entElem = doc.AddElement();
		uint32 entElemId = entElem->id;

		entElem->name = STATIC_TO_SUBSTRING("Entity");
		entElem->attributes.Insert("id", Itoa(ent->id));

		doc.elements.GetById(rootId)->childrenIds.PushBack(entElem->id);

		Transform* entTrans = GlobalScene->transforms.GetById(ent->transform);

		XMLElement* entTransElem = doc.AddElement();
		XMLSerializeTransform(entTransElem, entTrans);

		doc.elements.GetById(entElemId)->childrenIds.PushBack(entTransElem->id);

		for (int ct = 0; ct < CCT_Count; ct++) {
			MetaStruct* ms = componentMetaData[ct];

			Component* compBase = getComponentLevelArrayFuncs[ct](lvl);
			int compCount = getComponentLevelCountFuncs[ct](lvl);

			unsigned char* compCursor = (unsigned char*)compBase;

			for (int j = 0; j < compCount; j++) {
				Component* currComp = (Component*)compCursor;

				if (currComp->entity == ent->id) {
					XMLElement* compElem = doc.AddElement();
					componentXMLSerializeFuncs[ct](currComp, compElem);

					doc.elements.GetById(entElemId)->childrenIds.PushBack(compElem->id);
				}

				compCursor += ms->size;
			}
		}

		if (Material* mat = GlobalScene->res.materials.GetById(lvl->matIds.Get(i))) {
			XMLElement* matElem = doc.AddElement();
			matElem->name = STATIC_TO_SUBSTRING("Material");
			matElem->attributes.Insert("src", FindFileNameByIdAndExtension("mat", mat->id));
			doc.elements.GetById(entElemId)->childrenIds.PushBack(matElem->id);
		}

		if (Mesh* mesh = GlobalScene->res.meshes.GetById(lvl->meshIds.Get(i))) {
			XMLElement* meshElem = doc.AddElement();
			meshElem->name = STATIC_TO_SUBSTRING("Mesh");
			meshElem->attributes.Insert("src", FindFileNameByIdAndExtension("obj", mesh->id));
			doc.elements.GetById(entElemId)->childrenIds.PushBack(meshElem->id);
		}
	}

	SaveXMLDocToFile(&doc, fileName);
}

void ResourceManager::Render() {
	ExecuteDrawCalls(drawCalls.vals, drawCalls.currentCount);
}

String ResourceManager::FindFileNameByIdAndExtension(const char* ext, uint32 id) {
	for (int i = 0; i < assetIdMap.count; i++) {
		if (assetIdMap.values[i] == id) {
			//Check ext.
			const char* fileName = assetIdMap.names[i].string;
			int fileNameLength = assetIdMap.names[i].GetLength();
			const char* fileNameExt = fileName + (fileNameLength - 1);
			
			while (fileNameExt > fileName && *fileNameExt != '.'){
				fileNameExt--;
			}

			// It's pointing to the last dot, and should point just after.
			fileNameExt++;

			if (StrEqual(fileNameExt, ext)) {
				return assetIdMap.names[i];
			}
		}
	}

	return String("");
}

void ResourceManager::FindFileNamesByExtension(const char* ext, Vector<String>* outFiles) {
	for (int i = 0; i < assetIdMap.count; i++) {
		const char* fileName = assetIdMap.names[i].string;
		int fileNameLength = assetIdMap.names[i].GetLength();
		const char* fileNameExt = fileName + (fileNameLength - 1);

		while (fileNameExt > fileName && *fileNameExt != '.') {
			fileNameExt--;
		}

		// It's pointing to the last dot, and should point just after.
		fileNameExt++;

		if (StrEqual(fileNameExt, ext)) {
			outFiles->PushBack(assetIdMap.names[i]);
		}
	}
}
