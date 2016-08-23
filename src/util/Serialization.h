#ifndef SERIALIZATION_H
#define SERIALIZATION_H

struct Vector2;
struct Vector3;
struct Vector4;
struct Quaternion;

struct String;

Vector2 ParseVector2(const char* str);
String EncodeVector2(const Vector2& vec);

Vector3 ParseVector3(const char* str);
String EncodeVector3(const Vector3& vec);

Vector4 ParseVector4(const char* str);
String EncodeVector4(const Vector4& vec);

Quaternion ParseQuaternion(const char* str);
String EncodeQuaternion(const Quaternion& quat);

String Itoa(int val);
String Ftoa(float val);

String EncodeBool(bool val);
bool ParseBool(String str);

#pragma once


#endif
