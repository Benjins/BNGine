#include "Camera.h"
#include "Transform.h"
#include "../../ext/3dbasics/Mat4.h"
#include "../../ext/3dbasics/Vector4.h"

#include "Scene.h"

Camera::Camera() {
	farClip = 1000.0f;
	nearClip = 0.01f;
	fov = 80;
	widthPixels = 1280;
	heightPixels = 720;
}

Mat4x4 Camera::GetCameraMatrix() const {
	Mat4x4 transMat;

	Mat4x4 linMat;
	Mat4x4 affMat;

	Transform* camTrans = GlobalScene->transforms.GetById(transform);

	linMat.SetColumn(0, Vector4(Rotate(X_AXIS, camTrans->rotation.Conjugate()), 0));
	linMat.SetColumn(1, Vector4(Rotate(Y_AXIS, camTrans->rotation.Conjugate()), 0));
	linMat.SetColumn(2, Vector4(Rotate(Z_AXIS, camTrans->rotation.Conjugate()), 0));
	linMat.SetColumn(3, Vector4(0, 0, 0, 1));

	affMat.SetRow(0, Vector4(X_AXIS, -camTrans->position.x));
	affMat.SetRow(1, Vector4(Y_AXIS, -camTrans->position.y));
	affMat.SetRow(2, Vector4(Z_AXIS, -camTrans->position.z));
	affMat.SetRow(3, Vector4(0, 0, 0, 1));

	transMat = linMat * affMat;

	return transMat;
}

Mat4x4 Camera::GetPerspectiveMatrix() const {
	Mat4x4 persp;

	float aspectRatio = widthPixels / heightPixels;

	float fieldOfView_Rad = fov / 180 * 3.14159265f;
	float tanHalfFOV = tanf(fieldOfView_Rad / 2);
	float zRange = nearClip - farClip;

	float x1 = 1 / aspectRatio / tanHalfFOV;
	float y2 = 1 / tanHalfFOV;
	float z3 = (-nearClip - farClip) / zRange;
	float z4 = 2 * farClip*nearClip / zRange;

	persp.SetRow(0, Vector4(x1, 0, 0, 0));
	persp.SetRow(1, Vector4(0, y2, 0, 0));
	persp.SetRow(2, Vector4(0, 0, z3, z4));
	persp.SetRow(3, Vector4(0, 0, 1, 0));

	return persp;
}

