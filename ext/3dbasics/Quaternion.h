/*
This is the Quaternion header file for the 3dbasics program, which provides simple tools 
to represent and compute a 3D environment.

Author: Benji Smith, 2013
Licensed under the GNU GPL v3
Full License Text found in LICENSE file
*/

#ifndef QUATERNION_H
#define QUATERNION_H

#define QUAT_IDENTITY Quaternion(1,0,0,0)

struct Vector3;

/*
The Quaternion structure.  The math can seem a bit complex/arbitrary, but really the high-level ideas are easy to grasp.

Quaternions are primarily used to represent rotation in 3D.  The w represents the angle of the rotation, and the x, y, and z represent the vector to rotate about (although the numbers don't exactly line up).

Multiplying two quaternions combines the rotations as if they were done in sequence (note: order matters).

If you want to learn more about the specifics of Quaternions, the internet has a number of sources to look at.
*/

struct Quaternion {
	float w;
	float x;
	float y;
	float z;
	
	Quaternion();
	Quaternion(float wIn, float xIn, float yIn, float zIn);
	Quaternion(const Vector3& axis, float angle); //NOTE: angle is in radians at the moment
	Quaternion(const Vector3& vector);
	Quaternion(const Quaternion& quat);
	
	Quaternion operator+(const Quaternion& addQuat) const;
	Quaternion operator-(const Quaternion& subQuat) const;
	Quaternion operator*(float scale) const;
	Quaternion operator/(float scale) const;
	
	Quaternion operator*(const Quaternion& multQuat) const;
	
	bool operator==(const Quaternion& multQuat) const;
	
	Quaternion Conjugate() const;
	
	float Magnitude() const;
	Quaternion Normalized() const;
	void Normalize(void);
	
	void Print(void) const;

};


#endif
