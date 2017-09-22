#include "Collision.h"
#include "BoxCollider.h"
#include "../core/Scene.h"

#include <float.h>

Collision SeparateAxisTheorem(Vector3 axis, Vector3* points1, Vector3* points2);

Collision BoxBoxCollision(const BoxCollider& box1, const BoxCollider& box2) {
	Collision col;
	col.isColliding = false;

	Entity* ent1 = GlobalScene->entities.GetById(box1.entity);
	Entity* ent2 = GlobalScene->entities.GetById(box2.entity);

	Transform* trans1 = GlobalScene->transforms.GetById(ent1->transform);
	Transform* trans2 = GlobalScene->transforms.GetById(ent2->transform);

	Mat4x4 col2Loc2Glob = trans2->GetLocalToGlobalMatrix();
	Mat4x4 col1Glob2Loc = trans1->GetGlobaltoLocalMatrix();
	Mat4x4 col2ToCol1Matrix = col1Glob2Loc * col2Loc2Glob;

	Vector3 transformedCol2Axes[3] = { 
		col2ToCol1Matrix.MultiplyAsDirection(X_AXIS),
		col2ToCol1Matrix.MultiplyAsDirection(Y_AXIS),
		col2ToCol1Matrix.MultiplyAsDirection(Z_AXIS) 
	};

	Vector3 testAxes[15] = {};
	//0-8
	// TODO: There are NaN's being made, what gives?
	for (int i = 0; i < 3; i++) {
		Vector3 transformedAxis = transformedCol2Axes[i];
		//Crossed with X, Y, and Z axes
		if (transformedAxis.x == 0.0f && transformedAxis.y == 0.0f) {
			testAxes[3 * i] = transformedAxis;
		}
		else {
			testAxes[3 * i] = Vector3(0, transformedAxis.y, -transformedAxis.x).Normalized();
		}

		if (transformedAxis.x == 0.0f && transformedAxis.z == 0.0f) {
			testAxes[3 * i + 1] = transformedAxis;
		}
		else {
			testAxes[3 * i + 1] = Vector3(transformedAxis.x, -transformedAxis.z, 0).Normalized();
		}

		if (transformedAxis.y == 0.0f && transformedAxis.z == 0.0f) {
			testAxes[3 * i + 2] = transformedAxis;
		}
		else {
			testAxes[3 * i + 2] = Vector3(-transformedAxis.y, 0, transformedAxis.z).Normalized();
		}
	}

	//9-11
	for (int i = 0; i < 3; i++) {
		testAxes[9 + i] = transformedCol2Axes[i].Normalized();
	}

	//12-14
	testAxes[12] = X_AXIS;
	testAxes[13] = Y_AXIS;
	testAxes[14] = Z_AXIS;

	Vector3 col1Min = box1.position - box1.size;
	Vector3 col1Max = box1.position + box1.size;
	Vector3 col2Min = box2.position - box2.size;
	Vector3 col2Max = box2.position + box2.size;

	Vector3 col1Corners[8] = { 
		Vector3(col1Min.x, col1Min.y, col1Min.z),
		Vector3(col1Min.x, col1Min.y, col1Max.z),
		Vector3(col1Min.x, col1Max.y, col1Min.z),
		Vector3(col1Min.x, col1Max.y, col1Max.z),
		Vector3(col1Max.x, col1Min.y, col1Min.z),
		Vector3(col1Max.x, col1Min.y, col1Max.z),
		Vector3(col1Max.x, col1Max.y, col1Min.z),
		Vector3(col1Max.x, col1Max.y, col1Max.z)
	};

	Vector3 col2Corners[8] = { 
		col2ToCol1Matrix.MultiplyAsPosition(Vector3(col2Min.x, col2Min.y, col2Min.z)),
		col2ToCol1Matrix.MultiplyAsPosition(Vector3(col2Min.x, col2Min.y, col2Max.z)),
		col2ToCol1Matrix.MultiplyAsPosition(Vector3(col2Min.x, col2Max.y, col2Min.z)),
		col2ToCol1Matrix.MultiplyAsPosition(Vector3(col2Min.x, col2Max.y, col2Max.z)),
		col2ToCol1Matrix.MultiplyAsPosition(Vector3(col2Max.x, col2Min.y, col2Min.z)),
		col2ToCol1Matrix.MultiplyAsPosition(Vector3(col2Max.x, col2Min.y, col2Max.z)),
		col2ToCol1Matrix.MultiplyAsPosition(Vector3(col2Max.x, col2Max.y, col2Min.z)),
		col2ToCol1Matrix.MultiplyAsPosition(Vector3(col2Max.x, col2Max.y, col2Max.z))
	};

	Collision checkDepthCollision = {};
	checkDepthCollision.colType1 = CT_BOX;
	checkDepthCollision.colType2 = CT_BOX;
	checkDepthCollision.colId1 = box1.id;
	checkDepthCollision.colId2 = box2.id;

	checkDepthCollision.isColliding = true;
	checkDepthCollision.depth = FLT_MAX;

	for (int i = 0; i < 15; i++) {
		Vector3 testAxis = testAxes[i];
		Collision potentialCollision = SeparateAxisTheorem(testAxis, col1Corners, col2Corners);
		if (!potentialCollision.isColliding) {
			Collision x;
			x.isColliding = false;
			return x;
		}
		else {
			//Find the MVT by finding the axis with the smallest intersection
			if (potentialCollision.depth < checkDepthCollision.depth) {
				checkDepthCollision.depth = potentialCollision.depth;
				Mat4x4 col1ToGlobal = trans1->GetLocalToGlobalMatrix();
				checkDepthCollision.normal = col1ToGlobal.MultiplyAsDirection(potentialCollision.normal);
				checkDepthCollision.depth *= checkDepthCollision.normal.Magnitude();
				checkDepthCollision.normal.Normalize();
			}
		}
	}

	return checkDepthCollision;
}

Collision SeparateAxisTheorem(Vector3 axis, Vector3* points1, Vector3* points2) {
	float point1Min = FLT_MAX;
	float point1Max = -FLT_MAX;

	float point2Min = FLT_MAX;
	float point2Max = -FLT_MAX;

	for (int i = 0; i < 8; i++) {
		float projection1 = DotProduct(axis, points1[i]);
		point1Max = BNS_MAX(point1Max, projection1);
		point1Min = BNS_MIN(point1Min, projection1);

		float projection2 = DotProduct(axis, points2[i]);
		point2Max = BNS_MAX(point2Max, projection2);
		point2Min = BNS_MIN(point2Min, projection2);
	}

	if (point2Min < point1Max && point2Max > point1Min) {
		float maxMin = BNS_MAX(point1Min, point2Min);
		float minMax = BNS_MIN(point1Max, point2Max);

		Collision x;
		x.isColliding = true;
		x.normal = axis; // In local coords, converted to global in collision
		x.normal = x.normal * (point2Min > point1Min ? -1 : 1);
		x.depth = minMax - maxMin;
		return x;
	}

	Collision x;
	x.isColliding = false;
	return x;
}
