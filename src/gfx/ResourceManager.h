#ifndef ResourceManager_H
#define ResourceManager_H

#pragma once

#include "../../ext/CppUtils/idbase.h"

#include "Shader.h"
#include "Program.h"
#include "Mesh.h"
#include "DrawCall.h"
#include "Material.h"
#include "Texture.h"
#include "Level.h"
#include "BitmapFont.h"
#include "UniFont.h"
#include "Prefab.h"
#include "Armature.h"
#include "CubeMap.h"

#include "../anim/Animations.h"

#include "../script/ScriptObject.h"

struct MemStream;
struct Transform;

struct ResourceManager {
	IDTracker<Shader> shaders;
	IDTracker<Program> programs;
	IDTracker<Material> materials;
	IDTracker<Mesh> meshes;
	IDTracker<Texture> textures;
	IDTracker<DrawCall> drawCalls;
	IDTracker<Level> levels;
	IDTracker<BitmapFont> fonts;
	IDTracker<UniFont> uniFonts;
	IDTracker<Prefab> prefabs;
	IDTracker<ScriptObject> scripts;
	IDTracker<Armature> armatures;
	IDTracker<AnimationTrack> anims;
	IDTracker<CubeMap> cubeMaps;

	StringMap<int> assetIdMap;

	ResourceManager();

	void Reset();

	void LoadAssetFile(const char* fileName);

	void LoadMeshFromChunk(MemStream& stream, Mesh* outModel);
	void LoadArmatureFromChunk(MemStream& stream, Armature* outArmature);
	void LoadVShaderFromChunk(MemStream& stream, Shader* outShader);
	void LoadFShaderFromChunk(MemStream& stream, Shader* outShader);
	void LoadTextureFromChunk(MemStream& stream, Texture* outTexture);
	void LoadMaterialFromChunk(MemStream& stream, Material* outMat);
	void LoadLevelFromChunk(MemStream& stream, Level* outLevel);
	void LoadTransform(MemStream& stream, Transform* outTrans);
	void LoadBitmapFontFromChunk(MemStream& stream, BitmapFont* outFont);
	void LoadUniFontFromChunk(MemStream& stream, UniFont* outFont);
	void LoadPrefabFromChunk(MemStream& stream, Prefab* outPrefab);
	void LoadScriptObjectFromChunk(MemStream& stream, ScriptObject* outScript);
	void LoadAnimationTrackFromChunk(MemStream& stream, AnimationTrack* outTrack);
	void LoadCubeMapFromChunk(MemStream& stream, CubeMap* outCubeMap);

	void SaveLevelToFile(const Level* lvl, const char* fileName);
	void SavePrefabToFile(Prefab* prefab, const char* fileName);

	String FindFileNameByIdAndExtension(const char* ext, uint32 id);
	void FindFileNamesByExtension(const char* ext, Vector<String>* outFiles);

	void Render();
};

#endif