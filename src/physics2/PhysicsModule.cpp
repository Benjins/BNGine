#include "PhysicsModule.h"

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

	rb->angularMomentum = rb->angularMomentum * 0.995f;
	rb->momentum = rb->momentum * 0.995f;
}

void InitPhys2Rigidbody(Phys2RigidBody* rb, float mass, float boxSize) {
	rb->mass = mass;
	rb->invMass = 1.0f / mass;
	rb->inertiaTensor = mass * boxSize * boxSize / 6.0f;
	rb->invInertiaTensor = 1.0f / rb->inertiaTensor;
}

void Phys2AddForceAtPoint(Phys2RigidBody* rb, Vector3 point, Vector3 force) {
	rb->accumulatedTorque = rb->accumulatedTorque + CrossProduct(force, point);
	rb->accumulatedForces = rb->accumulatedForces + force;
}

