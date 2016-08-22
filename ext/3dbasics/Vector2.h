/*
This is the Vector2 header file for the 3dbasics program, which provides simple tools
to represent and compute a 3D environment.

Author: Benji Smith, 2013
Licensed under the GNU GPL v3
Full License Text found in LICENSE file
*/

#ifndef VECTOR2_H
#define	VECTOR2_H

#define X_AXIS_2D Vector2(1,0)
#define Y_AXIS_2D Vector2(0,1)

/*The Vector2 structure, which holds information about 2-dimensional vectors.

//Most functions (except cross product) are overloaded to work on Vector2's and Vector3's, however not on a mix of the two.

Uses include: screen coordinates, points on a plane, etc.*/

struct Vector3;

struct Vector2{
	float x;
	float y;

	//Constructors
	Vector2();
	Vector2(float xIn, float yIn);
	Vector2(const Vector2& newVec);

	//The following operators all apply to vector math, namely adding two vectors
	//and scalar multiplication.
	Vector2 operator+(const Vector2& addVec) const;
	Vector2 operator-(const Vector2& subVec) const;
	Vector2 operator*(float scale) const;
	Vector2 operator/(float scale) const;

	//Comparison operators simply check each corresponding component
	bool operator==(const Vector2& compare) const;
	bool operator!=(const Vector2& compare) const;

	//Get the index-th element (x = 0, y = 1)
	inline float& operator[](const int index) {
		return ((float*)this)[index];
	}

	//The length of the vector
	float Magnitude() const;

	//The square of the length of the vector (optimization, doesn't require a sqrt())
	float MagnitudeSquared() const;

	//Return the vector's normalized form
	//NOTE: This does not modify the original vector.  To do that, use Normlize() instead
	Vector2 Normalized() const;

	//Set the vector to its normalizd form, so that its magnitude (length) is one
	void Normalize();

	//Return the orthagonal complement of the vector (the vector perpendicular to it, of the same length)
	Vector2 Perp() const;

	//Print all values of the vector in a readable form (used mostly for debugging)
	void Print() const;
};

float DotProduct(const Vector2& a, const Vector2& b);

Vector2 Project(const Vector2& toProject, const Vector2& projectOnto);

float CalculateTriangleAreaBetween(const Vector2& first, const Vector2& second);

float CalculateArea(const Vector2& vert0, const Vector2& vert1, const Vector2& vert2);


#endif



