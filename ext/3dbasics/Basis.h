/*
This is the Basis header file for the 3dbasics program, which provides simple tools
to represent and compute a 3D environment.

Author: Benji Smith, 2013
Licensed under the GNU GPL v3
Full License Text found in LICENSE file
*/

#ifndef BASIS_H
#define BASIS_H

#include "Vector2.h"
#include "Vector3.h"

#define CARTESIAN_BASIS3D Basis3D(X_AXIS, Y_AXIS, Z_AXIS)
#define CARTESIAN_BASIS2D Basis2D(X_AXIS, Y_AXIS)

#define RH_Y_UP_BASIS3D Basis3D(X_AXIS*-1, Z_AXIS, Y_AXIS)
#define LH_Y_UP_BASIS3D Basis3D(X_AXIS,   Z_AXIS, Y_AXIS)
#define LH_CARTESIAN_BASIS3D Basis3D(X_AXIS*-1, Y_AXIS, _AXIS)

/* The Basis2D and Basis3D structs both hold information about coordinate systems.
 A coordinate system is what the components in a vector represent.
 The most common coordinate system is the Cartesian coordinate system, where each component represents a unit-length vector in the direction of one of the axes,
 however alternate coordinate systems are very useful in transformations, parent-child relationships, etc.
*/


struct Basis3D{
    Vector3 i; //Analagous to x-axis
    Vector3 j; //Analagous to y-axis
    Vector3 k; //Analagous to z-axis

    Basis3D();
    Basis3D(const Vector3& _i, const Vector3& _j, const Vector3& _k);
    Basis3D(const Basis3D& original);
    Basis3D(const Vector3& u, const Vector3& v);

    Vector3 ConvertLocalToGlobal(const Vector3& local) const;
    Vector3 ConvertGlobalToLocal(const Vector3& global) const;

    void Normalize();
};

Vector3 ConvertLocalToGlobal(const Basis3D& basis, const Vector3& local);
Vector3 ConvertGlobalToLocal(const Basis3D& basis, const Vector3& global);

struct Basis2D{
    Vector2 u; //Analagous to x-axis
    Vector2 v; //Analagous to y-axis

    Basis2D();
    Basis2D(const Vector2& _u, const Vector2& _v);
    Basis2D(const Basis2D& original);

    Vector2 ConvertLocalToGlobal(const Vector2& local) const;
    Vector2 ConvertGlobalToLocal(const Vector2& global) const;

    void Normalize();
};

Vector2 ConvertLocalToGlobal(const Basis2D& basis, const Vector2& local);
Vector2 ConvertGlobalToLocal(const Basis2D& basis, const Vector2& global);


#endif // BASIS_H
