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

	void Render();
};

#endif