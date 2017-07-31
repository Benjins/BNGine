#ifndef NAVMESH_H
#define NAVMESH_H

#pragma once

#include "../../ext/CppUtils/idbase.h"
#include "../../ext/CppUtils/vector.h"
#include "../../ext/3dbasics/Vector3.h"

struct NavMeshTri {
	int v0;
	int v1;
	int v2;

	NavMeshTri(const NavMeshTri&) = default;

	NavMeshTri(int _v0, int _v1, int _v2) {
		v0 = _v0;
		v1 = _v1;
		v2 = _v2;
	}
};

struct NavMeshEdge {
	Vector3 v0;
	Vector3 v1;

	NavMeshEdge(Vector3 _v0, Vector3 _v1) {
		v0 = _v0;
		v1 = _v1;
	}

	NavMeshEdge(const NavMeshEdge&) = default;
};

struct NavMesh : IDBase {
	Vector<Vector3> vertices;
	Vector<NavMeshTri> tris;

	// Edges are just cached data of triangles, to make some operations easier
	Vector<NavMeshEdge> edges;

	// Stub this in
	// TODO: Remove this, or get it reading from a real mesh
	NavMesh() {
		vertices.PushBack(Vector3(-2.0f, 1.5f, -2.0f));
		vertices.PushBack(Vector3( 2.0f, 1.5f, -1.0f));
		vertices.PushBack(Vector3( 4.0f, 1.5f,  1.0f));
		vertices.PushBack(Vector3(-1.0f, 1.5f,  4.0f));

		tris.PushBack(NavMeshTri(0, 1, 2));
		tris.PushBack(NavMeshTri(1, 2, 3));

		CalculateEdges();
	}

	void CalculateEdges() {
		edges.Clear();
		BNS_VEC_FOREACH(tris) {
			edges.PushBack(NavMeshEdge(vertices.data[ptr->v0], vertices.data[ptr->v1]));
			edges.PushBack(NavMeshEdge(vertices.data[ptr->v0], vertices.data[ptr->v2]));
			edges.PushBack(NavMeshEdge(vertices.data[ptr->v1], vertices.data[ptr->v2]));
		}
	}

	void DebugRender();
};




#endif
