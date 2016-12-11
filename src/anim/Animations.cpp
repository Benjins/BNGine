#include "Animations.h"

#include "../core/Scene.h"

#include "../../ext/CppUtils/assert.h"

int animTypeKeySize[ANT_Count] = {
	/*ANT_Float*/ 4,
	/*ANT_Vec2*/ 8,
	/*ANT_Vec3*/ 12,
	/*ANT_Vec4*/ 16,
	/*ANT_Quaternion*/ 16
};

float AnimationTrack::GetLength() const {
	float lastKey = times.Back();

	return lastKey;
}

inline float InterpolateFloat(float a, float b, float val) {
	ASSERT_MSG(val >= 0 && val <= 1, "Val %f is out of range for interpolation.", val);
	return a * (1 - val) + b * val;
}

inline Vector2 InterpolateVector2(Vector2 a, Vector2 b, float val) {
	ASSERT_MSG(val >= 0 && val <= 1, "Val %f is out of range for interpolation.", val);
	return a * (1 - val) + b * val;
}

inline Vector3 InterpolateVector3(Vector3 a, Vector3 b, float val) {
	ASSERT_MSG(val >= 0 && val <= 1, "Val %f is out of range for interpolation.", val);
	return a * (1 - val) + b * val;
}

inline Vector4 InterpolateVector4(Vector4 a, Vector4 b, float val) {
	ASSERT_MSG(val >= 0 && val <= 1, "Val %f is out of range for interpolation.", val);
	return a * (1 - val) + b * val;
}

inline Quaternion InterpolateQuaternion(Quaternion a, Quaternion b, float val) {
	ASSERT_MSG(val >= 0 && val <= 1, "Val %f is out of range for interpolation.", val);
	return (a * (1 - val) + b * val).Normalized();
}

int AnimationTrack::Calculate(float evalTime, float* outVal) {
	ASSERT_MSG(evalTime >= 0 && evalTime <= GetLength(),
		"Animation '%s' evaluated at time %f, which is beyond its range.",
		GlobalScene->res.FindFileNameByIdAndExtension("anim", id).string,
		evalTime);

	float time = 0.0f;
	for (int i = 0; i < times.count; i++) {
		time = times.Get(i);
		if (time >= evalTime) {
			if (i == 0) {
				if (type == ANT_Float) {
					BNS_MEMCPY(outVal, data.data, sizeof(float));
					return sizeof(float);
				}
				else if (type == ANT_Vec2) {
					BNS_MEMCPY(outVal, data.data, sizeof(Vector2));
					return sizeof(Vector2);
				}
				else if (type == ANT_Vec3) {
					BNS_MEMCPY(outVal, data.data, sizeof(Vector3));
					return sizeof(Vector3);
				}
				else if (type == ANT_Vec4 || type == ANT_Quaternion) {
					BNS_MEMCPY(outVal, data.data, sizeof(Vector4));
					return sizeof(Vector4);
				}
				else {
					ASSERT_WARN("%s", "Incorrect animation type.");
				}
			}
			else {
				float firstTime  = times.Get(i - 1);
				float secondTime = times.Get(i);

				float ratio = (evalTime - firstTime) / (secondTime - firstTime);

				#define INTERPOLATE(enumType, dataType, interpFunc)    \
					if (type == enumType){                             \
						dataType v1 = ((dataType *)data.data)[i-1];    \
						dataType v2 = ((dataType *)data.data)[i];      \
						dataType interp = interpFunc(v1, v2, ratio);   \
						*(dataType*)outVal = interp;                   \
						return sizeof(dataType);                       \
					}

				INTERPOLATE(ANT_Float, float, InterpolateFloat)
				else INTERPOLATE(ANT_Vec2, Vector2, InterpolateVector2)
				else INTERPOLATE(ANT_Vec3, Vector3, InterpolateVector3)
				else INTERPOLATE(ANT_Vec4, Vector4, InterpolateVector4)
				else INTERPOLATE(ANT_Quaternion, Quaternion, InterpolateQuaternion)
				else {
					ASSERT_WARN("%s", "Incorrect animation type.");
				}

				#undef INTERPOLATE
			}
			break;
		}

	}

	ASSERT(false);
	return 0;
}

void ResolveAnimationTarget(const AnimationTarget* target, const float* data, int size) {
	#define TARGET_3D_CHECK(target3d) \
		do { \
			if ((target3d) == A3DT_Position){ \
				ASSERT(size == sizeof(Vector3));\
			} \
			else if ((target3d) == A3DT_Rotation){ \
				ASSERT(size == sizeof(Quaternion));\
			} \
			else if ((target3d) == A3DT_Scale){ \
				ASSERT(size == sizeof(Vector3));\
			} \
			else{ASSERT_WARN("%s", "Target 3d type is incorrect.");}\
		} while(0)

	
	switch (target->type) {

	case AnimationTarget::UE_AnimationTransformTarget: {
		AnimationTransformTarget* trans = (AnimationTransformTarget*)&target->AnimationTransformTarget_data;
		Transform* transform = GlobalScene->transforms.GetById(trans->transformId);
		TARGET_3D_CHECK(trans->target3d);

		     if (trans->target3d == A3DT_Position) { transform->position = *(Vector3*)data; }
		else if (trans->target3d == A3DT_Rotation) { transform->rotation = *(Quaternion*)data; }
		else if (trans->target3d == A3DT_Scale)    { transform->scale    = *(Vector3*)data; }
	} break;

	case AnimationTarget::UE_AnimationBoneTarget: {
		AnimationBoneTarget* bone = (AnimationBoneTarget*)&target->AnimationBoneTarget_data;
		Armature* arm = GlobalScene->res.armatures.GetById(bone->armId);
		ASSERT(bone->boneIndex >= 0 && bone->boneIndex < arm->boneCount);

		TARGET_3D_CHECK(bone->target3d);

		BoneTransform* bt = &arm->bones[bone->boneIndex];

		     if (bone->target3d == A3DT_Position) { bt->pos   = *(Vector3*)data; }
		else if (bone->target3d == A3DT_Rotation) { bt->rot   = *(Quaternion*)data; }
		else if (bone->target3d == A3DT_Scale)    { bt->scale = *(Vector3*)data; }

	} break;

	case AnimationTarget::UE_AnimationUniformTarget: {
		AnimationUniformTarget* uniform = (AnimationUniformTarget*)&target->AnimationUniformTarget_data;
		Material* mat = GlobalScene->res.materials.GetById(uniform->matId);

		switch (uniform->uniformType) {
		case UT_FLOAT: {
			ASSERT(size == sizeof(float));
			mat->SetFloatUniform(uniform->uniformName, *(float*)data);
		} break;
		case UT_VEC2: {
			ASSERT(size == sizeof(Vector2));
			mat->SetVector2Uniform(uniform->uniformName, *(Vector2*)data);
		} break;
		case UT_VEC3: {
			ASSERT(size == sizeof(Vector3));
			mat->SetVector3Uniform(uniform->uniformName, *(Vector3*)data);
		} break;
		case UT_VEC4: {
			ASSERT(size == sizeof(Vector4));
			mat->SetVector4Uniform(uniform->uniformName, *(Vector4*)data);
		} break;
		default: {
			ASSERT_WARN("%s", "Incorrect uniform type.")
		} break;
		}

	} break;

	case AnimationTarget::UE_AnimationCustomFieldTarget: {
		AnimationCustomFieldTarget* component = (AnimationCustomFieldTarget*)&target->AnimationCustomFieldTarget_data;
		Component* comp = getComponentFuncs[component->compType](component->compId);
		char* fieldPtr = ((char*)comp) + component->fieldOffset;
		BNS_MEMCPY(fieldPtr, data, size);
	} break;

	default: {
		ASSERT_WARN("%s", "Invalid target type for animation target.");
	} break;

	}
}

void AnimationInstance::Start() {
	isPlaying = autoPlay;
	currTime = 0.0f;
}

void AnimationInstance::Update() {
	if (animId.id == 0xFFFFFFFF) {
		return;
	}
	if (isPlaying) {
		// The max number of channels we care about is 4
		float data[4];
		AnimationTrack* anim = GlobalScene->res.anims.GetById(animId);
		int dataSize = anim->Calculate(currTime, data);

		ResolveAnimationTarget(&target, data, dataSize);

		currTime += GlobalScene->GetDeltaTime();

		if (shouldLoop) {
			float animTime = anim->GetLength();
			while (currTime > animTime) {
				currTime -= animTime;
			}
		}
	}
}
