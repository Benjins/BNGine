#include "PhysicsModule.h"

Vector3 phys2Gravity = Vector3(0.0f, -9.81f, 0.0f);// *0.5f;

void Phys2StepRigidBody(Phys2RigidBody* rb, float dt) {
	rb->momentum = rb->momentum + rb->accumulatedForces * dt;
	rb->velocity = rb->momentum * rb->invMass;
	rb->position = rb->position + rb->velocity * dt;

	rb->angularMomentum = rb->angularMomentum + rb->accumulatedTorque * dt;
	rb->angularVelocity = rb->angularMomentum * rb->invInertiaTensor;
	rb->spin = Quaternion(0, rb->angularVelocity.x, rb->angularVelocity.y, rb->angularVelocity.z) * rb->orientation * 0.5f;
	rb->orientation = rb->orientation + rb->spin * dt;
	rb->orientation.Normalize();

	rb->accumulatedTorque = Vector3(0, 0, 0);
	rb->accumulatedForces = Vector3(0, 0, 0);

	rb->angularMomentum = rb->angularMomentum * 0.99f;
	//rb->momentum = rb->momentum * 0.995f;
	rb->momentum = rb->momentum + phys2Gravity * dt * rb->mass;

	Vector3 normal;
	float depth;
	Vector3 contactPoints[4];
	int contactPointCount;
	if (Phys2RigidBodyHitGround(rb, 3.0f, contactPoints, &contactPointCount, &normal, &depth)) {
		Vector3 avgContactPoint;
		for (int i = 0; i < contactPointCount; i++) {
			avgContactPoint = avgContactPoint + contactPoints[i];
		}
		avgContactPoint = avgContactPoint / contactPointCount - rb->position;

		Vector3 fulcrumEnd = rb->position - avgContactPoint;
		//Phys2AddForceAtPoint(rb, fulcrumEnd, phys2Gravity * dt);

		//rb->accumulatedTorque = rb->accumulatedTorque + CrossProduct(rb->angularMomentum, avgContactPoint);

		rb->position = rb->position - normal * depth;
		rb->momentum = rb->momentum * -0.6f;
		Vector3 torqueForce = rb->momentum * -1;
		rb->accumulatedTorque = rb->accumulatedTorque + CrossProduct(torqueForce, avgContactPoint);
	}
}

void InitPhys2Rigidbody(Phys2RigidBody* rb, float mass, float boxSize) {
	rb->mass = mass;
	rb->invMass = 1.0f / mass;
	rb->boxSize = boxSize;
	rb->inertiaTensor = mass * boxSize * boxSize / 6.0f;
	rb->invInertiaTensor = 1.0f / rb->inertiaTensor;
}

void Phys2AddForceAtPoint(Phys2RigidBody* rb, Vector3 point, Vector3 force) {
	rb->accumulatedTorque = rb->accumulatedTorque + CrossProduct(force, point);
	rb->accumulatedForces = rb->accumulatedForces + force;
}

// Max of 4 contact points for outContacts
bool Phys2RigidBodyHitGround(Phys2RigidBody* rb, float groundHeight, Vector3* outContacts,
	int* outContactCount, Vector3* outNormal, float* depth) {
	Vector3 cornerPoints[8] = {
		Vector3( 1, -1, -1) * rb->boxSize,
		Vector3( 1, -1,  1) * rb->boxSize,
		Vector3( 1,  1, -1) * rb->boxSize,
		Vector3( 1,  1,  1) * rb->boxSize,
		Vector3(-1, -1,  1) * rb->boxSize,
		Vector3(-1, -1, -1) * rb->boxSize,
		Vector3(-1,  1,  1) * rb->boxSize,
		Vector3(-1,  1, -1) * rb->boxSize,
	};

	Vector3 trueCornerPos[8];

	// depth below ground is positive if below ground
	float depthBelowGround[8] = {};
	for (int i = 0; i < 8; i++) {
		trueCornerPos[i] = Rotate(cornerPoints[i], rb->orientation) + rb->position;
		depthBelowGround[i] = groundHeight - trueCornerPos[i].y;
	}

	float maxDepth = 0;
	int maxIndices[4] = { -1, -1, -1, -1 };
	int maxIndexCount = 0;
	for (int i = 0; i < 8; i++) {
		if (depthBelowGround[i] > maxDepth) {
			maxIndices[0] = i;
			maxIndexCount = 1;
			maxDepth = depthBelowGround[i];
		}
		else if (depthBelowGround[i] == maxDepth) {
			ASSERT(maxIndexCount < 4);
			maxIndices[maxIndexCount] = i;
			maxIndexCount++;
		}
	}

	if (maxIndexCount == 0) {
		return false;
	}
	else {
		*outContactCount = maxIndexCount;
		for (int i = 0; i < maxIndexCount; i++) {
			outContacts[i] = trueCornerPos[maxIndices[i]];
		}

		Vector3 normal;
		for (int i = 0; i < maxIndexCount; i++) {
			normal = normal + cornerPoints[maxIndices[i]];
		}

		normal = Rotate(normal, rb->orientation);
		normal.Normalize();

		*outNormal = normal;

		*depth = depthBelowGround[maxIndices[0]];

		return true;
	}
}

