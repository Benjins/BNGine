
#include "Player.h"

#include "../core/Scene.h"

void Player::Update() {
	float floorHeight = -2;

	Transform* camTrans = GlobalScene->cam.transform;

	RaycastHit downCast = GlobalScene->phys.Raycast(camTrans->GetGlobalPosition() + Vector3(0, 0.1f - playerHeight, 0), Y_AXIS * -1);

	if (downCast.wasHit) {
		floorHeight = downCast.globalPos.y + playerHeight;
	}
	else {
		int xx = 9;
		BNS_UNUSED(xx);
	}

	camTrans->rotation = Quaternion(Y_AXIS, GlobalScene->input.cursorX / 80) * Quaternion(X_AXIS, GlobalScene->input.cursorY / 80 - 2);

	Vector3 moveVec;

	if (GlobalScene->input.KeyIsDown('W')) {
		moveVec = moveVec + camTrans->Forward() / 30.0f;
	}
	if (GlobalScene->input.KeyIsDown('S')) {
		moveVec = moveVec - camTrans->Forward() / 30.0f;
	}
	if (GlobalScene->input.KeyIsDown('A')) {
		moveVec = moveVec - camTrans->Right() / 30.0f;
	}
	if (GlobalScene->input.KeyIsDown('D')) {
		moveVec = moveVec + camTrans->Right() / 30.0f;
	}

	moveVec.y = 0;

	if (currState == CS_GROUNDED) {
		if (camTrans->position.y > floorHeight) {
			currState = CS_FALLING;
		}
		else if (GlobalScene->input.KeyIsDown(' ')) {
			yVelocity = jumpVelocity;
			currState = CS_JUMPING;
		}
	}
	else if (currState == CS_JUMPING) {
		yVelocity -= 0.05f;
		moveVec.y = yVelocity / 50.0f;

		if (yVelocity < 0) {
			currState = CS_FALLING;
		}
	}
	else if (currState == CS_FALLING) {
		yVelocity -= 0.05f;
		moveVec.y = yVelocity / 50.0f;

		if (camTrans->position.y + moveVec.y < floorHeight) {
			currState = CS_GROUNDED;
			yVelocity = 0;
			moveVec.y = 0;
			camTrans->position.y = floorHeight;
		}
	}

	camTrans->position = camTrans->position + moveVec;
}