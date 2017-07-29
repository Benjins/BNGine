#ifndef MESH_COLLIDER_H
#define MESH_COLLIDER_H

#pragma once

#include "ColliderFlags.h"

#include "../core/Component.h"

#include "../gfx/Mesh.h"

struct MeshCollider : Component {
	ColliderFlags colFlags;
	bool isTrigger;

	/*[SerializeFromId("src", "obj")]*/
	IDHandle<Mesh> mesh;

	/*[DoNotSerialize]*/
	bool hasComputedBoundingBox;
	/*[DoNotSerialize]*/
	Vector3 bbMin;
	/*[DoNotSerialize]*/
	Vector3 bbMax;

	MeshCollider() {
		isTrigger = false;
		colFlags = CF_NONE;

		hasComputedBoundingBox = false;

		//scale = 0.0f;
	}

	// In local coords, if it's not obvious
	void GetBoundingBoxMinMax(Vector3* outMin, Vector3* outMax);
};

#endif
