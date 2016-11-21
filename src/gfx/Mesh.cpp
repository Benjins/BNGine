#include "Mesh.h"

#include "GLExtInit.h"

Mesh::Mesh() {
	posVbo = -1;
	uvVbo = -1;
	normalVbo = -1;

	armatureId = -1;
}

void Mesh::UploadToGfxDevice() {
	Vector<Vector3> positionsData;
	positionsData.EnsureCapacity(faces.count*3);
	for (int i = 0; i < faces.count; i++) {
		for (int j = 0; j < 3; j++) {
			int posIdx = faces.data[i].posIndices[j];
			ASSERT(posIdx < positions.count);
			positionsData.PushBack(positions.data[posIdx]);
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

	Vector<Vector3> normalData;
	GenerateNormals(&normalData);

	glGenBuffers(1, &normalVbo);
	glBindBuffer(GL_ARRAY_BUFFER, normalVbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(Vector3)*normalData.count, normalData.data, GL_STATIC_DRAW);
}

void Mesh::GenerateNormals(Vector<Vector3>* outNormals) {
	Vector<Vector3> vertNormals;
	vertNormals.EnsureCapacity(positions.count);
	for (int i = 0; i < positions.count; i++) {
		Vector3 totalNormal;
		int normalCount = 0;
		for (int j = 0; j < faces.count; j++) {
			bool isPartOfFace = false;
			for (int k = 0; k < 3; k++) {
				if (faces.data[j].posIndices[k] == i) {
					isPartOfFace = true;
					break;
				}
			}

			if (isPartOfFace) {
				Vector3 facePositions[3];
				for (int k = 0; k < 3; k++) {
					facePositions[k] = positions.data[faces.data[j].posIndices[k]];
				}

				Vector3 edge1 = facePositions[0] - facePositions[1];
				Vector3 edge2 = facePositions[0] - facePositions[2];

				Vector3 normal = CrossProduct(edge1, edge2);

				totalNormal = totalNormal + normal.Normalized();
				normalCount++;
			}
		}

		vertNormals.PushBack(totalNormal / normalCount);
	}

	outNormals->Clear();
	outNormals->EnsureCapacity(faces.count * 3);
	for (int i = 0; i < faces.count; i++) {
		for (int j = 0; j < 3; j++) {
			outNormals->PushBack(vertNormals.data[faces.data[i].posIndices[j]]);
		}
	}
}

void Mesh::Destroy() {
	if ((int)posVbo != -1) {
		glDeleteBuffers(1, &posVbo);
		posVbo = -1;
	}

	if ((int)uvVbo != -1) {
		glDeleteBuffers(1, &uvVbo);
		uvVbo = -1;
	}

	if ((int)normalVbo != -1) {
		glDeleteBuffers(1, &normalVbo);
		normalVbo = -1;
	}
}

Mesh::~Mesh() {
	Destroy();
}
