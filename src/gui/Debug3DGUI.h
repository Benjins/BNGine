#ifndef DEBUG3DGUI_H
#define DEBUG3DGUI_H

#pragma once

struct Vector3;
struct Transform;

void DebugDrawWireCube(Vector3 pos, Vector3 size);
void DebugDrawWireCube(Vector3 pos, Vector3 size, const Transform* transform = nullptr);

#endif
