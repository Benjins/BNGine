/*
This is the Box source file for the 3dbasics program, which provides simple tools
to represent and compute a 3D environment.

Author: Benji Smith, 2013
Licensed under the GNU GPL v3
Full License Text found in LICENSE file
*/

#include "Box.h"
#include <cmath>

using namespace std;

Box::Box(){
    origin = Vector3();
    scale = Vector3(1,1,1);
}

Box::Box(const Vector3& _origin){
    origin = _origin;
    scale = Vector3(1,1,1);
}

Box::Box(const Vector3& _origin, const Vector3& _scale){
    origin = _origin;
    scale = _scale;
}

bool Box::Contains(const Vector3& testPt) const{
    Vector3 differenceVector = testPt - origin;

    return (   abs(differenceVector.x) <= scale.x
            && abs(differenceVector.y) <= scale.y
            && abs(differenceVector.z) <= scale.z);
}

