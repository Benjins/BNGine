#ifndef ANIMATIONS_H
#define ANIMATIONS_H

#include "../../ext/CppUtils/vector.h"
#include "../../gen/ComponentType.h"

#include "../core/Component.h"

#include "../gfx/Material.h"

#include "../../ext/CppUtils/disc_union.h"

enum AnimationType {
	ANT_Float,
	ANT_Vec2,
	ANT_Vec3,
	ANT_Vec4,
	ANT_Quaternion,
	ANT_Count
};

extern int animTypeKeySize[ANT_Count];

struct AnimationTrack : IDBase {
	Vector<float> data;
	Vector<float> times;
	AnimationType type;

	float GetLength() const;

	int Calculate(float evalTime, float* outVal);
};

enum AnimationTargetType {
	ATT_Transform,
	ATT_BoneTransform,
	ATT_Uniform,
	ATT_ComponentField,
	ATT_Count
};

enum Animation3DTarget {
	A3DT_Position,
	A3DT_Rotation,
	A3DT_Scale
};

struct Transform;
struct Armature;

struct AnimationTransformTarget {
	Animation3DTarget target3d;
	IDHandle<Transform> transformId;
};

struct AnimationBoneTarget {
	Animation3DTarget target3d;
	IDHandle<Armature> armId;
	int boneIndex;
};

struct AnimationUniformTarget {
	IDHandle<Material> matId;
	UniformType uniformType;
	// TODO: Some better way of doing this?
	char uniformName[16];
};

struct AnimationCustomFieldTarget {
	CustomComponentType compType;
	int compId;
	int fieldOffset;
};

#define AT_(mac) \
	mac(AnimationTransformTarget) \
	mac(AnimationBoneTarget) \
	mac(AnimationUniformTarget) \
	mac(AnimationCustomFieldTarget)


DEFINE_DISCRIMINATED_UNION(AnimationTarget, AT_);

#undef AT_

void ResolveAnimationTarget(const AnimationTarget* target, const float* data, int size);

struct AnimationInstance : Component {
	bool shouldLoop;
	bool autoPlay;

	/*[DoNotSerialize]*/
	float currTime;

	/*[DoNotSerialize]*/
	bool isPlaying;

	/*[SerializeFromId("src", "anim")]*/
	IDHandle<AnimationTrack> animId;

	AnimationTarget target;

	void Start();

	void Update();
};

struct AnimationSystem {
	IDTracker<AnimationInstance> animInsts;
};

#endif
