#include "Debug3DGUI.h"

#include "../core/Scene.h"
#include "../core/Transform.h"

#include "../../ext/3dbasics/Vector4.h"
#include "../../ext/3dbasics/Mat4.h"

#include "../gfx/GLWrap.h"
#include "../gfx/GLExtInit.h"
#include "../gfx/Material.h"

Material* GetDebugColMat() {
	int colMatId = -1;
	GlobalScene->res.assetIdMap.LookUp("debugCol.mat", &colMatId);
	ASSERT(colMatId != -1);
	Material* colMat = GlobalScene->res.materials.GetByIdNum(colMatId);
	ASSERT(colMat != nullptr);
	return colMat;
}

void DebugDrawWireCube(Vector3 pos, Vector3 size, const Transform* transform /*= nullptr*/) {
	Vector3 cornerMasks[16] = {
		Vector3( 1,  1,  1),
		Vector3( 1,  1, -1),
		Vector3( 1, -1, -1),
		Vector3( 1, -1,  1),
		Vector3( 1,  1,  1),
		Vector3(-1,  1,  1),
		Vector3(-1,  1, -1),
		Vector3(-1, -1, -1),
		Vector3(-1, -1,  1),
		Vector3(-1,  1,  1),
		Vector3(-1,  1, -1),
		Vector3( 1,  1, -1),
		Vector3( 1, -1, -1),
		Vector3(-1, -1, -1),
		Vector3(-1, -1,  1),
		Vector3( 1, -1,  1)
	};

	Vector3 corners[16];
	for (int i = 0; i < BNS_ARRAY_COUNT(corners); i++) {
		corners[i] = pos + size.Scaled(cornerMasks[i]);
	}

	Material* colMat = GetDebugColMat();

	if (colMat != nullptr) {
		Mat4x4 matrix;
		if (transform != nullptr) {
			matrix = transform->GetLocalToGlobalMatrix();
		}
		colMat->SetMatrix4Uniform("_objMatrix", matrix);

		Program* prog = GlobalScene->res.programs.GetById(colMat->programId);
		glUseProgram(prog->programObj);
		colMat->SetVector4Uniform("_col", Vector4(1, 1, 1, 1));
		colMat->UpdateUniforms();

		glBegin(GL_LINE_STRIP);
		for (int i = 0; i < BNS_ARRAY_COUNT(corners); i++) {
			glVertex3f(corners[i].x, corners[i].y, corners[i].z);
		}
		glEnd();
	}
}

void DebugDrawLine(Vector3 from, Vector3 to, Vector4 col) {
	Material* colMat = GetDebugColMat();
	if (colMat != nullptr) {
		Program* prog = GlobalScene->res.programs.GetById(colMat->programId);
		glUseProgram(prog->programObj);
		colMat->SetVector4Uniform("_col", col);
		colMat->SetMatrix4Uniform("_objMatrix", Mat4x4());
		colMat->UpdateUniforms();

		float prevWidth = 1;
		glGetFloatv(GL_LINE_WIDTH, &prevWidth);
		glLineWidth(5.0f);

		glBegin(GL_LINES);
			glVertex3f(from.x, from.y, from.z);
			glVertex3f(to.x, to.y, to.z);
		glEnd();

		glLineWidth(prevWidth);
	}
}

void DebugDrawTriangle(Vector3 v0, Vector3 v1, Vector3 v2, Vector4 col) {
	Material* colMat = GetDebugColMat();
	if (colMat != nullptr) {
		Program* prog = GlobalScene->res.programs.GetById(colMat->programId);
		glUseProgram(prog->programObj);
		colMat->SetVector4Uniform("_col", col);
		colMat->SetMatrix4Uniform("_objMatrix", Mat4x4());
		colMat->UpdateUniforms();

		glBegin(GL_TRIANGLES);
			glVertex3f(v0.x, v0.y, v0.z);
			glVertex3f(v1.x, v1.y, v1.z);
			glVertex3f(v2.x, v2.y, v2.z);
		glEnd();
	}
}

