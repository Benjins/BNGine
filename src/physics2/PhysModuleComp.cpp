#include "PhysModuleComp.h"

void PhysModuleComp::Update() {
	BNS_VEC_FOREACH(sys.rigidbodies) {
		Phys2StepRigidBody(ptr, 0.01f);
	}

	float dt = GlobalScene->GetDeltaTime();
	const float speed = 4.0f;
	Transform* camTrans = GlobalScene->transforms.GetById(GlobalScene->cam.transform);
	if (GlobalScene->input.KeyIsDown('W')) {
		camTrans->position = camTrans->position + camTrans->Forward() * dt * speed;
	}
	if (GlobalScene->input.KeyIsDown('S')) {
		camTrans->position = camTrans->position - camTrans->Forward() * dt * speed;
	}
	if (GlobalScene->input.KeyIsDown('A')) {
		camTrans->position = camTrans->position - camTrans->Right() * dt * speed;
	}
	if (GlobalScene->input.KeyIsDown('D')) {
		camTrans->position = camTrans->position + camTrans->Right() * dt * speed;
	}

	if (GlobalScene->input.KeyIsDown('Q')) {
		camTrans->position = camTrans->position + camTrans->Up() * dt * speed;
	}
	if (GlobalScene->input.KeyIsDown('Z')) {
		camTrans->position = camTrans->position - camTrans->Up() * dt * speed;
	}

	if (GlobalScene->input.KeyIsDown('F')) {
		camTrans->rotation = camTrans->rotation * Quaternion(Y_AXIS, dt);
	}
	if (GlobalScene->input.KeyIsDown('G')) {
		camTrans->rotation = camTrans->rotation * Quaternion(Y_AXIS, -dt);
	}

	if (GlobalScene->input.MouseButtonIsReleased(PRIMARY)) {
		Vector3 localCoords = Vector3((rand() % 100) / 100.0f, (rand() % 100) / 100.0f, (rand() % 100) / 100.0f);
		localCoords[rand() % 3] = (rand() % 2) * 2 - 1;

		Phys2AddForceAtPoint(&sys.rigidbodies.data[0], localCoords, camTrans->Forward() * 10);
	}
}
