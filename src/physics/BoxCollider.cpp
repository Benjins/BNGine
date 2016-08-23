#include "BoxCollider.h"

bool WithinVolume(Vector3 pos, Vector3 min, Vector3 max) {
	return min.x < pos.x && pos.x < max.x
		&& min.y < pos.y && pos.y < max.y
		&& min.z < pos.z && pos.z < max.z;
}

