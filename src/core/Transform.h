#ifndef TRANSFORM_H
#define TRANSFORM_H

#include "../../ext/3dbasics/Vector3.h"
#include "../../ext/3dbasics/Quaternion.h"

#include "Component.h"

struct Mat4x4;

struct Transform : Component{
	Vector3 position;
	Quaternion rotation;
	Vector3 scale;

	int parent;

	Mat4x4 GetLocalToGlobalMatrix();
	Mat4x4 GetGlobaltoLocalMatrix();
	Vector3 GetGlobalPosition();
};

#endif