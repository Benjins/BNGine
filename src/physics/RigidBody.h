#ifndef RIGIDBODY_H
#define RIGIDBODY_H

#pragma once

#include "../core/Component.h"

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

	RigidBody() {
		rbFlags = RBF_None;
		mass = 1.0f;
	}
};


#endif
