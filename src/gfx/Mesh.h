#ifndef MESH_H
#define MESH_H

#pragma once

#include "../../ext/CppUtils/idbase.h"
#include "../../ext/CppUtils/vector.h"

#include "../../ext/3dbasics/Vector3.h"
#include "../../ext/3dbasics/Vector2.h"

#include "GLWrap.h"

struct Face {
	int posIndices[3];
	int uvIndices[3];
};

struct Armature;

struct Mesh : IDBase {
	Vector<Vector3> positions;
	Vector<Vector2> uvs;
	Vector<Face> faces;
	
	GLuint posVbo;
	GLuint uvVbo;
	GLuint normalVbo;

	IDHandle<Armature> armatureId;

	Mesh();

	void GenerateNormals(Vector<Vector3>* outNormals);
	void UploadToGfxDevice();

	void Destroy();

	~Mesh();
};


#endif