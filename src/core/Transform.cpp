#include "Transform.h"
#include "../../ext/3dbasics/Mat4.h"
#include "../../ext/3dbasics/Vector4.h"

Mat4x4 Transform::GetLocalToGlobalMatrix() {
	Mat4x4 matrix;
	matrix.SetColumn(0, Vector4(Rotate(X_AXIS * scale.x, rotation), 0));
	matrix.SetColumn(1, Vector4(Rotate(Y_AXIS * scale.y, rotation), 0));
	matrix.SetColumn(2, Vector4(Rotate(Z_AXIS * scale.z, rotation), 0));
	matrix.SetColumn(3, Vector4(position, 1));
	
	return matrix;
}

Mat4x4 Transform::GetGlobaltoLocalMatrix() {
	//TODO
	return Mat4x4();
}

Vector3 Transform::GetGlobalPosition() {
	//TODO
	return Vector3();
}
