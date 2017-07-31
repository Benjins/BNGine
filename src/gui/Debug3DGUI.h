#ifndef DEBUG3DGUI_H
#define DEBUG3DGUI_H

#pragma once

struct Vector3;
struct Transform;

void DebugDrawWireCube(Vector3 pos, Vector3 size, const Transform* transform = nullptr);

void DebugDrawLine(Vector3 from, Vector3 to, Vector4 col);
void DebugDrawTriangle(Vector3 v0, Vector3 v1, Vector3 v2, Vector4 col);

#endif
