/*
This is the Ray header file for the 3dbasics program, which provides simple tools
to represent and compute a 3D environment.

Author: Benji Smith, 2013
Licensed under the GNU GPL v3
Full License Text found in LICENSE file
*/

#ifndef RAY_H
#define RAY_H

#include "Vector3.h"

struct Ray{
	Vector3 origin;
	
	private:
	Vector3 direction; //direction is exclusively a normal length vector, meaning that we don't want arbitrary mutation.
	
	public:
	Ray();
	Ray(const Vector3& _origin, const Vector3& _direction);
	
	void SetDirection(const Vector3& _direction);
	Vector3 GetDirection();
	
};



#endif