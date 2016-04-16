#ifndef SERIALIZATION_H
#define SERIALIZATION_H

struct Vector2;
struct Vector3;
struct Quaternion;

struct String;

Vector3 ParseVector3(const char* str);
String EncodeVector3(const Vector3& vec);

Quaternion ParseQuaternion(const char* str);
String EncodeQuaternion(const Quaternion& quat);

#pragma once


#endif
