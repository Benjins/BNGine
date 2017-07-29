#include "CubeMap.h"

#include "GLExtInit.h"

#include "../core/Scene.h"

GLenum cubeMapTypes[] = {   GL_TEXTURE_CUBE_MAP_NEGATIVE_X, GL_TEXTURE_CUBE_MAP_POSITIVE_X,
							GL_TEXTURE_CUBE_MAP_NEGATIVE_Y, GL_TEXTURE_CUBE_MAP_POSITIVE_Y,
							GL_TEXTURE_CUBE_MAP_NEGATIVE_Z, GL_TEXTURE_CUBE_MAP_POSITIVE_Z };

inline void SetPixel(Texture* tex, int x, int y, unsigned char r, unsigned char g, unsigned char b) {
	unsigned char* pixel = &tex->texMem[(tex->width * y + x) * 3];
	pixel[0] = r;
	pixel[1] = g;
	pixel[2] = b;
}

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

		//Edge debugging.
		/*
		for(int x = 0; x < tex->width; x++){
			SetPixel(tex, x, 0, 255, 0, 0);
			SetPixel(tex, x, tex->height - 1, 255, 0, 0);
		}

		for(int y = 0; y < tex->height; y++){
			SetPixel(tex, 0, y, 255, 0, 0);
			SetPixel(tex, tex->width-1, y, 255, 0, 0);
		}
		*/

		glTexImage2D(cubeMapTypes[i], 0, GL_RGB, tex->width, tex->height, 0, GL_BGR, GL_UNSIGNED_BYTE, tex->texMem);
	}
}

void CubeMap::Bind(GLenum textureTarget) {
	glActiveTexture(textureTarget);
	glBindTexture(GL_TEXTURE_CUBE_MAP, cubeMapObj);
}

