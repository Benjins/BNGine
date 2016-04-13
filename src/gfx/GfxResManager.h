#ifndef GFXRESMANAGER_H
#define GFXRESMANAGER_H

#pragma once

#include "../../ext/CppUtils/idbase.h"

#include "Shader.h"
#include "Program.h"
#include "Mesh.h"
#include "DrawCall.h"
#include "Material.h"

struct MemStream;

struct GfxResManager {
	IDTracker<Shader> shaders;
	IDTracker<Program> programs;
	IDTracker<Material> materials;
	IDTracker<Mesh> meshes;
	IDTracker<DrawCall> drawCalls;

	StringMap<int> assetIdMap;

	GfxResManager();

	void LoadAssetFile(const char* fileName);

	void LoadMeshFromChunk(MemStream& stream, Mesh* outModel);
	void LoadVShaderFromChunk(MemStream& stream, Shader* outModel);
	void LoadFShaderFromChunk(MemStream& stream, Shader* outModel);

	void Render();
};

#endif