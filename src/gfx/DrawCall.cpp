#include "DrawCall.h"
#include "Material.h"
#include "Mesh.h"
#include "../core/Scene.h"
#include "../core/Camera.h"

#include "GLExtInit.h"

void ExecuteDrawCalls(DrawCall* calls, int count) {
	Mat4x4 camera = GlobalScene->cam.GetCameraMatrix();
	Mat4x4 persp = GlobalScene->cam.GetPerspectiveMatrix();

	for (int i = 0; i < count; i++) {
		Material* mat = GlobalScene->gfx.materials.GetById(calls[i].matId);
		Program* prog = GlobalScene->gfx.programs.GetById(mat->programId);
		glUseProgram(prog->programObj);

		Mesh* mesh = GlobalScene->gfx.meshes.GetById(calls[i].meshId);

		Entity* ent = GlobalScene->entities.GetById(calls[i].entId);
		Transform* trans = GlobalScene->transforms.GetById(ent->transform);

		mat->SeMatrix4Uniform("_objMatrix", trans->GetLocalToGlobalMatrix());
		mat->SeMatrix4Uniform("_camMatrix", camera);
		mat->SeMatrix4Uniform("_perspMatrix", persp);

		GLint posAttribLoc = glGetAttribLocation(prog->programObj, "pos");
		glEnableVertexAttribArray(posAttribLoc);
		glBindBuffer(GL_ARRAY_BUFFER, mesh->vbo);
		glVertexAttribPointer(posAttribLoc, 3, GL_FLOAT, GL_FALSE, 0, 0);

		glDrawArrays(GL_TRIANGLES,0,mesh->faces.count*3);

		glDisableVertexAttribArray(posAttribLoc);
	}
}