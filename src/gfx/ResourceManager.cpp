#include "ResourceManager.h"

#include "GLExtInit.h"

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

#include "../../ext/3dbasics/Vector4.h"

#include "../../gen/ScriptGen.h"

ResourceManager::ResourceManager() 
	: shaders(30), programs(20), materials(15), meshes(30), textures(50), drawCalls(40), levels(10){

}

void ResourceManager::Reset() {
	assetIdMap = StringMap<int>();

	fonts.Reset();
	uniFonts.Reset();
	levels.Reset();
	materials.Reset();
	meshes.Reset();
	programs.Reset();
	shaders.Reset();
	textures.Reset();
	scripts.Reset();
	prefabs.Reset();
	anims.Reset();
	armatures.Reset();
	cubeMaps.Reset();
}

void ResourceManager::LoadAssetFile(const char* fileName) {
	Reset();

	char fileIdx[] = "BNSA";

	MemStream fileBufferStream;
	fileBufferStream.ReadInFromFile(fileName);

	ASSERT_MSG(fileBufferStream.readHead != nullptr, "Could not load asset file '%s'", fileName);

	char fileHead[4];
	fileBufferStream.ReadArray<char>(fileHead, 4);
	ASSERT(memcmp(fileHead, fileIdx, 4) == 0);

	int version = fileBufferStream.Read<int>();
	// TODO: Check this
	BNS_UNUSED(version);

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

		if (memcmp(chunkId, "BNMD", 4) == 0) {
			Mesh* mesh = meshes.AddWithId(assetId);
			LoadMeshFromChunk(fileBufferStream, mesh);
		}
		else if (memcmp(chunkId, "BNAM", 4) == 0) {
			Armature* arm = armatures.AddWithId(assetId);
			LoadArmatureFromChunk(fileBufferStream, arm);
		}
		else if (memcmp(chunkId, "BNAT", 4) == 0) {
			AnimationTrack* track = anims.AddWithId(assetId);
			LoadAnimationTrackFromChunk(fileBufferStream, track);
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
		else if (memcmp(chunkId, "BNUF", 4) == 0) {
			UniFont* font = uniFonts.AddWithId(assetId);
			LoadUniFontFromChunk(fileBufferStream, font);
		}
		else if (memcmp(chunkId, "BNCM", 4) == 0) {
			CubeMap* cubeMap = cubeMaps.AddWithId(assetId);
			LoadCubeMapFromChunk(fileBufferStream, cubeMap);
		}
		else if (memcmp(chunkId, "BNPF", 4) == 0) {
			Prefab* prefab = prefabs.AddWithId(assetId);
			LoadPrefabFromChunk(fileBufferStream, prefab);
		}
		else if (memcmp(chunkId, "BNVM", 4) == 0) {
			ScriptObject* script = scripts.AddWithId(assetId);
			LoadScriptObjectFromChunk(fileBufferStream, script);
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
	outMesh->armatureId = IDHandle<Armature>(stream.Read<int>());

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

void ResourceManager::LoadArmatureFromChunk(MemStream& stream, Armature* outArmature) {
	int meshId = stream.Read<int>();
	int boneCount = stream.Read<int>();

	outArmature->modelId = IDHandle<Mesh>(meshId);

	for (int i = 0; i < boneCount; i++) {
		BoneTransform* bt = outArmature->AddBone();
		stream.ReadArray<char>(bt->name, MAX_BONE_NAME_LENGTH);
		stream.ReadArray<Mat4x4>(&outArmature->inverseBindPoses[i], 1);
		bt->parent = stream.Read<int>();

		outArmature->boneTrackData[i].posTrack = stream.Read<int>();
		outArmature->boneTrackData[i].rotTrack = stream.Read<int>();
		outArmature->boneTrackData[i].scaleTrack = stream.Read<int>();

		bt->pos = stream.Read<Vector3>();
		bt->rot = stream.Read<Quaternion>();
		bt->scale = stream.Read<Vector3>();
	}

	ASSERT(outArmature->boneCount == boneCount);

	int vertCount = stream.Read<int>();
	Vector<float> boneWeights(vertCount * MAX_BONES_PER_VERTEX);
	Vector<float> boneIndices(vertCount * MAX_BONES_PER_VERTEX);
	for (int i = 0; i < vertCount; i++) {
		int boneCount = stream.Read<int>();
		ASSERT(boneCount <= MAX_BONES_PER_VERTEX);
		
		float mag = 0.0f;
		for (int j = 0; j < boneCount; j++) {
			boneIndices.PushBack(stream.Read<int>());
			float weight = stream.Read<float>();
			boneWeights.PushBack(weight);
			mag += (weight*weight);
		}

		ASSERT(BNS_ABS(mag - 1) < 0.001f);

		//Pad to MAX_BONES_PER_VERTEX bones
		for (int j = boneCount; j < MAX_BONES_PER_VERTEX; j++) {
			boneIndices.PushBack(0);
			boneWeights.PushBack(0.0f);
		}
	}

	ASSERT(boneIndices.count == vertCount * MAX_BONES_PER_VERTEX);
	ASSERT(boneWeights.count == vertCount * MAX_BONES_PER_VERTEX);

	Mesh* mesh = meshes.GetByIdNum(meshId);
	outArmature->boneIndices.EnsureCapacity(mesh->faces.count * 3 * MAX_BONES_PER_VERTEX);
	outArmature->boneWeights.EnsureCapacity(mesh->faces.count * 3 * MAX_BONES_PER_VERTEX);
	for (int i = 0; i < mesh->faces.count; i++) {
		for (int j = 0; j < 3; j++) {
			int idx = mesh->faces.data[i].posIndices[j];
			ASSERT(idx < vertCount);
			for (int k = 0; k < MAX_BONES_PER_VERTEX; k++) {
				outArmature->boneIndices.PushBack(boneIndices.data[idx * MAX_BONES_PER_VERTEX + k]);
				outArmature->boneWeights.PushBack(boneWeights.data[idx * MAX_BONES_PER_VERTEX + k]);
			}
		}
	}

	outArmature->UploadDataToGfxDevice();
}

void ResourceManager::LoadAnimationTrackFromChunk(MemStream& stream, AnimationTrack* outTrack) {
	outTrack->type = stream.Read<AnimationType>();
	int keyCount = stream.Read<int>();

	outTrack->times.EnsureCapacity(keyCount);
	stream.ReadArray<float>(outTrack->times.data, keyCount);
	outTrack->times.count = keyCount;

	int dataSize = keyCount * animTypeKeySize[outTrack->type] / 4;
	outTrack->data.EnsureCapacity(dataSize);
	stream.ReadArray<float>(outTrack->data.data, dataSize);
	outTrack->data.count = dataSize;
}

void ResourceManager::LoadCubeMapFromChunk(MemStream& stream, CubeMap* outCubeMap) {
	stream.ReadArray<int>((int*)outCubeMap->textures, 6);

	outCubeMap->UploadToGraphicsDevice();
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
	uint32 vShaderId = stream.Read<uint32>();
	uint32 fShaderId = stream.Read<uint32>();

	Program* prog = nullptr;
	for (int i = 0; i < programs.currentCount; i++) {
		if (programs.vals[i].vertShader.id == vShaderId && programs.vals[i].fragShader.id == fShaderId) {
			prog = &programs.vals[i];
			break;
		}
	}

	if (prog == nullptr) {
		prog = programs.CreateAndAdd();
		prog->vertShader = IDHandle<Shader>(vShaderId);
		prog->fragShader = IDHandle<Shader>(fShaderId);
		prog->CompileProgram();
	}

	outMat->programId = IDHandle<Program>(prog->id);

	//-------------------------
	// GL Program Binary debug code
	/*
	{
		int progBinLength = 0;
		glGetProgramiv(prog->programObj, GL_PROGRAM_BINARY_LENGTH, &progBinLength);

		GLsizei actualSize = 0;
		GLenum binaryFormat = 0;
		void* progBinBuffer = malloc(progBinLength);
		glGetProgramBinary(prog->programObj, progBinLength, &actualSize, &binaryFormat, progBinBuffer);

		String matFileName = FindFileNameByIdAndExtension("mat", outMat->id);
		StringStackBuffer<256> binFileName("%s.bin", matFileName.string);

		FILE* progBin = fopen(binFileName.buffer, "wb");
		fwrite(progBinBuffer, progBinLength, 1, progBin);
		fclose(progBin);

		free(progBinBuffer);
	}
	*/
	//--------------------------

	int uniformCount = stream.Read<int>();

	int cubeMapId = -1;

	char* cubeMapName = nullptr;

	for (int i = 0; i < uniformCount; i++) {
		int strLength = stream.Read<int>();
		BNS_UNUSED(strLength);
		char* uniformName = stream.ReadStringInPlace();

		UniformType uniformType = stream.Read<UniformType>();
		if (uniformType == UT_TEXTURE2D) {
			uint32 texId = stream.Read<uint32>();
			outMat->AddTexture(IDHandle<Texture>(texId));

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
		else if (uniformType == UT_VEC4) {
			Vector4 val = stream.Read<Vector4>();
			outMat->SetVector4Uniform(uniformName, val);
		}
		else if (uniformType == UT_CUBEMAP) {
			uint32 texId = stream.Read<uint32>();
			outMat->cubeMap = IDHandle<CubeMap>(texId);
			cubeMapName = uniformName;
		}
	}

	if (cubeMapName != nullptr) {
		outMat->SetIntUniform(cubeMapName, outMat->texCount);
	}
}

void ResourceManager::LoadTransform(MemStream& stream, Transform* outTrans) {
	outTrans->id = stream.Read<int>();
	outTrans->parent= IDHandle<Transform>(stream.Read<int>());
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

		outLevel->cam.transform = IDHandle<Transform>(trans.id);

		int endId = stream.Read<int>();
		ASSERT(endId == ~*(int*)camChunkId);
	}

	int entCount = stream.Read<int>();

	for (int i = 0; i < entCount; i++) {
		char enttChunkId[4];
		stream.ReadArray<char>(enttChunkId, 4);
		if (memcmp(enttChunkId, "ENTT", 4) == 0) {

			Entity ent;
			ent.id = stream.Read<int>();

			Transform entTrans;
			LoadTransform(stream, &entTrans);
			entTrans.entity = IDHandle<Entity>(ent.id);

			outLevel->transforms.PushBack(entTrans);

			ent.transform = IDHandle<Transform>(entTrans.id);

			outLevel->entities.PushBack(ent);

			outLevel->meshIds.PushBack(stream.Read<int>());
			outLevel->matIds.PushBack(stream.Read<int>());

			int customComponentCount = stream.Read<int>();
			for (int j = 0; j < customComponentCount; j++) {
				int id = stream.Read<int>();
				ASSERT(id >= 0 && id < CCT_Count);
				Component* toAdd = (addComponentToLevelFuncs[id])(outLevel);
				toAdd->id = (getComponentLevelCountFuncs[id])(outLevel) - 1;
				(componentMemDeserializeFuncs[id])(toAdd, &stream);
				toAdd->entity = IDHandle<Entity>(ent.id);
			}
		}
		else if (memcmp(enttChunkId, "ENPI", 4) == 0) {
			int entId = stream.Read<int>();

			Transform entTrans;
			LoadTransform(stream, &entTrans);

			int prefabId = stream.Read<int>();

			PrefabInstance inst;
			inst.pos = entTrans.position;
			inst.parentTransform = entTrans.parent.id;
			inst.rot = entTrans.rotation;
			inst.prefabId = IDHandle<Prefab>(prefabId);
			inst.instanceId = entId;
			inst.instanceTransformId = entTrans.id;

			outLevel->prefabInsts.PushBack(inst);
		}
		else {
			ASSERT_WARN("Unknown subchunk id: '%.*s'", 4, enttChunkId)
		}

		int endId = stream.Read<int>();
		ASSERT(endId == ~*(int*)enttChunkId);
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
	
	outFont->textureId = IDHandle<Texture>(fontTex->id);
	
	int codepointCount = stream.Read<int>();
	
	outFont->codepointListing.EnsureCapacity(codepointCount);
	outFont->codepointListing.count = codepointCount;
	stream.ReadArray<CodepointInfo>(outFont->codepointListing.data, codepointCount); 
}

void ResourceManager::LoadUniFontFromChunk(MemStream& stream, UniFont* outFont) {
	outFont->fontScale = stream.Read<int>();
	int cacheSize = stream.Read<int>();
	int fontCount = stream.Read<int>();

	for (int i = 0; i < fontCount; i++) {
		int low  = stream.Read<int>();
		int high = stream.Read<int>();

		int fontBufferSize = stream.Read<int>();
		unsigned char* buffer = (unsigned char*)malloc(fontBufferSize);
		stream.ReadArray<unsigned char>(buffer, fontBufferSize);
		outFont->AddFont(buffer, fontBufferSize, low, high);
	}

	Texture* tex = textures.CreateAndAdd();
	tex->width = cacheSize;
	tex->height = cacheSize;
	// calloc is for chumps.
	tex->texMem = (unsigned char*)malloc(cacheSize * cacheSize);
	memset(tex->texMem, 0, cacheSize*cacheSize);

	tex->textureType = GL_TEXTURE_2D;
	tex->internalColourFormat = GL_RED;
	tex->externalColourFormat = GL_RED;

	outFont->textureId = IDHandle<Texture>(tex->id);
}

void ResourceManager::LoadScriptObjectFromChunk(MemStream& stream, ScriptObject* outScript) {
	outScript->vm.ReadByteCodeFromMemStream(&stream);
	RegisterBNGineVM(&outScript->vm);
}

void ResourceManager::LoadPrefabFromChunk(MemStream& stream, Prefab* outPrefab) {
	char enttChunkId[4];
	stream.ReadArray<char>(enttChunkId, 4);
	ASSERT(memcmp(enttChunkId, "ENTT", 4) == 0);

	// We don't care aobut the entity id that was serialised, we have the prefab's id already
	stream.Read<int>();

	LoadTransform(stream, &outPrefab->transform);

	outPrefab->meshId = IDHandle<Mesh>(stream.Read<int>());
	outPrefab->matId  = IDHandle<Material>(stream.Read<int>());

	int customComponentCount = stream.Read<int>();
	for (int j = 0; j < customComponentCount; j++) {
		int id = stream.Read<int>();
		ASSERT(id >= 0 && id < CCT_Count);

		outPrefab->customComponents.Write(id);

		Component* comp = componentSerializeBuffer[id];

		(componentResetFuncs[id])(comp);
		(componentMemDeserializeFuncs[id])(comp, &stream);
		(componentMemSerializeFuncs[id])(comp, &outPrefab->customComponents);
	}

	int endId = stream.Read<int>();
	ASSERT(endId == ~*(int*)enttChunkId);

}

void XMLSerializeTransform(XMLElement* elem, const Transform* trans) {
	elem->name = STATIC_TO_SUBSTRING("Transform");
	//<Transform id='3' pos='0,0,0' rotation='1,0,0,0' scale='1,1,1' parent='2'/>
	elem->attributes.Insert("id", Itoa(trans->id));
	elem->attributes.Insert("pos", EncodeVector3(trans->position));
	elem->attributes.Insert("rotation", EncodeQuaternion(trans->rotation));
	elem->attributes.Insert("scale", EncodeVector3(trans->scale));
	elem->attributes.Insert("parent", Itoa(trans->parent.id));
}

void ResourceManager::SavePrefabToFile(Prefab* prefab, const char* fileName) {
	XMLDoc doc;
	XMLElement* rootElem = doc.AddElement();
	rootElem->name = STATIC_TO_SUBSTRING("Prefab");
	rootElem->attributes.Insert("id", Itoa(prefab->id));
	IDHandle<XMLElement> rootId = IDHandle<XMLElement>(rootElem->id);

	XMLElement* transElem = doc.AddElement();
	XMLSerializeTransform(transElem, &prefab->transform);

	// It may have re-allocated.  Maybe I didn't think this through...
	doc.elements.GetById(rootId)->childrenIds.PushBack(IDHandle<XMLElement>(transElem->id));

	if (prefab->matId.id != 0xFFFFFFFF) {
		if (Material* mat = GlobalScene->res.materials.GetById(prefab->matId)) {
			XMLElement* matElem = doc.AddElement();
			matElem->name = STATIC_TO_SUBSTRING("Material");
			matElem->attributes.Insert("src", FindFileNameByIdAndExtension("mat", mat->id));
			// TODO: Easier way to add children elements
			doc.elements.GetById(rootId)->childrenIds.PushBack(IDHandle<XMLElement>(matElem->id));
		}
	}

	if (prefab->meshId.id != 0xFFFFFFFF) {
		if (Mesh* mesh = GlobalScene->res.meshes.GetById(prefab->meshId)) {
			XMLElement* meshElem = doc.AddElement();
			meshElem->name = STATIC_TO_SUBSTRING("Mesh");
			String meshName = FindFileNameByIdAndExtension("obj", mesh->id);
			if (meshName == "") {
				meshName = FindFileNameByIdAndExtension("dae", mesh->id);
			}
			meshElem->attributes.Insert("src", meshName);
			doc.elements.GetById(rootId)->childrenIds.PushBack(IDHandle<XMLElement>(meshElem->id));
		}
	}

	while (prefab->customComponents.GetLength() > 0) {
		int ct = prefab->customComponents.Read<int>();

		Component* buffer = componentSerializeBuffer[ct];
		componentMemDeserializeFuncs[ct](buffer, &prefab->customComponents);
		
		XMLElement* compElem = doc.AddElement();
		componentXMLSerializeFuncs[ct](buffer, compElem);
		doc.elements.GetById(rootId)->childrenIds.PushBack(IDHandle<XMLElement>(compElem->id));
	}

	prefab->customComponents.readHead = prefab->customComponents.base;

	SaveXMLDocToFile(&doc, fileName);
}

void ResourceManager::SaveLevelToFile(const Level* lvl, const char* fileName) {
	XMLDoc doc;
	XMLElement* rootElem = doc.AddElement();
	rootElem->name = STATIC_TO_SUBSTRING("Scene");
	IDHandle<XMLElement> rootId = IDHandle<XMLElement>(rootElem->id);

	XMLElement* cameraElem = doc.AddElement();
	IDHandle<XMLElement> camElemId = IDHandle<XMLElement>(cameraElem->id);
	cameraElem->name = STATIC_TO_SUBSTRING("Camera");
	cameraElem->attributes.Insert("fov", Itoa(lvl->cam.fov));
	cameraElem->attributes.Insert("nearClip", Ftoa(lvl->cam.nearClip));
	cameraElem->attributes.Insert("farClip", Ftoa(lvl->cam.farClip));

	doc.elements.GetById(rootId)->childrenIds.PushBack(IDHandle<XMLElement>(cameraElem->id));

	Transform* camTrans = GlobalScene->transforms.GetById(lvl->cam.transform);
	ASSERT(camTrans != nullptr);

	XMLElement* camTransElem = doc.AddElement();
	XMLSerializeTransform(camTransElem, camTrans);

	// It may have re-allocated.  Maybe I didn't think this through...
	doc.elements.GetById(camElemId)->childrenIds.PushBack(IDHandle<XMLElement>(camTransElem->id));

	for (int i = 0; i < lvl->entities.count; i++) {
		Entity* ent = &lvl->entities.data[i];

		XMLElement* entElem = doc.AddElement();
		IDHandle<XMLElement> entElemId = IDHandle<XMLElement>(entElem->id);

		entElem->name = STATIC_TO_SUBSTRING("Entity");
		entElem->attributes.Insert("id", Itoa(ent->id));

		doc.elements.GetById(rootId)->childrenIds.PushBack(IDHandle<XMLElement>(entElem->id));

		Transform* entTrans = GlobalScene->transforms.GetById(ent->transform);

		XMLElement* entTransElem = doc.AddElement();
		XMLSerializeTransform(entTransElem, entTrans);

		doc.elements.GetById(entElemId)->childrenIds.PushBack(IDHandle<XMLElement>(entTransElem->id));

		for (int ct = 0; ct < CCT_Count; ct++) {
			MetaStruct* ms = componentMetaData[ct];

			Component* compBase = getComponentLevelArrayFuncs[ct](lvl);
			int compCount = getComponentLevelCountFuncs[ct](lvl);

			unsigned char* compCursor = (unsigned char*)compBase;

			for (int j = 0; j < compCount; j++) {
				Component* currComp = (Component*)compCursor;

				if (!(currComp->flags & CF_RuntimeOnly) && currComp->entity.id == ent->id) {
					XMLElement* compElem = doc.AddElement();
					componentXMLSerializeFuncs[ct](currComp, compElem);

					doc.elements.GetById(entElemId)->childrenIds.PushBack(IDHandle<XMLElement>(compElem->id));
				}

				compCursor += ms->size;
			}
		}

		if (Material* mat = GlobalScene->res.materials.GetByIdNum(lvl->matIds.Get(i))) {
			XMLElement* matElem = doc.AddElement();
			matElem->name = STATIC_TO_SUBSTRING("Material");
			matElem->attributes.Insert("src", FindFileNameByIdAndExtension("mat", mat->id));
			doc.elements.GetById(entElemId)->childrenIds.PushBack(IDHandle<XMLElement>(matElem->id));
		}

		if (Mesh* mesh = GlobalScene->res.meshes.GetByIdNum(lvl->meshIds.Get(i))) {
			XMLElement* meshElem = doc.AddElement();
			meshElem->name = STATIC_TO_SUBSTRING("Mesh");
			String meshName = FindFileNameByIdAndExtension("obj", mesh->id);
			if (meshName == "") {
				meshName = FindFileNameByIdAndExtension("dae", mesh->id);
			}
			meshElem->attributes.Insert("src", meshName);
			doc.elements.GetById(entElemId)->childrenIds.PushBack(IDHandle<XMLElement>(meshElem->id));
		}
	}

	SaveXMLDocToFile(&doc, fileName);
}

void ResourceManager::Render() {
	ExecuteDrawCalls(drawCalls.vals, drawCalls.currentCount);
}

String ResourceManager::FindFileNameByIdAndExtension(const char* ext, uint32 id) {
	for (int i = 0; i < assetIdMap.count; i++) {
		if (assetIdMap.values[i] == (int)id) {
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
