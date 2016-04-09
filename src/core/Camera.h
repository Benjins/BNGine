#ifndef CAMERA_H
#define CAMERA_H

#pragma once

struct Transform;
struct Mat4x4;

struct Camera {
	Transform* transform;
	float fov;
	float nearClip;
	float farClip;
	float widthPixels;
	float heightPixels;

	Camera();

	Mat4x4 GetCameraMatrix() const;
	Mat4x4 GetPerspectiveMatrix() const;
};

#endif