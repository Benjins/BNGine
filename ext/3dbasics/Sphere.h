/*
This is the Sphere header file for the 3dbasics program, which provides simple tools
to represent and compute a 3D environment.

Author: Benji Smith, 2013
Licensed under the GNU GPL v3
Full License Text found in LICENSE file
*/

#ifndef SPHERE_H
#define SPHERE_H

#include "Vector3.h"

struct Sphere{
    Vector3 origin;
    float radius;

    Sphere();
    Sphere(const Vector3& _origin);
    Sphere(const Vector3& _origin, float _radius);

    bool Contains(const Vector3& testPt) const;
};

#endif
