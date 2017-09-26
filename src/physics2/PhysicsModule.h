#ifndef PHYSICS_MODULE_H
#define PHYSICS_MODULE_H

#pragma once

#include "../../ext/3dbasics/Vector3.h"
#include "../../ext/3dbasics/Quaternion.h"

#include "../../ext/CppUtils/vector.h"

struct Phys2RigidBody {
	unsigned int id;

	// TODO: Flags
	bool isActive;
	bool hasGravity;

	Vector3 position; // x
	Vector3 momentum; // p

	Vector3 velocity; // dx/dt

	Quaternion orientation; // r
	Quaternion spin; // ???????

	Vector3 angularVelocity; // dr/dt
	Vector3 angularMomentum; // O

	Vector3 accumulatedForces; // dp/dt
	Vector3 accumulatedTorque; // dO/dt 

	float inertiaTensor;
	float invInertiaTensor;
	float mass;
	float invMass;
};

void InitPhys2Rigidbody(Phys2RigidBody* rb, float mass, float boxSize);

struct Phys2BoxCollider {
	unsigned int id;
	Vector3 position;
	Vector3 size;
};

struct Phys2System {
	Vector<Phys2RigidBody> rigidbodies;
};

void Phys2StepRigidBody(Phys2RigidBody* rb, float dt);

void Phys2AddForceAtPoint(Phys2RigidBody* rb, Vector3 point, Vector3 force);

#endif
