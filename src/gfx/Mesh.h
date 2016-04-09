#ifndef MESH_H
#define MESH_H

#pragma once

#include "../../ext/CppUtils/idbase.h"
#include "../../ext/CppUtils/vector.h"

#include "../../ext/3dbasics/Vector3.h"
#include "../../ext/3dbasics/Vector2.h"

#include "GLWrap.h"

struct Face {
	int indices[3];
};

struct Vertex {
	Vector3 position;

	Vertex(const Vector3& pos) : position(pos) {
	}
};

struct Mesh : IDBase {
	Vector<Vertex> vertices;
	Vector<Face> faces;
	
	GLuint vbo;

	void UploadToGfxDevice();
};


#endif