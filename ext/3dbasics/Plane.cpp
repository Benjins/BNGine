/*
This is the Plane source file for the 3dbasics program, which provides simple tools
to represent and compute a 3D environment.

Author: Benji Smith, 2013
Licensed under the GNU GPL v3
Full License Text found in LICENSE file
*/

#include "Plane.h"

Plane::Plane(){
	origin = Vector3(0,0,0);
	normal = Vector3(0,0,1);
}

Plane::Plane(const Vector3& _normal){
	origin = Vector3(0,0,0);
	normal = _normal.Normalized();
}

Plane::Plane(const Vector3& _origin, const Vector3& _normal){
	origin = _origin;
	normal = _normal.Normalized();
}

void Plane::SetNormal(const Vector3& _normal){
	normal = _normal.Normalized();
}

Vector3 Plane::GetNormal(){
	return normal;
}
