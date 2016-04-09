/*
This is the Plane header file for the 3dbasics program, which provides simple tools
to represent and compute a 3D environment.

Author: Benji Smith, 2013
Licensed under the GNU GPL v3
Full License Text found in LICENSE file
*/

#ifndef PLANE_H
#define PLANE_H

#include "Vector3.h"

struct Plane{
	Vector3 origin;
	
	private:
	Vector3 normal;

	public:
	Plane();
	Plane(const Vector3& _normal);
	Plane(const Vector3& _origin, const Vector3& _normal);
	
	void SetNormal(const Vector3& _normal);
	Vector3 GetNormal();
	
};


#endif