#include "Texture.h"

#include "GLExtInit.h"

void Texture::UploadToGraphicsDevice() {
	glGenTextures(1, &textureObj);
	
	glBindTexture(textureType, textureObj);
	glTexImage2D(textureType, 0, GL_RGB, width, height, 0, GL_BGR, GL_UNSIGNED_BYTE, texMem);
	glTexParameterf(textureType, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameterf(textureType, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
}

void Texture::Bind(GLenum textureTarget) {
	glActiveTexture(textureTarget);
	glBindTexture(textureType, textureObj);
}