#ifndef ARMATURE_H
#define ARMATURE_H

#pragma once

#include "../core/Transform.h"

#include "../../ext/CppUtils/idbase.h"
#include "../../ext/CppUtils/vector.h"

#include "../../ext/3dbasics/Vector3.h"
#include "../../ext/3dbasics/Quaternion.h"

#define MAX_BONE_NAME_LENGTH 16

struct BoneTransform {
	char name[MAX_BONE_NAME_LENGTH];
	Vector3 pos;
	Quaternion rot;
	Vector3 scale;

	int parent;
};

#define MAX_BONES_PER_VERTEX 4

#define MAX_BONE_COUNT 32

struct BoneAnimTracks {
	int posTrack;
	int rotTrack;
	int scaleTrack;
};

struct Mesh;

struct Armature : IDBase {
	Mat4x4 inverseBindPoses[MAX_BONE_COUNT];
	BoneTransform bones[MAX_BONE_COUNT];
	BoneAnimTracks boneTrackData[MAX_BONE_COUNT];
	int boneCount;

	Vector<float> boneWeights;
	Vector<float> boneIndices;

	IDHandle<Mesh> modelId;

	GLuint boneWeightsVbo;
	GLuint boneIndicesVbo;

	BoneTransform* AddBone() {
		ASSERT(boneCount < MAX_BONE_COUNT);
		BoneTransform* bone = &bones[boneCount];
		boneCount++;
		return bone;
	}

	void CalculateBoneMatrices(Mat4x4* mats);
	Mat4x4 GetBoneMatrix(int index);

	void UploadDataToGfxDevice();
};


#endif
