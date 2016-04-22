#include "Serialization.h"

#include "../../ext/3dbasics/Vector2.h"
#include "../../ext/3dbasics/Vector3.h"
#include "../../ext/3dbasics/Vector4.h"
#include "../../ext/3dbasics/Quaternion.h"
#include "../../ext/CppUtils/strings.h"
#include "../../ext/CppUtils/assert.h"

#include <string.h>

Vector2 ParseVector2(const char* str) {
	const char* firstNum = str;
	ASSERT(firstNum != nullptr);
	const char* secondNum = strchr(firstNum, ',') + 1;
	ASSERT(secondNum != nullptr);

	return Vector2((float)atof(firstNum), (float)atof(secondNum));
}

String EncodeVector2(const Vector2& vec) {
	String str;
	//God I hope this doesn't cause problems later on...
	str.SetSize(64);
	snprintf(str.string, 64, "%f,%f", vec.x, vec.y);

	return str;
}

Vector3 ParseVector3(const char* str) {
	const char* firstNum = str;
	ASSERT(firstNum != nullptr);
	const char* secondNum = strchr(firstNum, ',') + 1;
	ASSERT(secondNum != nullptr);
	const char* thirdNum = strchr(secondNum, ',') + 1;
	ASSERT(thirdNum != nullptr);

	return Vector3((float)atof(firstNum), (float)atof(secondNum), (float)atof(thirdNum));
}

String EncodeVector3(const Vector3& vec){
	String str;
	//God I hope this doesn't cause problems later on...
	str.SetSize(64);
	snprintf(str.string, 64, "%f,%f,%f", vec.x, vec.y, vec.z);

	return str;
}

Vector4 ParseVector4(const char* str) {
	const char* firstNum = str;
	ASSERT(firstNum != nullptr);
	const char* secondNum = strchr(firstNum, ',') + 1;
	ASSERT(secondNum != nullptr);
	const char* thirdNum = strchr(secondNum, ',') + 1;
	ASSERT(thirdNum != nullptr);
	const char* fourthNum = strchr(thirdNum, ',') + 1;
	ASSERT(fourthNum != nullptr);

	return Vector4((float)atof(firstNum), (float)atof(secondNum), (float)atof(thirdNum), (float)atof(fourthNum));
}

String EncodeVector4(const Vector4& vec) {
	String str;
	//God I hope this doesn't cause problems later on...
	str.SetSize(64);
	snprintf(str.string, 64, "%f,%f,%f,%f", vec.x, vec.y, vec.z, vec.w);

	return str;
}

Quaternion ParseQuaternion(const char* str){
	const char* firstNum = str;
	ASSERT(firstNum != nullptr);
	const char* secondNum = strchr(firstNum, ',') + 1;
	ASSERT(secondNum != nullptr);
	const char* thirdNum = strchr(secondNum, ',') + 1;
	ASSERT(thirdNum != nullptr);
	const char* fourthNum = strchr(thirdNum, ',') + 1;
	ASSERT(fourthNum != nullptr);

	return Quaternion((float)atof(firstNum), (float)atof(secondNum), (float)atof(thirdNum), (float)atof(fourthNum));
}

String EncodeQuaternion(const Quaternion& quat){
	String str;
	//God I hope this doesn't cause problems later on...
	str.SetSize(64);
	snprintf(str.string, 64, "%f,%f,%f,%f", quat.w, quat.x, quat.y, quat.z);

	return str;
}

String Itoa(int val) {
	String str;
	str.SetSize(16);
	snprintf(str.string, 64, "%d", val);

	return str;
}

String Ftoa(float val) {
	String str;
	str.SetSize(32);
	snprintf(str.string, 64, "%f", val);

	return str;
}

