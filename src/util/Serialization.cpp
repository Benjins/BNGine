#include "Serialization.h"

#include "../../ext/3dbasics/Vector3.h"
#include "../../ext/3dbasics/Quaternion.h"
#include "../../ext/CppUtils/strings.h"
#include "../../ext/CppUtils/assert.h"

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

