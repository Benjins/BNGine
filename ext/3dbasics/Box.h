/*
This is the Box header file for the 3dbasics program, which provides simple tools
to represent and compute a 3D environment.

Author: Benji Smith, 2013
Licensed under the GNU GPL v3
Full License Text found in LICENSE file
*/

#ifndef BOX_H
#define BOX_H

#include "Vector3.h"

/*
The box structure holds information about a Cuboid (box) in 3D.  It has an origin (centre), and scale(size).
For now, it can only tell if points are inside the box, and those points need to be given in local space
(any Transforms it's parented to will need to localise the vector before Contains is called).
*/

struct Box
{
    Vector3 origin;
    Vector3 scale;

    Box();
    Box(const Vector3& _origin);
    Box(const Vector3& _origin, const Vector3& _scale);

    bool Contains(const Vector3& testPt) const;
};

#endif // BOX_H
