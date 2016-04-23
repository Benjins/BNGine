#ifndef EDITOR_H
#define EDITOR_H

#pragma once

#include "../core/Scene.h"

struct Editor {
	Scene scene;

	Camera cam;
	Transform editorCamTrans;

	void Update();
	void Render();
	void StartUp();
};

#endif
