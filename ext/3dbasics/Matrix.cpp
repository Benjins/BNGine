/*
This is the Matrix source file for the 3dbasics program, which provides simple tools
to represent and compute a 3D environment.

Author: Benji Smith, 2013
Licensed under the GNU GPL v3
Full License Text found in LICENSE file
*/

#include "Matrix.h"

//This is the identity matrix
Matrix3x3::Matrix3x3(){
    row1 = X_AXIS; //(1,0,0)
    row2 = Y_AXIS; //(0,1,0)
    row3 = Z_AXIS; //(0,0,1)
}

Matrix3x3::Matrix3x3(const Vector3& _row1, const Vector3& _row2, const Vector3& _row3){
    row1 = _row1;
    row2 = _row2;
    row3 = _row3;
}

Vector3 Matrix3x3::Column1() const{
    return Vector3(row1.x, row2.x, row3.x);
}

Vector3 Matrix3x3::Column2() const{
    return Vector3(row1.y, row2.y, row3.y);
}

Vector3 Matrix3x3::Column3() const{
    return Vector3(row1.z, row2.z, row3.z);
}

//The transpose is basically taking the rows, and making them columns (reflecting it across the diagonal)
Matrix3x3 Matrix3x3::Transpose() const{
    return Matrix3x3(Column1(), Column2(), Column3());
}

 //Adding matrices adds each corresponding value
Matrix3x3 Matrix3x3::operator+(const Matrix3x3& multMatrix) const{
    return Matrix3x3( row1+ multMatrix.row1,
                      row2+ multMatrix.row2,
                      row3+ multMatrix.row3);
}

 //Multiplying matrices is a bit mathy, but effectively combines their transformations, much like multiplying Quaternions
Matrix3x3 Matrix3x3::operator*(const Matrix3x3& param) const{
    return Matrix3x3( Vector3( DotProduct(row1, param.Column1()),
                               DotProduct(row1, param.Column2()),
                               DotProduct(row1, param.Column3())),

                      Vector3( DotProduct(row2, param.Column1()),
                               DotProduct(row2, param.Column2()),
                               DotProduct(row2, param.Column3())),

                      Vector3( DotProduct(row3, param.Column1()),
                               DotProduct(row3, param.Column2()),
                               DotProduct(row3, param.Column3())));
}

//Multiplying a matrix by a vector transforms that vector (again, details are a bit mathy)
Vector3 Matrix3x3::operator*(const Vector3& multVector) const{
    return Vector3( DotProduct(row1, multVector),
                    DotProduct(row2, multVector),
                    DotProduct(row3, multVector));
}

//Multiplying a matrix by a float just multiplies each element of the matrix by that float
Matrix3x3 Matrix3x3::operator*(float scale) const{
    return Matrix3x3(row1 * scale,
                     row2 * scale,
                     row3 * scale);
}

//Print the values of the matrix in a readable format
void Matrix3x3::Print() const{
    cout << "\n| " << row1.x << " " << row1.y << " " << row1.z << " |\n";
    cout <<   "| " << row2.x << " " << row2.y << " " << row2.z << " |\n";
    cout <<   "| " << row3.x << " " << row3.y << " " << row3.z << " |\n";
}

Matrix3x3 MakeScaleMatrix3D(float scale){
    return Matrix3x3( Vector3(scale, 0,     0),
                      Vector3(0,     scale, 0),
                      Vector3(0,     0,     scale));
}

Matrix3x3 MakeMatrixFromColumns(const Vector3& column1,
                                           const Vector3& column2,
                                           const Vector3& column3){

    return Matrix3x3( Vector3(column1.x, column2.x, column3.x),
                      Vector3(column1.y, column2.y, column3.y),
                      Vector3(column1.x, column2.z, column3.z));
}

/*
//Not needed now, and they didn't work to begin with
Vector3 ArrayToVector3(float arr[]){
    return Vector3(arr[0],
                   arr[1],
                   arr[2]);
}

Vector2 ArrayToVector2(float arr[]){
    return Vector3(arr[0],
                   arr[1]);
}*/





	
Matrix2x2::Matrix2x2(){
	row1 = X_AXIS_2D;
	row2 = Y_AXIS_2D;
}

Matrix2x2::Matrix2x2(const Vector2& _row1, const Vector2& _row2){
	row1 = _row1;
	row2 = _row2;
}

Vector2 Matrix2x2::Column1() const{
	return Vector2(row1.x, row2.x);
}

Vector2 Matrix2x2::Column2() const{
	return Vector2(row1.y, row2.y);
}

//The transpose is basically taking rows, and making them columns (reflecting it across the diagonal)
Matrix2x2 Matrix2x2::Transpose() const{
	return Matrix2x2( Column1(), 
					  Column2());
}

//Adding matrices adds each corresponding value
Matrix2x2 Matrix2x2::operator+(const Matrix2x2& plustMatrix) const{
	return Matrix2x2( row1 + plustMatrix.row1,
					  row2 + plustMatrix.row2);
}

//Multiplying matrices is a bit mathy, but effectively combines their transformations, much like multiplying Quaternions
Matrix2x2 Matrix2x2::operator*(const Matrix2x2& multMatrix) const{
	return Matrix2x2( Vector2( DotProduct(row1, multMatrix.Column1()),
                               DotProduct(row1, multMatrix.Column2())),

                      Vector2( DotProduct(row2, multMatrix.Column1()),
                               DotProduct(row2, multMatrix.Column2())));
}

//Multiplying a matrix by a float just multiplies each element of the matrix by that number
Matrix2x2 Matrix2x2::operator*(float scale) const{
	return Matrix2x2( row1 * scale,
					  row2 * scale);
}

//Multiplying a matrix by a vector transforms that vector (again, details are a bit mathy)
Vector2 Matrix2x2::operator*(const Vector2& multVector) const{
    return Vector2( DotProduct(row1, multVector),
                	DotProduct(row2, multVector));
}

//Print the values of the matrix in a readable format
void Matrix2x2::Print() const{
	cout << "\n| " << row1.x << " " << row1.y << " |\n";
    cout <<   "| " << row2.x << " " << row2.y << " |\n";
}

Matrix2x2 MakeScaleMatrix2D(float scale){
	return Matrix2x2( Vector2(scale,    0),
                      Vector2(0,    scale));
}

Matrix2x2 MakeMatrixFromColumns(const Vector2& column1, const Vector2& column2){
	return Matrix2x2( Vector2(column1.x, column2.x),
					  Vector2(column1.y, column2.y));
}




