/*
This is the Quaternion source file for the 3dbasics program, which provides simple tools
to represent and compute a 3D environment.

Author: Benji Smith, 2013
Licensed under the GNU GPL v3
Full License Text found in LICENSE file
*/

#include "Quaternion.h"
#include "Vector3.h"

#include <cmath>
#include <iostream>

using namespace std;

//Default constructor
Quaternion::Quaternion(){
	w = 1;
	x = 0;
	y = 0;
	z = 0;
}

Quaternion::Quaternion(float wIn, float xIn, float yIn, float zIn){
	w = wIn;
	x = xIn;
	y = yIn;
	z = zIn;
}

//Constructs a Quaternion that represents the given axis-angle rotation
Quaternion::Quaternion(const Vector3& axis, float angle){
	const float _Pi = 3.14159265358979323846264338f;

	float halfAngle = fmodf(angle/2, _Pi);
	w = cos(halfAngle);

	Vector3 normalizedAxis = axis.Normalized();
	float sinHalfAngle = sin(halfAngle);

	x = normalizedAxis.x * sinHalfAngle;
	y = normalizedAxis.y * sinHalfAngle;
	z = normalizedAxis.z * sinHalfAngle;
}

//Constructs a Quaternion from a Vector3, basically just adding a 0 component in front of it.
Quaternion::Quaternion(const Vector3& vector){
	w = 0;
	x = vector.x;
	y = vector.y;
	z = vector.z;
}

Quaternion::Quaternion(const Quaternion& quat){
	w = quat.w;
	x = quat.x;
	y = quat.y;
	z = quat.z;
}

//Add two quaternions: add each component and construct a new quaternion
Quaternion Quaternion::operator+(const Quaternion& addQuat) const{
	return Quaternion(w + addQuat.w,
					  x + addQuat.x,
					  y + addQuat.y,
					  z + addQuat.z);
}

//Subtract two quaternions: subtract each component and construct a new quaternion
Quaternion Quaternion::operator-(const Quaternion& subQuat) const{
	return Quaternion(w - subQuat.w,
					  x - subQuat.x,
					  y - subQuat.y,
					  z - subQuat.z);
}

//Scale a quaternion by a float/scalar.  Multiply each component by the scale
Quaternion Quaternion::operator*(float scale) const{
	return Quaternion(w * scale,
					  x * scale,
					  y * scale,
					  z * scale);
}

//Scale a quaternion by the inverse of a float/scalar.  Divide each component by the scale
Quaternion Quaternion::operator/(float scale) const{
	return Quaternion(w / scale,
					  x / scale,
					  y / scale,
					  z / scale);
}

//Multiply two Quaternions, combine two rotations together
//Although it looks weird/complex, it's really just math that lets you multiply two quaternions together.
//It combines the rotations as if you'd done one, then the other.
//NOTE: multiplication of quaternions is non-commutative (order matters), just like rotations
Quaternion Quaternion::operator*(const Quaternion& multQuat) const{
	return Quaternion(w*multQuat.w - x*multQuat.x - y*multQuat.y - z*multQuat.z,
					  x*multQuat.w + w*multQuat.x - z*multQuat.y + y*multQuat.z,
					  y*multQuat.w + z*multQuat.x + w*multQuat.y - x*multQuat.z,
					  z*multQuat.w - y*multQuat.x + x*multQuat.y + w*multQuat.z);
}

bool Quaternion::operator==(const Quaternion& param) const{
	return x == param.x && y == param.y && z == param.z && w == param.w;
}

//Get the conjugate (inverse) of the Quaternion
Quaternion Quaternion::Conjugate() const{
	return Quaternion(w,-x,-y,-z);
}

//The magnitude of a quaternion is similar to that of a vector
//(sqrt of the sum of squares of components)
float Quaternion::Magnitude() const{
	return sqrt(w*w + x*x + y*y + z*z);
}

//Return the normalization of the quaternion (same proportions, magnitude of one)
//NOTE: this does not modify the original quaternion.  For that, use Normalize
Quaternion Quaternion::Normalized() const{
	float magnitude = Magnitude();
	return Quaternion(w / magnitude,
					  x / magnitude,
					  y / magnitude,
					  z / magnitude);
}

//Normalize the quaternion to a magnitude of 1
//NOTE: this alters the original quaternion.  To get its normalization without modifying it, use Normalized() instead
void Quaternion::Normalize(){
	float magnitude = Magnitude();
	w /= magnitude;
	x /= magnitude;
	y /= magnitude;
	z /= magnitude;
}

//Print the values of a quaternion in a readable fashion
void Quaternion::Print() const{
	cout << "\nThe W value is: " << w;
	cout << "\nThe X value is: " << x;
	cout << "\nThe Y value is: " << y;
	cout << "\nThe Z value is: " << z << endl;
}




