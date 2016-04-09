/*
This is the Vector2 source file for the 3dbasics program, which provides simple tools
to represent and compute a 3D environment.

Author: Benji Smith, 2013
Licensed under the GNU GPL v3
Full License Text found in LICENSE file
*/

#include "Vector2.h"
#include "Vector3.h"
#include <cmath>
#include <iostream>

using namespace std;


//----------------------------------------------------------
//Constructors
Vector2::Vector2(){
	x = 0;
	y = 0;
}

Vector2::Vector2(float xIn, float yIn){
	x = xIn;
	y = yIn;
}

Vector2::Vector2(const Vector2& newVec){
	x = newVec.x;
	y = newVec.y;
}


//The following operators all apply to vector math, namely adding two vectors
//and scalar multiplication.
Vector2 Vector2::operator+(const Vector2& addVec) const{
	return Vector2(x + addVec.x, y + addVec.y);
}

Vector2 Vector2::operator-(const Vector2& subVec) const{
	return Vector2(x - subVec.x, y - subVec.y);
}

Vector2 Vector2::operator*(float scale) const{
	return Vector2(x*scale, y*scale);
}

Vector2 Vector2::operator/(float scale) const{
	return Vector2(x/scale, y/scale);
}

//Comparison operators simply check each corresponding component
bool Vector2::operator==(const Vector2& compare) const{
	return ((x == compare.x) && (y == compare.y));
}

bool Vector2::operator!=(const Vector2& compare) const{
	return ((x != compare.x) || (y != compare.y));
}

//The length of the vector
float Vector2::Magnitude() const{
	return (sqrt(x*x + y*y));
}

//The square of the length of the vector (optimization, doesn't require a sqrt())
float Vector2::MagnitudeSquared() const{
	return (x*x + y*y);
}

//Return the vector's normalized form
//NOTE: This does not modify the original vector.  To do that, use Normlize() instead
Vector2 Vector2::Normalized() const{
	float magnitude = Magnitude();
	return Vector2(x/magnitude, y/magnitude);
}

//Set the vector to its normalizd form, so that its magnitude (length) is one
void Vector2::Normalize(){
	float magnitude = Magnitude();

	x /= magnitude;
	y /= magnitude;
}

//Return the orthagonal complement of the vector (the vector perpendicular to it, of the same length)
Vector2 Vector2::Perp() const{
    return Vector2(-y, x);
}

//Print all values of the vector in a readable form (used mostly for debugging)
void Vector2::Print() const{
	cout << "\nX is: " << x;
	cout << "\nY is: " << y << endl;
}

//--------------------------------------------------------------------------------------
//Non-method functions of two Vector2's

float DotProduct(const Vector2& a, const Vector2& b){
	return (a.x * b.x
		  + a.y * b.y);
}

Vector2 Project(const Vector2& toProject, const Vector2& projectOnto){
	return projectOnto * (DotProduct(toProject, projectOnto) / projectOnto.MagnitudeSquared());
}

float CalculateTriangleAreaBetween(const Vector2& first, const Vector2& second){
	return CrossProduct( Vector3(first, 0), Vector3(second, 0) ).Magnitude();
}

//-------------------------------------------------------------------------------------
//Additional functions

float CalculateArea(const Vector2& vert0, const Vector2& vert1, const Vector2& vert2){
	Vector2 edge1 = vert1 - vert0;
	Vector2 edge2 = vert2 - vert0;
	
	return CalculateTriangleAreaBetween(edge1, edge2);
}
