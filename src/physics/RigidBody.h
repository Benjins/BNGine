#ifndef RIGIDBODY_H
#define RIGIDBODY_H

#pragma once

#include "../core/Component.h"

#include "../../ext/3dbasics/Vector3.h"

/*[Flags]*/
enum RigidBodyFlags {
	/*[NoneFlag]*/
	RBF_None = 0,
	RBF_HasGravity = (1 << 0),
	RBF_IsEnabled  = (1 << 1),
	RBF_IsSleeping = (1 << 2),
	RBF_IsGrounded = (1 << 3)
};

struct RigidBody : Component {
	RigidBodyFlags rbFlags;
	float mass;

	Vector3 velocity;
	Vector3 acceleration;

	float maxAirSpeed;
	float bounceVelocityRatio;

	Vector3 cachedCentreOfMass;
	bool centreOfMassIsCached;

	RigidBody() {
		rbFlags = RBF_None;
		mass = 1.0f;
		centreOfMassIsCached = false;
		maxAirSpeed = 4.0f;
		bounceVelocityRatio = 0.3f;
	}

	void SimulateFrame(float dt, Vector3* outTranslation) {
		velocity = velocity + acceleration * dt;
		// TODO: Exceptions? Only for gravity?
		if (velocity.Magnitude() > maxAirSpeed) {
			velocity = velocity.Normalized() * maxAirSpeed;
		}
		*outTranslation = velocity * dt;
	}

	void AddForceAtCentre(Vector3 force, float dt) {
		acceleration = acceleration + force * dt;
	}

	inline Vector3 GetMomentum() const {
		return velocity * mass;
	}

	// TODO: Point should be in local coords I guess?
	// Maybe we can switch to global if that's hard
	void AddForceAtPoint(Vector3 force, Vector3 point, float dt) {

	}

	Vector3 GetCentreOfMass();
};


#endif
