#include "NavMesh.h"

#include "../gui/Debug3DGUI.h"

void NavMesh::DebugRender() {
	BNS_VEC_FOREACH(tris) {
		DebugDrawTriangle(vertices.data[ptr->v0], vertices.data[ptr->v1], vertices.data[ptr->v2], Vector4(0.3f, 0.4f, 0.8f, 0.3f));
	}

	BNS_VEC_FOREACH(edges) {
		DebugDrawLine(ptr->v0, ptr->v1, Vector4(0.2f, 0.3f, 0.6f, 0.6f));
	}
}


