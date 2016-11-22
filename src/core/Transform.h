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

	Transform();
	
	Vector3 Forward();
	Vector3 Right();
	Vector3 Up();

	Mat4x4 GetLocalToGlobalMatrix() const;
	Mat4x4 GetGlobaltoLocalMatrix() const;
	Vector3 GetGlobalPosition() const;
};

Mat4x4 LocRotScaleToMat(Vector3 loc, Quaternion rot, Vector3 scale);

#endif