#include "PhysicsSystem.h"

#include "../core/Scene.h"

#include "../../ext/CppUtils/macros.h"

#include <cfloat>

const float PhysicsSystem::fixedTimestep = 0.02f;

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
	for (int i = 0; i < meshCols.currentCount; i++) {
		if (!meshCols.vals[i].isTrigger) {
			RaycastHit meshHit = RaycastMesh(origin, direction, &meshCols.vals[i]);

			if (meshHit.wasHit && meshHit.depth < finalHit.depth) {
				finalHit = meshHit;
			}
		}
	}

	return finalHit;
}

void PhysicsSystem::AdvanceTime(float time) {
	timeOffset += time;
	while (timeOffset >= fixedTimestep) {
		StepFrame();
		timeOffset -= fixedTimestep;
	}

	EndFrame();
}

void PhysicsSystem::StepFrame(float dt /*= fixedTimestep*/) {
	collisions.Clear();

	Vector3 gravity = Y_AXIS * -9.81f * dt * 0.05f;

	BNS_FOR_I(rigidBodies.currentCount) {
		int flagsNeeded = RBF_HasGravity | RBF_IsEnabled;
		if ((rigidBodies.vals[i].rbFlags & flagsNeeded) == flagsNeeded) {
			Entity* ent = GlobalScene->entities.GetById(rigidBodies.vals[i].entity);
			ASSERT(ent != nullptr);
			Transform* trans = GlobalScene->transforms.GetById(ent->transform);
			ASSERT(trans != nullptr);
			trans->position = trans->position + gravity;
		}
	}

	for (int i = 0; i < boxCols.currentCount; i++) {
		for (int j = i + 1; j < boxCols.currentCount; j++) {
			Collision col = BoxBoxCollision(boxCols.vals[i], boxCols.vals[j]);
			if (col.isColliding) {
				collisions.PushBack(col);
				IDHandle<Entity> entity1 = boxCols.GetByIdNum(col.colId1)->entity;
				GlobalScene->SendCollisionToCustomComponents(entity1, col);

				if (RigidBody* rb1 = FIND_COMPONENT_BY_ENTITY(RigidBody, entity1)) {
					if ((rb1->rbFlags & RBF_IsEnabled) != 0) {
						Entity* ent = GlobalScene->entities.GetById(rb1->entity);
						Transform* trans = GlobalScene->transforms.GetById(ent->transform);
						trans->position = trans->position + col.normal * col.depth;
					}
				}

				Collision col2 = col;
				col2.colId2 = col.colId1;
				col2.colId1 = col.colId2;
				col2.colType1 = col.colType2;
				col2.colType2 = col.colType1;

				IDHandle<Entity> entity2 = boxCols.GetByIdNum(col2.colId1)->entity;
				GlobalScene->SendCollisionToCustomComponents(entity2, col2);

				if (RigidBody* rb2 = FIND_COMPONENT_BY_ENTITY(RigidBody, entity2)) {
					if ((rb2->rbFlags & RBF_IsEnabled) != 0) {
						Entity* ent = GlobalScene->entities.GetById(rb2->entity);
						Transform* trans = GlobalScene->transforms.GetById(ent->transform);
						trans->position = trans->position + col.normal * col.depth;
					}
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
		hit.type = CT_BOX;

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

RaycastHit RaycastSphere(Vector3 origin, Vector3 direction, SphereCollider* sphereCol){
	RaycastHit hit;
	hit.wasHit = false;
	return hit;
}


RaycastHit RaycastMesh(Vector3 origin, Vector3 direction, MeshCollider* meshCol) {
	Entity* ent = GlobalScene->entities.GetById(meshCol->entity);
	ASSERT(ent != nullptr);
	IDHandle<Transform> transId = ent->transform;
	Transform* trans = GlobalScene->transforms.GetById(transId);
	ASSERT(trans != nullptr);

	Mat4x4 objMatrix = trans->GetGlobaltoLocalMatrix();

	Vector3 localOrigin = objMatrix.MultiplyAsPosition(origin);
	Vector3 localDirection = objMatrix.MultiplyAsDirection(direction);

	RaycastHit hit;
	hit.wasHit = false;

	Mesh* mesh = GlobalScene->res.meshes.GetById(meshCol->mesh);
	ASSERT(mesh != nullptr);
	BNS_VEC_FOREACH(mesh->faces) {
		Vector3 v0 = mesh->positions.data[ptr->posIndices[0]];
		Vector3 v1 = mesh->positions.data[ptr->posIndices[1]];
		Vector3 v2 = mesh->positions.data[ptr->posIndices[2]];

		Vector3 normal = CrossProduct(v1 - v0, v2 - v0).Normalized();
		Vector3 v0Toorigin = localOrigin - v0;
		Vector3 v1Toorigin = localOrigin - v1;
		Vector3 v2Toorigin = localOrigin - v2;

		float planeDistance = DotProduct(v0Toorigin, normal);
		// negate it so we go toward plane
		float rayDist = -DotProduct(localDirection, normal);

		// You could argue there are cases where this
		// holds, and we still get a result,
		// but it seems like an edge case we can just avoid for now.
		if (rayDist == 0) {
			continue;
		}

		float rayDistanceToPlane = planeDistance / rayDist;
		//OutputDebugStringA(StringStackBuffer<256>("rayDistanceToPlane: %f\n", rayDistanceToPlane).buffer);

		if (rayDistanceToPlane < 0) {
			continue;
		}

		Vector3 rayHitsPlane = localOrigin + localDirection * rayDistanceToPlane;

		Vector3 v01Check = CrossProduct(v0 - rayHitsPlane, v1 - v0);
		Vector3 v12Check = CrossProduct(v1 - rayHitsPlane, v2 - v1);
		Vector3 v20Check = CrossProduct(v2 - rayHitsPlane, v0 - v2);

		Vector3 v01Expect = CrossProduct(v0 - v2, v1 - v0);
		Vector3 v12Expect = CrossProduct(v1 - v0, v2 - v1);
		Vector3 v20Expect = CrossProduct(v2 - v1, v0 - v2);

		bool isInsideTriangle = DotProduct(v01Check, v01Expect) >= 0;
		isInsideTriangle &= DotProduct(v12Check, v12Expect) >= 0;
		isInsideTriangle &= DotProduct(v20Check, v20Expect) >= 0;

		if (isInsideTriangle) {
			Mat4x4 loc2glob = trans->GetLocalToGlobalMatrix();
			Vector3 globalHitPos = loc2glob.MultiplyAsPosition(rayHitsPlane);

			float depth = (origin - globalHitPos).Magnitude();

			if (!hit.wasHit || depth < hit.depth) {
				hit.wasHit = true;
				hit.type = CT_MESH;
				hit.depth = depth;
				hit.globalPos = globalHitPos;
				hit.colId = meshCol->id;
				hit.globalNormal = loc2glob.MultiplyAsDirection(normal).Normalized();
			}
		}
	}

	return hit;
}

