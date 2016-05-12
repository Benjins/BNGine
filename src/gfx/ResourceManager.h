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

	StringMap<int> assetIdMap;

	ResourceManager();

	void LoadAssetFile(const char* fileName);

	void LoadMeshFromChunk(MemStream& stream, Mesh* outModel);
	void LoadVShaderFromChunk(MemStream& stream, Shader* outShader);
	void LoadFShaderFromChunk(MemStream& stream, Shader* outShader);
	void LoadTextureFromChunk(MemStream& stream, Texture* outTexture);
	void LoadMaterialFromChunk(MemStream& stream, Material* outMat);
	void LoadLevelFromChunk(MemStream& stream, Level* outLevel);
	void LoadTransform(MemStream& stream, Transform* outTrans);
	void LoadBitmapFontFromChunk(MemStream& stream, BitmapFont* outFont);

	void Render();
};

#endif