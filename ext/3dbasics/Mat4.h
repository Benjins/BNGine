#ifndef MAT4X4_H
#define MAT4X4_H

#pragma once

struct Vector3;
struct Vector4;

struct Mat4x4{
	float m[4][4];
	
	Mat4x4(const Mat4x4& mat);
	explicit Mat4x4(float scale = 1);

	Mat4x4 operator*(const Mat4x4& param) const;
	Vector4 operator*(const Vector4& param) const;

	Vector3 MultiplyAsDirection(const Vector3& vec) const;
	Vector3 MultiplyAsPosition(const Vector3& vec) const;

	Vector4 GetColumn(int index) const;
	Vector4 GetRow(int index) const;

	float GetDeterminant() const;
	Mat4x4 GetTranspose() const;
	Mat4x4 GetInverse() const;

	void SetRow(int index, const Vector4& value);
	void SetColumn(int index, const Vector4& value);
};

float DotProduct(const Vector4& param1, const Vector4& param2);

float DotProduct(const float a[], const float b[], int length);

#endif