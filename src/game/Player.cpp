
#include "Player.h"

#include "../core/Scene.h"

void Player::Update() {
	float floorHeight = -2;

	Transform* camTrans = GlobalScene->transforms.GetById(GlobalScene->cam.transform);

	RaycastHit downCast = GlobalScene->phys.Raycast(camTrans->GetGlobalPosition() + Vector3(0, 0.1f - playerHeight, 0), Y_AXIS * -1);

	if (downCast.wasHit) {
		floorHeight = downCast.globalPos.y + playerHeight;
	}

	camTrans->rotation = Quaternion(Y_AXIS, GlobalScene->input.cursorX / 80) * Quaternion(X_AXIS, GlobalScene->input.cursorY / 80 - 2);

	Vector3 moveVec;

	const float movementSpeed = 3.0f;

	if (GlobalScene->input.KeyIsDown('W')) {
		moveVec = moveVec + camTrans->Forward() * GlobalScene->GetDeltaTime() * movementSpeed;
	}
	if (GlobalScene->input.KeyIsDown('S')) {
		moveVec = moveVec - camTrans->Forward() * GlobalScene->GetDeltaTime() * movementSpeed;
	}
	if (GlobalScene->input.KeyIsDown('A')) {
		moveVec = moveVec - camTrans->Right() * GlobalScene->GetDeltaTime() * movementSpeed;
	}
	if (GlobalScene->input.KeyIsDown('D')) {
		moveVec = moveVec + camTrans->Right() * GlobalScene->GetDeltaTime() * movementSpeed;
	}

	moveVec.y = 0;

	if (currState == CS_GROUNDED) {
		Vector3 newPos = camTrans->GetGlobalPosition() + moveVec;
		RaycastHit newDownCast = GlobalScene->phys.Raycast(newPos, Y_AXIS * -1);

		const float floorStickHeight = 0.1f;

		if (newDownCast.wasHit && newDownCast.depth < playerHeight + floorStickHeight) {
			float heightShift = playerHeight - newDownCast.depth;
			moveVec.y = heightShift;
		}

		if (camTrans->position.y > floorHeight) {
			currState = CS_FALLING;
		}
		
		if (GlobalScene->input.KeyIsDown(KC_Space)) {
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
		const float fallingSpeed = 1.0f;
		moveVec.y = yVelocity * GlobalScene->GetDeltaTime() * fallingSpeed;

		if (camTrans->position.y + moveVec.y < floorHeight) {
			currState = CS_GROUNDED;
			yVelocity = 0;
			moveVec.y = 0;
			camTrans->position.y = floorHeight;
		}
	}

	float heightDiff = moveVec.y;
	moveVec.y = 0;

	RaycastHit moveCast = GlobalScene->phys.Raycast(camTrans->position, moveVec.Normalized());
	if (moveCast.wasHit && moveCast.depth <= playerWidth + moveVec.Magnitude()) {
		Vector3 goodVec = moveVec.Normalized() * (moveCast.depth - playerWidth - 0.001f);
		Vector3 badVec = moveVec - goodVec;

		Vector3 projectedVec = badVec - VectorProject(badVec, moveCast.globalNormal);
		moveVec = goodVec + projectedVec;
	}

	moveVec.y = heightDiff;

	camTrans->position = camTrans->position + moveVec;
}
