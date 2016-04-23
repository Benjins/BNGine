#ifndef CAMERA_H
#define CAMERA_H

#pragma once

struct Transform;
struct Mat4x4;

struct Camera {
	float fov;
	float nearClip;
	float farClip;
	float widthPixels;
	float heightPixels;
	float xOffset;
	float yOffset;

	int transform;

	Camera();

	Mat4x4 GetCameraMatrix() const;
	Mat4x4 GetPerspectiveMatrix() const;
};

#endif