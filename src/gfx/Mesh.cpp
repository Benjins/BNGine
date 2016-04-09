#include "Mesh.h"

#include "GLExtInit.h"

void Mesh::UploadToGfxDevice() {
	Vector<Vector3> positions;
	positions.EnsureCapacity(faces.count*3);
	for (int i = 0; i < faces.count; i++) {
		for (int j = 0; j < 3; j++) {
			positions.PushBack(vertices.data[faces.data[i].indices[j]].position);
		}
	}

	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(Vector3)*positions.count, positions.data, GL_STATIC_DRAW);
}