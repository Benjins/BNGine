#ifndef CUBE_MAP_H
#define CUBE_MAP_H

#pragma once

#include "Texture.h"

struct CubeMap : IDBase {
	union {
		struct {
			IDHandle<Texture> rightTex;
			IDHandle<Texture> leftTex;
			IDHandle<Texture> bottomTex;
			IDHandle<Texture> topTex;
			IDHandle<Texture> backTex;
			IDHandle<Texture> frontTex;
		};

		IDHandle<Texture> textures[6];
	};

	CubeMap() : IDBase() {

	}

	void Bind(GLenum textureTarget);

	CubeMap(const CubeMap& orig) {
		id = orig.id;
		BNS_FOR_I(6) {
			textures[i] = orig.textures[i];
		}
	}

	GLuint cubeMapObj;

	void UploadToGraphicsDevice();
};

#endif
