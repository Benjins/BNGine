#include "Vector4.h"
#include "Vector3.h"



Vector4::Vector4(){
	w = x = y = z = 0;
}

Vector4::Vector4(float _x, float _y, float _z, float _w){
	x = _x;
	y = _y; 
	z = _z;
	w = _w;
}

Vector4::Vector4(const Vector3& vec, float _w){
	x = vec.x;
	y = vec.y;
	z = vec.z;
	w = _w;
}

Vector4::Vector4(float* arrayInit){
	x = arrayInit[0];
	y = arrayInit[1];
	z = arrayInit[2];
	w = arrayInit[3];
}
