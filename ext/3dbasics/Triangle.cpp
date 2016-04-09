/*
This is the Triangle source file for the 3dbasics program, which provides simple tools
to represent and compute a 3D environment.

Author: Benji Smith, 2013
Licensed under the GNU GPL v3
Full License Text found in LICENSE file
*/

#include "Triangle.h"

Triangle::Triangle(){
	point0 = Vector3(0,0,0);
	point1 = Vector3(0,1,0);
	point2 = Vector3(1,0,0);
}

Triangle::Triangle(const Vector3& _point0, const Vector3& _point1, const Vector3& _point2){
	point0 = _point0;
	point1 = _point1;
	point2 = _point2;
}

Vector3 Triangle::GetNormal(){
	Vector3 edge1 = point1 - point0;
	Vector3 edge2 = point2 - point0;
	
	Vector3 nonUnitNormal = CrossProduct(edge2, edge1);
	return nonUnitNormal.Normalized();
}

float Triangle::CalculateArea(){
	Vector3 edge1 = point1 - point0;
	Vector3 edge2 = point2 - point0;
	
	Vector3 normal = CrossProduct(edge1, edge2); 
	return 0.5f * normal.Magnitude();  //1/2 * base * height
}

