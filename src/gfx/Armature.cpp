#include "Armature.h"

#include "GLExtInit.h"

#include "../../ext/CppUtils/bitset.h"

Mat4x4 Armature::GetBoneMatrix(int index) {
	Mat4x4 mat = LocRotScaleToMat(bones[index].pos, bones[index].rot, bones[index].scale);
	if (bones[index].parent >= 0) {
		Mat4x4 parentMat = GetBoneMatrix(bones[index].parent);
		mat = parentMat * mat;
	}

	return mat;
}

void Armature::CalculateBoneMatrices(Mat4x4* mats) {
	for (int i = 0; i < boneCount; i++) {
		mats[i] = GetBoneMatrix(i);
	}
}

void Armature::UploadDataToGfxDevice() {
	glGenBuffers(1, &boneWeightsVbo);
	glBindBuffer(GL_ARRAY_BUFFER, boneWeightsVbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float)*boneWeights.count, boneWeights.data, GL_STATIC_DRAW);

	glGenBuffers(1, &boneIndicesVbo);
	glBindBuffer(GL_ARRAY_BUFFER, boneIndicesVbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float)*boneIndices.count, boneIndices.data, GL_STATIC_DRAW);
}