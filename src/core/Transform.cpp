#include "Transform.h"
#include "../../ext/3dbasics/Mat4.h"
#include "../../ext/3dbasics/Vector4.h"

#include "Scene.h"

Transform::Transform() {
	parent = -1;
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
	Mat4x4 matrix;
	matrix.SetColumn(0, Vector4(Rotate(X_AXIS * scale.x, rotation), 0));
	matrix.SetColumn(1, Vector4(Rotate(Y_AXIS * scale.y, rotation), 0));
	matrix.SetColumn(2, Vector4(Rotate(Z_AXIS * scale.z, rotation), 0));
	matrix.SetColumn(3, Vector4(position, 1));
	
	if (parent < 0) {
		return matrix;
	}
	else {
		return GlobalScene->transforms.GetById(parent)->GetLocalToGlobalMatrix() * matrix;
	}
}

Mat4x4 Transform::GetGlobaltoLocalMatrix() const {
	Mat4x4 linMat;
	Mat4x4 affMat;

	linMat.SetColumn(0, Vector4(Rotate(X_AXIS, rotation.Conjugate()) / scale.x, 0));
	linMat.SetColumn(1, Vector4(Rotate(Y_AXIS, rotation.Conjugate()) / scale.y, 0));
	linMat.SetColumn(2, Vector4(Rotate(Z_AXIS, rotation.Conjugate()) / scale.z, 0));
	linMat.SetColumn(3, Vector4(0, 0, 0, 1));

	affMat.SetRow(0, Vector4(X_AXIS, -position.x));
	affMat.SetRow(1, Vector4(Y_AXIS, -position.y));
	affMat.SetRow(2, Vector4(Z_AXIS, -position.z));
	affMat.SetRow(3, Vector4(0, 0, 0, 1));

	Mat4x4 transMat = linMat * affMat;

	if (parent < 0) {
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
