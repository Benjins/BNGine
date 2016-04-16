#include "PhysicsSystem.h"

#include "../core/Scene.h"

#include "../../ext/CppUtils/macros.h"

#include <cfloat>

RaycastHit PhysicsSystem::Raycast(Vector3 origin, Vector3 direction) {
	RaycastHit finalHit;
	finalHit.wasHit = false;
	finalHit.depth = FLT_MAX;
	for (int i = 0; i < boxCols.currentCount; i++) {
		RaycastHit boxHit = RaycastBox(origin, direction, &boxCols.vals[i]);

		if (boxHit.wasHit && boxHit.depth < finalHit.depth) {
			finalHit = boxHit;
		}
	}

	return finalHit;
}

void PhysicsSystem::StepFrame() {

}

void PhysicsSystem::EndFrame() {

}


RaycastHit RaycastBox(Vector3 origin, Vector3 direction, BoxCollider* boxCol) {
	Entity* ent = GlobalScene->entities.GetById(boxCol->entity);
	ASSERT(ent != nullptr);
	uint32 transId = ent->transform;
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

		Vector3 boxSpaceHitLoc = localHitPos - boxCol->position;

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
