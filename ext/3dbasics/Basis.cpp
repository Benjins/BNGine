/*
This is the Basis source file for the 3dbasics program, which provides simple tools
to represent and compute a 3D environment.

Author: Benji Smith, 2013
Licensed under the GNU GPL v3
Full License Text found in LICENSE file
*/

#include "Basis.h"

Basis3D::Basis3D(){
    i = X_AXIS;
    j = Y_AXIS;
    k = Z_AXIS;
}
Basis3D::Basis3D(const Vector3& _i, const Vector3& _j, const Vector3& _k){
    i = _i;
    j = _j;
    k = _k;
}

Basis3D::Basis3D(const Basis3D& original){
    i = original.i;
    j = original.j;
    k = original.k;
}

Basis3D::Basis3D(const Vector3& u, const Vector3& v){
    i = u.Normalized();
    k = CrossProduct(i,v).Normalized();
    j = CrossProduct(k,i);
}

Vector3 Basis3D::ConvertLocalToGlobal(const Vector3& local) const{
    return (i*local.x + j*local.y + k*local.z);
}

Vector3 Basis3D::ConvertGlobalToLocal(const Vector3& global) const{
    return Vector3(DotProduct(global, i),
                   DotProduct(global, j),
                   DotProduct(global, k));
}

void Basis3D::Normalize(){
    i.Normalize();
    j.Normalize();
    k.Normalize();
}

Vector3 ConvertLocalToGlobal(const Basis3D& basis, const Vector3& local){
    return (basis.i * local.x +
            basis.j * local.y +
            basis.k * local.z);
}
Vector3 ConvertGlobalToLocal(const Basis3D& basis, const Vector3& global){
    return Vector3(DotProduct(global, basis.i),
                   DotProduct(global, basis.j),
                   DotProduct(global, basis.k));
}

Basis2D::Basis2D(){
    u = X_AXIS_2D;
    v = Y_AXIS_2D;
}

Basis2D::Basis2D(const Vector2& _u, const Vector2& _v){
    u = _u;
    v = _v;
}

Basis2D::Basis2D(const Basis2D& original){
    u = original.u;
    v = original.v;
}

Vector2 Basis2D::ConvertLocalToGlobal(const Vector2& local) const{
    return (u*local.x + v*local.y);
}

Vector2 Basis2D::ConvertGlobalToLocal(const Vector2& global) const{
    return Vector2(DotProduct(global, u),
                   DotProduct(global, v));
}

void Basis2D::Normalize(){
    u.Normalize();
    v.Normalize();
}

Vector2 ConvertLocalToGlobal(const Basis2D& basis, const Vector2& local){
    return ( basis.u * local.x
            +basis.v * local.y);
}

Vector2 ConvertGlobalToLocal(const Basis2D& basis, const Vector2& global){
    return Vector2(DotProduct(global, basis.u),
                   DotProduct(global, basis.v));
}


