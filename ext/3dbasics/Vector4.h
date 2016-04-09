#ifndef VECTOR4_H
#define VECTOR4_H

#pragma once

struct Vector3;

struct Vector4{
	float x;
	float y;
	float z;
	float w;

	Vector4();
	Vector4(float _x, float _y, float _z, float _w);
	explicit Vector4(const Vector3& vec, float _w = 1);
	explicit Vector4(float* arrayInit);

	Vector4 operator+(const Vector4& param){
		return Vector4(x+param.x, y+param.y, z+param.z, w + param.w);
	}

	Vector4 operator*(float scale){
		return Vector4(x*scale, y*scale, z*scale, w*scale);
	}
};

#endif