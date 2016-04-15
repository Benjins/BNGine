#include "Mesh.h"

#include "GLExtInit.h"

void Mesh::UploadToGfxDevice() {
	Vector<Vector3> positionsData;
	positionsData.EnsureCapacity(faces.count*3);
	for (int i = 0; i < faces.count; i++) {
		for (int j = 0; j < 3; j++) {
			positionsData.PushBack(positions.data[faces.data[i].posIndices[j]]);
		}
	}

	glGenBuffers(1, &posVbo);
	glBindBuffer(GL_ARRAY_BUFFER, posVbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(Vector3)*positionsData.count, positionsData.data, GL_STATIC_DRAW);

	if (uvs.count > 0) {
		Vector<Vector2> uvData;
		uvData.EnsureCapacity(faces.count * 3);
		for (int i = 0; i < faces.count; i++) {
			for (int j = 0; j < 3; j++) {
				uvData.PushBack(uvs.data[faces.data[i].uvIndices[j]]);
			}
		}

		glGenBuffers(1, &uvVbo);
		glBindBuffer(GL_ARRAY_BUFFER, uvVbo);
		glBufferData(GL_ARRAY_BUFFER, sizeof(Vector2)*uvData.count, uvData.data, GL_STATIC_DRAW);
	}
}