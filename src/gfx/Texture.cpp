#include "Texture.h"

#include "GLExtInit.h"

Texture::Texture(){
	externalColourFormat = GL_RGB;
	internalColourFormat = GL_BGR;
}

void Texture::UploadToGraphicsDevice() {
	glGenTextures(1, &textureObj);
	
	glBindTexture(textureType, textureObj);
	glTexImage2D(textureType, 0, externalColourFormat, width, height, 0, internalColourFormat, GL_UNSIGNED_BYTE, texMem);
	glTexParameterf(textureType, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameterf(textureType, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
}

void Texture::Bind(GLenum textureTarget) {
	glActiveTexture(textureTarget);
	glBindTexture(textureType, textureObj);
}