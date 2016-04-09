/*
This is the Ray source file for the 3dbasics program, which provides simple tools
to represent and compute a 3D environment.

Author: Benji Smith, 2013
Licensed under the GNU GPL v3
Full License Text found in LICENSE file
*/

#include "Ray.h"

Ray::Ray(){
	origin = Vector3(0,0,0);
	direction = Vector3(0,0,1);
}

Ray::Ray(const Vector3& _origin, const Vector3& _direction){
	origin    = _origin;
	direction = _direction.Normalized(); 
}

void Ray::SetDirection(const Vector3& _direction){
	direction = _direction.Normalized();
}

Vector3 Ray::GetDirection(){
	return direction;
}