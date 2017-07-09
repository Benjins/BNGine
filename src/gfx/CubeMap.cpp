#include "CubeMap.h"

#include "GLExtInit.h"

#include "../core/Scene.h"

GLenum cubeMapTypes[] = {   GL_TEXTURE_CUBE_MAP_NEGATIVE_X, GL_TEXTURE_CUBE_MAP_POSITIVE_X,
							GL_TEXTURE_CUBE_MAP_NEGATIVE_Y, GL_TEXTURE_CUBE_MAP_POSITIVE_Y,
							GL_TEXTURE_CUBE_MAP_NEGATIVE_Z, GL_TEXTURE_CUBE_MAP_POSITIVE_Z };

void CubeMap::UploadToGraphicsDevice() {
	// TODO: ???
	glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);

	glGenTextures(1, &cubeMapObj);
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_CUBE_MAP, cubeMapObj);

	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

	for (int i = 0; i < 6; i++) {
		Texture* tex = GlobalScene->res.textures.GetById(textures[i]);
		// TODO: Seems like a lot of copying?  Way to flag texture type?
		// But then couldn't be re-used....
		glTexImage2D(cubeMapTypes[i], 0, GL_RGB, tex->width, tex->height, 0, GL_BGR, GL_UNSIGNED_BYTE, tex->texMem);
	}


}

