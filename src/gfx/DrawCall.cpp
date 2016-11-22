#include "DrawCall.h"
#include "Material.h"
#include "Texture.h"
#include "Mesh.h"
#include "../core/Scene.h"
#include "../core/Camera.h"

#include "GLExtInit.h"

void ExecuteDrawCalls(DrawCall* calls, int count) {
	Mat4x4 camera = GlobalScene->cam.GetCameraMatrix();
	Mat4x4 persp = GlobalScene->cam.GetPerspectiveMatrix();

	static Vector3 lightVec = Vector3(0, 0.7f, 0.7f);
	static Quaternion lightRot = Quaternion(Y_AXIS, 0.005f);

	lightVec = Rotate(lightVec, lightRot).Normalized();

	for (int i = 0; i < count; i++) {
		Material* mat = GlobalScene->res.materials.GetById(calls[i].matId);
		Program* prog = GlobalScene->res.programs.GetById(mat->programId);
		glUseProgram(prog->programObj);

		Mesh* mesh = GlobalScene->res.meshes.GetById(calls[i].meshId);

		Entity* ent = GlobalScene->entities.GetById(calls[i].entId);
		Transform* trans = GlobalScene->transforms.GetById(ent->transform);

		mat->SetMatrix4Uniform("_objMatrix", trans->GetLocalToGlobalMatrix());
		mat->SetMatrix4Uniform("_camMatrix", camera);
		mat->SetMatrix4Uniform("_perspMatrix", persp);
		mat->SetVector3Uniform("_lightAngle", lightVec);

		for (int i = 0; i < mat->texCount; i++) {
			Texture* tex =  GlobalScene->res.textures.GetById(mat->texIds[i]);
			tex->Bind(GL_TEXTURE0 + i);
		}

		mat->UpdateUniforms();

		GLint posAttribLoc = glGetAttribLocation(prog->programObj, "pos");
		glEnableVertexAttribArray(posAttribLoc);
		glBindBuffer(GL_ARRAY_BUFFER, mesh->posVbo);
		glVertexAttribPointer(posAttribLoc, 3, GL_FLOAT, GL_FALSE, 0, 0);

		GLint uvAttribLoc = glGetAttribLocation(prog->programObj, "uv");
		if (uvAttribLoc >= 0) {
			glEnableVertexAttribArray(uvAttribLoc);
			glBindBuffer(GL_ARRAY_BUFFER, mesh->uvVbo);
			glVertexAttribPointer(uvAttribLoc, 2, GL_FLOAT, GL_FALSE, 0, 0);
		}

		GLint normAttribLoc = glGetAttribLocation(prog->programObj, "normal");
		if (normAttribLoc >= 0) {
			glEnableVertexAttribArray(normAttribLoc);
			glBindBuffer(GL_ARRAY_BUFFER, mesh->normalVbo);
			glVertexAttribPointer(normAttribLoc, 3, GL_FLOAT, GL_FALSE, 0, 0);
		}

		glDrawArrays(GL_TRIANGLES,0,mesh->faces.count*3);

		glDisableVertexAttribArray(posAttribLoc);

		if (uvAttribLoc >= 0) {
			glDisableVertexAttribArray(uvAttribLoc);
		}

		if (normAttribLoc >= 0) {
			glDisableVertexAttribArray(normAttribLoc);
		}
	}
}