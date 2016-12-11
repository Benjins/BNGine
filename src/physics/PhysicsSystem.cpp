#include "PhysicsSystem.h"

#include "../core/Scene.h"

#include "../../ext/CppUtils/macros.h"

#include <cfloat>

float PhysicsSystem::fixedTimestep = 0.02f;

RaycastHit PhysicsSystem::Raycast(Vector3 origin, Vector3 direction) {
	RaycastHit finalHit;
	finalHit.wasHit = false;
	finalHit.depth = FLT_MAX;
	for (int i = 0; i < boxCols.currentCount; i++) {
		if (!boxCols.vals[i].isTrigger) {
			RaycastHit boxHit = RaycastBox(origin, direction, &boxCols.vals[i]);

			if (boxHit.wasHit && boxHit.depth < finalHit.depth) {
				finalHit = boxHit;
			}
		}
	}

	return finalHit;
}

void PhysicsSystem::AdvanceTime(float time) {
	timeOffset += time;
	while (timeOffset >= fixedTimestep) {
		StepFrame(fixedTimestep);
		timeOffset -= fixedTimestep;
	}

	EndFrame();
}

void PhysicsSystem::StepFrame(float dt) {
	collisions.Clear();
	for (int i = 0; i < boxCols.currentCount; i++) {
		for (int j = 0; j < boxCols.currentCount; j++) {
			if (i != j) {
				Collision col = BoxBoxCollision(boxCols.vals[i], boxCols.vals[j]);
				if (col.isColliding) {
					collisions.PushBack(col);
					IDHandle<Entity> entity1 = boxCols.GetByIdNum(col.colId1)->entity;
					GlobalScene->SendCollisionToCustomComponents(entity1, col);

					Collision col2 = col;
					col2.colId2 = col.colId1;
					col2.colId1 = col.colId2;
					col2.colType1 = col.colType2;
					col2.colType2 = col.colType1;

					IDHandle<Entity> entity2 = boxCols.GetByIdNum(col2.colId1)->entity;
					GlobalScene->SendCollisionToCustomComponents(entity2, col2);
				}
			}
		}
	}
}

void PhysicsSystem::EndFrame() {
	prevCollisions.Swap(collisions);
}


RaycastHit RaycastBox(Vector3 origin, Vector3 direction, BoxCollider* boxCol) {
	Entity* ent = GlobalScene->entities.GetById(boxCol->entity);
	ASSERT(ent != nullptr);
	IDHandle<Transform> transId = ent->transform;
	Transform* trans = GlobalScene->transforms.GetById(transId);
	ASSERT(trans != nullptr);

	Mat4x4 objMatrix = trans->GetGlobaltoLocalMatrix();

	Vector3 localOrigin = objMatrix.MultiplyAsPosition(origin);
	Vector3 localDirection = objMatrix.MultiplyAsDirection(direction);

	// size really shouldn't be negative.  It would just mean taking the abs value, 
	// but if it is negative something probably went wrong.
	ASSERT(boxCol->size.x > 0);
	ASSERT(boxCol->size.y > 0);
	ASSERT(boxCol->size.z > 0);

	Vector3 minPos = boxCol->position - boxCol->size;
	Vector3 maxPos = boxCol->position + boxCol->size;

	float minDepth = -FLT_MAX;
	float maxDepth = FLT_MAX;

	for (int i = 0; i < 3; i++) {
		if (localDirection[i] != 0.0f) {
			float depthRange1 = (minPos[i] - localOrigin[i]) / localDirection[i];
			float depthRange2 = (maxPos[i] - localOrigin[i]) / localDirection[i];

			float minDepthRange = BNS_MIN(depthRange1, depthRange2);
			float maxDepthRange = BNS_MAX(depthRange1, depthRange2);

			minDepth = BNS_MAX(minDepth, minDepthRange);
			maxDepth = BNS_MIN(maxDepth, maxDepthRange);
		}
		else {
			float range1 = (minPos[i] - localOrigin[i]);
			float range2 = (maxPos[i] - localOrigin[i]);

			if (range1 * range2 > 0) {
				RaycastHit hit;
				hit.wasHit = false;
				return hit;
			}
		}
	}

	if (minDepth > 0 && minDepth < maxDepth) {
		RaycastHit hit;
		hit.wasHit = true;

		Mat4x4 loc2glob = trans->GetLocalToGlobalMatrix();

		float localDepth = minDepth;
		Vector3 localHitPos = localOrigin + (localDirection * localDepth);
		Vector3 globalHitPos = loc2glob.MultiplyAsPosition(localHitPos);

		// Get boxSpaceHitLoc into [-1, 1] range
		Vector3 boxSpaceHitLoc = localHitPos - boxCol->position;
		for (int i = 0; i < 3; i++) {
			boxSpaceHitLoc[i] /= boxCol->size[i];
		}

		int maxIndicesFlags = 0;
		float maxVal = 0;
		for (int i = 0; i < 3; i++) {
			if (BNS_ABS(boxSpaceHitLoc[i]) > BNS_ABS(maxVal)) {
				maxIndicesFlags = (1 << i);
				maxVal = boxSpaceHitLoc[i];
			}
			else if (BNS_ABS(boxSpaceHitLoc[i]) == BNS_ABS(maxVal)) {
				maxIndicesFlags |= (1 << i);
			}
		}

		Vector3 localNormal;
		for (int i = 0; i < 3; i++) {
			if ((1 << i) & maxIndicesFlags) {
				localNormal[i] = boxSpaceHitLoc[i];
			}
		}

		hit.globalNormal = loc2glob.MultiplyAsDirection(localNormal).Normalized();

		hit.depth = (globalHitPos - origin).Magnitude();
		hit.globalPos = globalHitPos;
		hit.colId = boxCol->id;
		return hit;
	}
	else {
		RaycastHit hit;
		hit.wasHit = false;
		return hit;
	}
}

RaycastHit RaycastSphere(Vector3 origin, Vector3 direction, SphereCollider* boxCol){
	RaycastHit hit;
	hit.wasHit = false;
	return hit;
}
