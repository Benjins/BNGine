/*
This is the Sphere source file for the 3dbasics program, which provides simple tools
to represent and compute a 3D environment.

Author: Benji Smith, 2013
Licensed under the GNU GPL v3
Full License Text found in LICENSE file
*/

#include "Sphere.h"

Sphere::Sphere(){
    origin = Vector3();
    radius = 1;
}

Sphere::Sphere(const Vector3& _origin){
    origin = _origin;
    radius = 1;
}

Sphere::Sphere(const Vector3& _origin, float _radius){
    origin = _origin;
    radius = _radius;
}

bool Sphere::Contains(const Vector3& testPt) const{
    return ( (testPt - origin).MagnitudeSquared() < radius*radius );
}

