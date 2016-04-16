/*
This is the Vector3 header file for the 3dbasics program, which provides simple tools
to represent and compute a 3D environment.

Author: Benji Smith, 2013
Licensed under the GNU GPL v3
Full License Text found in LICENSE file
*/

#ifndef VECTOR3_H
#define	VECTOR3_H

#define X_AXIS Vector3(1,0,0)
#define Y_AXIS Vector3(0,1,0)
#define Z_AXIS Vector3(0,0,1)

#define DEG2RAD 0.01745329251f
#define RAD2DEG 57.2957795131f

#include <cmath>
#include <iostream>

using namespace std;

struct Quaternion;
struct Vector2;

/*
The Vector3 structure, which holds information about a three-dimensional vector

Most of the functions here involve vector math, so if anything doesn't look familiar,
just Google it.  It should be easy to get the intuition behind it.
*/

struct Vector3 {
	float x;
	float y;
	float z;

	Vector3();
	Vector3(float xIn, float yIn, float zIn);
	Vector3(const Vector2& vec, float zIn);
	Vector3(float xIn, const Vector2& vec);
	Vector3(const Vector3& vector);

	//The following operators all apply to vector math, namely adding two vectors
	//and scalar multiplication.
	Vector3 operator+(const Vector3& addVec) const;
	Vector3 operator-(const Vector3& subVec) const;
	Vector3 operator*(float scale) const;
	Vector3 operator/(float scale) const;

	//Comparison operator just checks to see if all corresponding components are equal
	bool operator==(const Vector3& compareVec) const;
	bool operator!=(const Vector3& compareVec) const;

	//Get the index-th element (x = 0, y = 1, z = 2)
	inline float& operator[](const int index) {
		return ((float*)this)[index];
	}

	//The length of the vector
	float Magnitude() const;

	//The square of the length of the vector (for optimization, doesn't require a sqrt())
	float MagnitudeSquared() const;

	//Return the vector's normalized form
	//NOTE: This does not modify the original vector.  To do that, use Normlize() instead
	Vector3 Normalized() const;

	//Set the vector to its normalizd form, so that its magnitude (length) is one
	void Normalize();

    //Scale each componenet by the corresponing componenet in the scale vector
    //(e.g. scaling a vector by Vector3(1,1,2) would keep x and y constant, but double z).
	Vector3 Scaled(const Vector3& scaleVector) const;

	//Print all values of the vector in a readable form (used mostly for debugging)
	void Print() const;

};

//The Dot Product, or overlap of two vectors
float DotProduct(const Vector3& a, const Vector3& b);

//Cross Product, returns a vector orthogonal/perpendicular to the two input vectors
Vector3 CrossProduct(const Vector3& a, const Vector3& b);

//Returns the projection of the first vector onto the second
Vector3 VectorProject(const Vector3& toProject, const Vector3& projectOnto);

//Rotates the given vector by the quaternion
Vector3 Rotate(const Vector3& vector, const Quaternion& rotation);

#endif





