
#include "Player.h"

#include "../core/Scene.h"

#include "../net/NetworkSystem.h"

void PlayerComponent::Start() {
	//GuiFormData* form = GlobalScene->gui.guiFormStack.CreateAndAdd();
	//form->type = GFT_GuiHealthForm;
	//
	//GuiFormData* form2 = GlobalScene->gui.guiFormStack.CreateAndAdd();
	//form2->type = GFT_GuiIPConnectForm;
}

void PlayerComponent::Update() {
	float floorHeight = -10;

	//GuiFormData* form = &GlobalScene->gui.guiFormStack.vals[0];
	//form->GuiHealthForm_Data.health = currHealth;
	//form->GuiHealthForm_Data.maxHealth = maxHealth;

	Entity* ent = GlobalScene->entities.GetById(entity);
	Transform* entTrans = GlobalScene->transforms.GetById(ent->transform);
	Transform* camTrans = GlobalScene->transforms.GetById(GlobalScene->cam.transform);

	RaycastHit downCast = GlobalScene->phys.Raycast(entTrans->GetGlobalPosition() + Vector3(0, 0.1f - playerHeight, 0), Y_AXIS * -1);

	if (downCast.wasHit) {
		floorHeight = downCast.globalPos.y + playerHeight;
	}

	entTrans->rotation = Quaternion(Y_AXIS, GlobalScene->input.cursorX / 80);
	camTrans->rotation = Quaternion(X_AXIS, GlobalScene->input.cursorY / 80 - 2);

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

	if (GlobalScene->input.MouseButtonIsReleased(PRIMARY)) {
		int prefId = -1;
		GlobalScene->res.assetIdMap.LookUp("bullet.bnp", &prefId);

		Prefab* pref = GlobalScene->res.prefabs.GetByIdNum(prefId);
		ASSERT(pref != nullptr);

		Vector3 bulletSpawnPosition = entTrans->GetGlobalPosition() + entTrans->Forward() * 0.21f;
		Entity* bullet = pref->Instantiate(bulletSpawnPosition, entTrans->rotation * camTrans->rotation);

		PrefabInstanceComponent* instComp = FIND_COMPONENT_BY_ENTITY(PrefabInstanceComponent, GET_PTR_HANDLE(bullet));
		ASSERT(instComp != nullptr);

		GlobalScene->net.RegisterSpawnedEntity(bullet);

		// TODO: Instantiating a prefab invalidates pointers on realloc, should look into that
		ent = GlobalScene->entities.GetById(entity);
		entTrans = GlobalScene->transforms.GetById(ent->transform);
		camTrans = GlobalScene->transforms.GetById(GlobalScene->cam.transform);
	}

	// States can use this to restore vertical movement, defaults to none
	float oldMoveVecY = moveVec.y;
	moveVec.y = 0;

	static const float gravity = 0.08f;
	static const float waterMoveVerticalSpeed = 1.5f;
	static const float waterMaxHorizontalSpeed = 0.3f;

	if (currState == CS_GROUNDED) {
		Vector3 newPos = entTrans->GetGlobalPosition() + moveVec;
		RaycastHit newDownCast = GlobalScene->phys.Raycast(newPos, Y_AXIS * -1);

		const float floorStickHeight = 0.1f;

		if (newDownCast.wasHit && newDownCast.depth < playerHeight + floorStickHeight) {
			float heightShift = playerHeight - newDownCast.depth;
			moveVec.y = heightShift;
		}

		if (entTrans->position.y > floorHeight + 0.005f) {
			currState = CS_FALLING;
		}
		else if (GlobalScene->input.KeyIsDown(KC_Space)) {
			yVelocity = jumpVelocity;
			currState = CS_JUMPING;
		}
		else if (CheckLadder(entTrans->position + moveVec)) {
			currState = CS_LADDERCLIMB;
		}
	}
	else if (currState == CS_JUMPING) {
		yVelocity -= gravity;
		moveVec.y = yVelocity / 50.0f;

		if (yVelocity < 0) {
			currState = CS_FALLING;
		}
		else if (CheckLadder(entTrans->position + moveVec)) {
			currState = CS_LADDERCLIMB;
		}
	}
	else if (currState == CS_FALLING) {
		yVelocity -= gravity;
		moveVec.y = yVelocity * GlobalScene->GetDeltaTime();

		if (entTrans->position.y + moveVec.y < floorHeight) {
			currState = CS_GROUNDED;
			currHealth--;
			yVelocity = 0;
			moveVec.y = 0;
			entTrans->position.y = floorHeight;
		}

		bool inWater = CheckWater(entTrans->position + moveVec);

		if (inWater) {
			currState = CS_FALLINGWATER;
		}
		else if (CheckLadder(entTrans->position + moveVec)) {
			currState = CS_LADDERCLIMB;
		}
	}
	else if (currState == CS_FALLINGWATER) {
		yVelocity += gravity / 4;

		if (GlobalScene->input.KeyIsDown('Q')) {
			yVelocity += waterMoveVerticalSpeed * GlobalScene->GetDeltaTime();
		}
		if (GlobalScene->input.KeyIsDown('Z')) {
			yVelocity -= waterMoveVerticalSpeed * GlobalScene->GetDeltaTime();
		}

		moveVec.y = yVelocity * GlobalScene->GetDeltaTime();

		Vector2 lateralMotion = Vector2(moveVec.x, moveVec.z);
		float lateralMag = lateralMotion.Magnitude();
		if (lateralMag > waterMaxHorizontalSpeed) {
			lateralMotion = lateralMotion / (lateralMag / waterMaxHorizontalSpeed);
			lateralMotion = lateralMotion * lateralMag;
			moveVec.x = lateralMotion.x;
			moveVec.x = lateralMotion.y;
		}

		if (entTrans->position.y + moveVec.y < floorHeight) {
			yVelocity = 0;
			moveVec.y = 0;
			entTrans->position.y = floorHeight;
		}

		if (yVelocity >= 0) {
			currState = CS_RISINGWATER;
		}
		else if (CheckLadder(entTrans->position + moveVec)) {
			currState = CS_LADDERCLIMB;
		}
	}
	else if (currState == CS_RISINGWATER) {
		yVelocity += gravity / 10;

		if (GlobalScene->input.KeyIsDown('Q')) {
			yVelocity += waterMoveVerticalSpeed * GlobalScene->GetDeltaTime();
		}
		if (GlobalScene->input.KeyIsDown('Z')) {
			yVelocity -= waterMoveVerticalSpeed * GlobalScene->GetDeltaTime();
		}

		static float maxVelocity = 0.4f;
		yVelocity = BNS_MIN(yVelocity, maxVelocity);

		if (yVelocity < 0) {
			currState = CS_FALLINGWATER;
		}

		moveVec.y = yVelocity * GlobalScene->GetDeltaTime();

		Vector2 lateralMotion = Vector2(moveVec.x, moveVec.z);
		float lateralMag = lateralMotion.Magnitude();
		if (lateralMag > waterMaxHorizontalSpeed) {
			lateralMotion = lateralMotion / (lateralMag / waterMaxHorizontalSpeed);
			lateralMotion = lateralMotion * lateralMag;
			moveVec.x = lateralMotion.x;
			moveVec.z = lateralMotion.y;
		}

		bool inWater = CheckWater(entTrans->position + moveVec);

		if (CheckLadder(entTrans->position + moveVec)) {
			currState = CS_LADDERCLIMB;
		}
		else if (!inWater) {
			currState = CS_FALLING;
		}
	}
	else if (currState == CS_LADDERCLIMB) {
		moveVec.y = oldMoveVecY;
		if (GlobalScene->input.KeyIsDown(KC_Space)) {
			yVelocity = jumpVelocity;
			currState = CS_JUMPING;
		}
		else if (!CheckLadder(entTrans->position + moveVec)) {
			currState = CS_FALLING;
		}
	}
	else {
		ASSERT_WARN("'%s': Yo, currState is all out of wack.", __FUNCTION__);
	}

	//float heightDiff = moveVec.y;
	//moveVec.y = 0;

	RaycastHit moveCast = GlobalScene->phys.Raycast(entTrans->position, moveVec.Normalized());
	if (moveCast.wasHit && moveCast.depth <= playerWidth + moveVec.Magnitude()) {
		Vector3 goodVec = moveVec.Normalized() * (moveCast.depth - playerWidth - 0.001f);
		Vector3 badVec = moveVec - goodVec;

		Vector3 projectedVec = badVec - VectorProject(badVec, moveCast.globalNormal);
		moveVec = goodVec + projectedVec;
	}

	//moveVec.y = heightDiff;

	entTrans->position = entTrans->position + moveVec;
}

bool PlayerComponent::CheckWater(Vector3 pos) {
	for (int i = 0; i < GlobalScene->gameplay.waterComps.currentCount; i++) {
		if (GlobalScene->gameplay.waterComps.vals[i].IsInside(pos)) {
			return true;
		}
	}

	return false;
}

bool PlayerComponent::CheckLadder(Vector3 pos) {
	for (int i = 0; i < GlobalScene->gameplay.ladderComps.currentCount; i++) {
		if (GlobalScene->gameplay.ladderComps.vals[i].IsInside(pos)) {
			return true;
		}
	}

	return false;
}
