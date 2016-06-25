#ifndef TEXTURE_H
#define TEXTURE_H

#pragma once

#include "GLWrap.h"

#include "../../ext/CppUtils/idbase.h"


struct Texture : IDBase {
	GLenum textureType;
	GLuint textureObj;
	
	GLenum internalColourFormat;
	GLenum externalColourFormat;
	
	Texture();

	int width;
	int height;
	unsigned char* texMem;

	void UploadToGraphicsDevice();
	void Bind(GLenum textureTarget);

	void Destroy();

	~Texture();
};


#endif