#include "Mat4.h"
#include "Vector4.h"
#include "Vector3.h"

Mat4x4::Mat4x4(const Mat4x4& mat){
	for(int i = 0; i < 4; i++){
		for(int j = 0; j < 4; j++){
			m[i][j] = mat.m[i][j];
		}
	}
}

Mat4x4::Mat4x4(float scale){
	for(int i = 0; i < 4; i++){
		for(int j = 0; j < 4; j++){
			m[i][j] = (i==j? scale : 0);
		}
	}
}


Mat4x4 Mat4x4::operator*(const Mat4x4& param) const{
	Mat4x4 result;
	
	for (int j = 0; j < 4; j++) {
		for(int i = 0; i < 4; i++){
			result.m[i][j] = DotProduct(GetRow(i), param.GetColumn(j));
		}
	}
	
	return result;
}

constexpr int fac(int num){
	return num*fac(num - 1);
}

Vector3 Mat4x4::MultiplyAsDirection(const Vector3& vec) const {
	Vector4 oVec = Vector4(vec, 0.0f);
	Vector4 mult = (*this) * oVec;
	return Vector3(mult.x, mult.y, mult.z);
}

Vector3 Mat4x4::MultiplyAsPosition(const Vector3& vec) const {
	Vector4 oVec = Vector4(vec, 1.0f);
	Vector4 mult = (*this) * oVec;
	return Vector3(mult.x, mult.y, mult.z);
}

Vector4 Mat4x4::operator*(const Vector4& param) const{
	return Vector4( DotProduct(param, Vector4((float*)m[0])),
					DotProduct(param, Vector4((float*)m[1])),
					DotProduct(param, Vector4((float*)m[2])),
					DotProduct(param, Vector4((float*)m[3])));
}

Vector4 Mat4x4::GetColumn(int index) const{
	float col[4];
	
	for(int i = 0; i < 4; i++){
		col[i] = m[i][index];
	}
	
	return Vector4(col);
}

Vector4 Mat4x4::GetRow(int index) const{
	return Vector4( (float*)m[index]);
}

Mat4x4 Mat4x4::GetTranspose() const{
	Mat4x4 transpose;
	for(int i = 0; i < 4; i++){
		transpose.SetRow(i, GetColumn(i));
	}

	return transpose;
}

float Mat4x4::GetDeterminant() const{
	return 0;
}

//NOTE: this makes the asusmption that the bottom row is [0,0,0,1],
//as it is in most cases
Mat4x4 Mat4x4::GetInverse() const{
	float ulDet = m[0][0] * ( m[1][1] * m[2][2] - m[1][2] * m[2][1] )
				- m[0][1] * ( m[1][0] * m[2][2] - m[1][2] * m[2][0] )
				+ m[0][2] * ( m[1][0] * m[2][1] - m[1][1] * m[2][0] );

    Mat4x4 res;
    res.m[0][0] = ( m[1][1] * m[2][2] - m[1][2] * m[2][1] ) / ulDet;
    res.m[1][0] = ( m[1][2] * m[2][0] - m[1][0] * m[2][2] ) / ulDet;
    res.m[2][0] = ( m[1][0] * m[2][1] - m[1][1] * m[2][0] ) / ulDet;
 
    res.m[0][1] = ( m[0][2] * m[2][1] - m[0][1] * m[2][2] ) / ulDet;
    res.m[1][1] = ( m[0][0] * m[2][2] - m[0][2] * m[2][0] ) / ulDet;
    res.m[2][1] = ( m[0][1] * m[2][0] - m[0][0] * m[2][1] ) / ulDet;

    res.m[0][2] = ( m[0][1] * m[1][2] - m[0][2] * m[1][1] ) / ulDet;
    res.m[1][2] = ( m[0][2] * m[1][0] - m[0][0] * m[1][2] ) / ulDet;
    res.m[2][2] = ( m[0][0] * m[1][1] - m[0][1] * m[1][0] ) / ulDet;

	Vector3 trans = MultiplyAsPosition(Vector3(0,0,0)) * -1;
	res.SetColumn(3, Vector4(trans.x, trans.y, trans.z, 1));

    return res;
}

void Mat4x4::SetRow(int index, const Vector4& value){
	m[index][0] = value.x;
	m[index][1] = value.y;
	m[index][2] = value.z;
	m[index][3] = value.w;
}

void Mat4x4::SetColumn(int index, const Vector4& value){
	m[0][index] = value.x;
	m[1][index] = value.y;
	m[2][index] = value.z;
	m[3][index] = value.w;
}

float DotProduct(const Vector4& param1, const Vector4& param2){
	return param1.x * param2.x
		 + param1.y * param2.y
		 + param1.z * param2.z
		 + param1.w * param2.w;
}

float DotProduct(const float a[], const float b[], int length){
	float total = 0;
	for(int i = 0; i < length; i++){
		total += (a[i] * b[i]);
	}
	
	return total;
}