#ifndef LEVEL_H
#define LEVEL_H

#pragma once

#include "../core/Camera.h"

#include "../../ext/CppUtils/idbase.h"
#include "../../ext/CppUtils/vector.h"

struct Transform;
struct Entity;
struct BoxCollider;
struct SphereCollider;
struct MeshCollider;
struct PlayerComponent;
struct BulletComponent;
struct EnemyComponent;
struct HealthComponent;
struct WaterVolumeComponent;
struct LadderComponent;
struct ScriptInstance;
struct AnimationInstance;
struct Prefab;
struct PrefabInstanceComponent;
struct PatrolPointComponent;
struct RigidBody;

struct PrefabInstance {
	Quaternion rot;
	Vector3 pos;
	int parentTransform;
	IDHandle<Prefab> prefabId;
	uint32 instanceId;
	uint32 instanceTransformId;
};

struct Level : IDBase{
	Camera cam;
	Vector<Transform> transforms;
	Vector<Entity> entities;
	Vector<PrefabInstance> prefabInsts;

	Vector<int> meshIds;
	Vector<int> matIds;

	// TODO: Automate
	Vector<BoxCollider> boxCols;
	Vector<SphereCollider> sphereCols;
	Vector<MeshCollider> meshCols;
	Vector<PlayerComponent> playerComps;
	Vector<BulletComponent> bulletComps;
	Vector<EnemyComponent> enemyComps;
	Vector<HealthComponent> healthComps;
	Vector<WaterVolumeComponent> waterComps;
	Vector<LadderComponent> ladderComps;
	Vector<ScriptInstance> scriptInsts;
	Vector<AnimationInstance> animInsts;
	Vector<PrefabInstanceComponent> prefabInstComps;
	Vector<PatrolPointComponent> patrolPoints;
	Vector<RigidBody> rigidBodies;
};

#endif
