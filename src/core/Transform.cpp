#include "Transform.h"
#include "../../ext/3dbasics/Mat4.h"
#include "../../ext/3dbasics/Matrix.h"
#include "../../ext/3dbasics/Vector4.h"

#include "Scene.h"

Transform::Transform() {
	rotation = QUAT_IDENTITY;
	scale = Vector3(1, 1, 1);
}

Vector3 Transform::Forward() {
	return GetLocalToGlobalMatrix().MultiplyAsDirection(Z_AXIS).Normalized();
}

Vector3 Transform::Right(){
	return GetLocalToGlobalMatrix().MultiplyAsDirection(X_AXIS).Normalized();
}

Vector3 Transform::Up(){
	return GetLocalToGlobalMatrix().MultiplyAsDirection(Y_AXIS).Normalized();
}

Mat4x4 Transform::GetLocalToGlobalMatrix() const {
	Mat4x4 matrix = LocRotScaleToMat(position, rotation, scale);
	
	if (parent.id == 0xFFFFFFFF) {
		return matrix;
	}
	else {
		return GlobalScene->transforms.GetById(parent)->GetLocalToGlobalMatrix() * matrix;
	}
}

Mat4x4 Transform::GetGlobaltoLocalMatrix() const {

	Mat4x4 linMat;
	Mat4x4 scaleMat;
	Mat4x4 affMat;

	linMat.SetColumn(0, Vector4(Rotate(X_AXIS, rotation.Conjugate()), 0));
	linMat.SetColumn(1, Vector4(Rotate(Y_AXIS, rotation.Conjugate()), 0));
	linMat.SetColumn(2, Vector4(Rotate(Z_AXIS, rotation.Conjugate()), 0));
	linMat.SetColumn(3, Vector4(0, 0, 0, 1));

	scaleMat.SetColumn(0, Vector4(X_AXIS / scale.x, 0));
	scaleMat.SetColumn(1, Vector4(Y_AXIS / scale.y, 0));
	scaleMat.SetColumn(2, Vector4(Z_AXIS / scale.z, 0));
	scaleMat.SetColumn(3, Vector4(0, 0, 0, 1));

	affMat.SetRow(0, Vector4(X_AXIS, -position.x));
	affMat.SetRow(1, Vector4(Y_AXIS, -position.y));
	affMat.SetRow(2, Vector4(Z_AXIS, -position.z));
	affMat.SetRow(3, Vector4(0, 0, 0, 1));

	Mat4x4 transMat = scaleMat * linMat * affMat;

	if (parent.id == 0xFFFFFFFF) {
		return transMat;
	}
	else {
		return transMat * GlobalScene->transforms.GetById(parent)->GetGlobaltoLocalMatrix();
	}
}

Vector3 Transform::GetGlobalPosition() const {
	Mat4x4 loc2glob = GetLocalToGlobalMatrix();
	return loc2glob.MultiplyAsPosition(Vector3(0, 0, 0));
}

Mat4x4 LocRotScaleToMat(Vector3 loc, Quaternion rot, Vector3 scale) {
	Mat4x4 matrix;
	matrix.SetColumn(0, Vector4(Rotate(X_AXIS * scale.x, rot), 0));
	matrix.SetColumn(1, Vector4(Rotate(Y_AXIS * scale.y, rot), 0));
	matrix.SetColumn(2, Vector4(Rotate(Z_AXIS * scale.z, rot), 0));
	matrix.SetColumn(3, Vector4(loc, 1));

	return matrix;
}

