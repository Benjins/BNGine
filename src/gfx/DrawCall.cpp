#include "DrawCall.h"
#include "Material.h"
#include "Mesh.h"
#include "../core/Scene.h"

#include "GLExtInit.h"

void ExecuteDrawCalls(DrawCall* calls, int count) {
	for (int i = 0; i < count; i++) {
		Material* mat = GlobalScene->gfx.materials.GetById(calls[i].matId);
		Program* prog = GlobalScene->gfx.programs.GetById(mat->programId);
		glUseProgram(prog->programObj);

		Mesh* mesh = GlobalScene->gfx.meshes.GetById(calls[i].matId);

		GLint posAttribLoc = glGetAttribLocation(prog->programObj, "pos");
		glEnableVertexAttribArray(posAttribLoc);
		glBindBuffer(GL_ARRAY_BUFFER, mesh->vbo);
		glVertexAttribPointer(posAttribLoc, 3, GL_FLOAT, GL_FALSE, 0, 0);

		glDrawArrays(GL_TRIANGLES,0,mesh->faces.count*3);

		glDisableVertexAttribArray(posAttribLoc);
	}
}