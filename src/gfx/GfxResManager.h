#ifndef GFXRESMANAGER_H
#define GFXRESMANAGER_H

#pragma once

#include "../../ext/CppUtils/idbase.h"

#include "Shader.h"
#include "Program.h"
#include "Mesh.h"
#include "DrawCall.h"
#include "Material.h"

struct GfxResManager {
	IDTracker<Shader> shaders;
	IDTracker<Program> programs;
	IDTracker<Material> materials;
	IDTracker<Mesh> meshes;
	IDTracker<DrawCall> drawCalls;

	GfxResManager();

	void LoadAssetFile(const char* fileName);

	void LoadMeshFromChunk(unsigned char* meshChunk, int chunkLength, Mesh* outModel);
	void LoadVShaderFromChunk(unsigned char* shaderChunk, int chunkLength, Shader* outModel);
	void LoadFShaderFromChunk(unsigned char* shaderChunk, int chunkLength, Shader* outModel);

	void Render();
};

#endif