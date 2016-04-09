/*
This is the Triangle header file for the 3dbasics program, which provides simple tools
to represent and compute a 3D environment.

Author: Benji Smith, 2013
Licensed under the GNU GPL v3
Full License Text found in LICENSE file
*/

#ifndef TRIANGLE_H
#define TRIANGLE_H

#include "Vector3.h"

struct Plane;

struct Triangle{
	Vector3 point0;
	Vector3 point1;
	Vector3 point2;
	
	Triangle();
	Triangle(const Vector3& pt0, const Vector3& pt1, const Vector3& pt2);
	
	Vector3 GetNormal();
	float CalculateArea();
	
};


#endif