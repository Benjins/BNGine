
#include <Windows.h>
#include <Windowsx.h>
#include <gl/GL.h>

#include "../core/Scene.h"
#include "../core/Camera.h"
#include "../gfx/Shader.h"
#include "../gfx/Material.h"
#include "../gfx/Mesh.h"
#include "../gfx/DrawCall.h"
#include "../gfx/GLExtInit.h"

#include "../../ext/3dbasics/Vector4.h"

LRESULT CALLBACK MainWindowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);

int CALLBACK WinMain(HINSTANCE instance, HINSTANCE hPrev, LPSTR cmdLine, int cmdShow) {

	WNDCLASS windowCls = {};
	windowCls.hInstance = instance;
	windowCls.style = CS_OWNDC | CS_HREDRAW | CS_VREDRAW;
	windowCls.lpfnWndProc = MainWindowProc;
	windowCls.lpszClassName = "bngine-window";

	RegisterClass(&windowCls);

	HWND window = CreateWindow(windowCls.lpszClassName, "BNgine Runtime", WS_OVERLAPPEDWINDOW | WS_VISIBLE, 50, 50, 1280, 720, 0, 0, instance, 0);

	HDC hdc = GetDC(window);

	PIXELFORMATDESCRIPTOR desiredPixelFormat = {};
	desiredPixelFormat.nSize = sizeof(desiredPixelFormat);
	desiredPixelFormat.nVersion = 1;
	desiredPixelFormat.dwFlags = PFD_SUPPORT_OPENGL | PFD_DRAW_TO_WINDOW | PFD_DOUBLEBUFFER;
	desiredPixelFormat.cColorBits = 32;
	desiredPixelFormat.cAlphaBits = 8;
	desiredPixelFormat.cDepthBits = 32;
	desiredPixelFormat.iLayerType = PFD_MAIN_PLANE;

	int actualFormatIndex = ChoosePixelFormat(hdc, &desiredPixelFormat);
	PIXELFORMATDESCRIPTOR actualPixelFormat;
	DescribePixelFormat(hdc, actualFormatIndex, sizeof(actualPixelFormat), &actualPixelFormat);
	SetPixelFormat(hdc, actualFormatIndex, &actualPixelFormat);

	HGLRC glContext = wglCreateContext(hdc);
	if (!wglMakeCurrent(hdc, glContext)) {
		OutputDebugStringA("OIh god, no OpenGL cont3ext.\n  :O");
	}

	InitGlExts();

	Scene scn;

	glClearColor(1.0f, 0.0f, 1.0f, 1.0f);
	glEnable(GL_DEPTH_TEST);
	glViewport(0, 0, 1280, 720);
	glLoadIdentity();

	ReleaseDC(window, hdc);

	float x = 0;

	Shader* vs = scn.gfx.shaders.CreateAndAdd();
	vs->CompileShader("#version 130\n"
		"attribute vec3 pos;\n"
		"uniform float _x;\n"
		"uniform mat4 _objMatrix;\n"
		"uniform mat4 _camMatrix;\n"
		"uniform mat4 _perspMatrix;\n"
		"out vec4 _outPos;"
		"void main(){vec4 outPos = vec4(pos, 1); _outPos = outPos; gl_Position = _perspMatrix * _camMatrix * _objMatrix * outPos;}", GL_VERTEX_SHADER);

	Shader* fs = scn.gfx.shaders.CreateAndAdd();
	fs->CompileShader("out vec4 FragColor; in vec4 _outPos; void main(){FragColor = (_outPos + vec4(1,1,1,1))/2;}", GL_FRAGMENT_SHADER);

	Program* prog = scn.gfx.programs.CreateAndAdd();
	prog->vertShader = vs->id;
	prog->fragShader = fs->id;
	prog->CompileProgram();

	Material* mat = scn.gfx.materials.CreateAndAdd();
	mat->programId = prog->id;

	Mesh* mesh = scn.gfx.meshes.CreateAndAdd();
	mesh->vertices.PushBack(Vector3(1, -1, -1));
	mesh->vertices.PushBack(Vector3(1, -1, 1));
	mesh->vertices.PushBack(Vector3(-1, -1, 1));
	mesh->vertices.PushBack(Vector3(-1, -1, -1));
	mesh->vertices.PushBack(Vector3(1, 1, -1));
	mesh->vertices.PushBack(Vector3(1, 1, 1));
	mesh->vertices.PushBack(Vector3(-1, 1, 1));
	mesh->vertices.PushBack(Vector3(-1, 1, -1));

	int indices[] = { 1, 2, 3,
					  8, 7, 6,
					  5, 6, 2,
					  6, 7, 3,
					  3, 7, 8,
					  5, 1, 4,
					  4, 1, 3,
					  5, 8, 6,
					  1, 5, 2,
					  2, 6, 3,
					  4, 3, 8,
					  8, 5, 4 };
	for (int i = 0; i < 12; i++) {
		Face f = { indices[3*i]-1, indices[3*i+1]-1, indices[3*i+2]-1};
		mesh->faces.PushBack(f);
	}
	mesh->UploadToGfxDevice();

	DrawCall* dc = scn.gfx.drawCalls.CreateAndAdd();
	dc->meshId = mesh->id;
	dc->matId = mat->id;

	Transform* trans = scn.transforms.CreateAndAdd();
	trans->parent = -1;

	Entity* ent = scn.entities.CreateAndAdd();
	ent->transform = trans->id;

	Transform* camTrans = scn.transforms.CreateAndAdd();
	camTrans->position = Vector3(0, 0, 0);
	camTrans->rotation = QUAT_IDENTITY;
	camTrans->scale = Vector3(1, 1, 1);
	camTrans->parent = -1;

	GlobalScene->cam.transform = camTrans;

	dc->entId = ent->id;

	DrawCall* dc2 = scn.gfx.drawCalls.CreateAndAdd();

	Transform* trans2 = scn.transforms.CreateAndAdd();
	trans2->parent = -1;

	Entity* ent2 = scn.entities.CreateAndAdd();
	ent2->transform = trans2->id;

	dc2->entId = ent2->id;
	dc2->matId = mat->id;
	dc2->meshId = mesh->id;

	{
		DrawCall* floorDc = scn.gfx.drawCalls.CreateAndAdd();
		floorDc->matId = mat->id;

		Mesh* floorMesh = scn.gfx.meshes.CreateAndAdd();
		floorMesh->vertices.PushBack(Vector3(-5,  5, 0));
		floorMesh->vertices.PushBack(Vector3( 5,  5, 0));
		floorMesh->vertices.PushBack(Vector3( 5, -5, 0));
		floorMesh->vertices.PushBack(Vector3(-5, -5, 0));
		Face f1 = { 0,1,2 };
		Face f2 = { 1,2,3 };
		floorMesh->faces.PushBack(f1);
		floorMesh->faces.PushBack(f2);
		floorMesh->UploadToGfxDevice();

		floorDc->meshId = floorMesh->id;

		Entity* floorEnt = scn.entities.CreateAndAdd();
		Transform* floorTrans = scn.transforms.CreateAndAdd();
		floorTrans->parent = -1;
		floorEnt->transform = floorTrans->id;
		floorDc->entId = floorEnt->id;

		floorTrans->rotation = Quaternion(X_AXIS, 0.2f);
	}

	Camera cam;
	cam.transform = camTrans;

	bool isRunning = true;
	while (isRunning) {
		tagMSG msg;

		while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE | PM_QS_INPUT)){
			TranslateMessage(&msg);
			DispatchMessage(&msg);

			//TODO: Why the f--k is this 161, not WM_QUIT?
			if (msg.message == 161 && msg.wParam == 20) {
				isRunning = false;
			}
		}

		HDC hdc = GetDC(window);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		
		//Quaternion rotation = Quaternion(Y_AXIS, 20) * Quaternion(X_AXIS, x);

		trans->rotation = Quaternion(Y_AXIS, 20) * Quaternion(X_AXIS, x);
		trans->scale = Vector3(0.3f, .3f, 0.4f);

		trans2->position = Vector3(x / 12, -0.5f, -0.5f);
		trans2->rotation = Quaternion(X_AXIS, 0.4f) * Quaternion(Z_AXIS, 0.2f + x/15);
		trans2->scale = Vector3(0.1f, 2.1f, 0.1f);

		scn.gfx.Render();

		x = x + 0.01f;

		if (x > 6.2f) {
			x -= 6.0f;
		}

		SwapBuffers(hdc);
		ReleaseDC(window, hdc);
		
		Sleep(16);
	}

	return 0;
}

LRESULT CALLBACK MainWindowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam) {
	LRESULT result = 0;

	switch (message) {
	case WM_SIZE: {
		RECT clientRect;
		GetClientRect(hwnd, &clientRect);
		//ResizeWindow(clientRect.right - clientRect.left, clientRect.bottom - clientRect.top);
	}break;

	case WM_MOUSEMOVE:
	{
		int mouseX = GET_X_LPARAM(lParam);
		int mouseY = GET_Y_LPARAM(lParam);

		//currMouseX = mouseX;
		//currMouseY = mouseY;
		//
		//if (wParam & MK_LBUTTON) {
		//	mouseState = HOLD;
		//}
	}break;

	case WM_LBUTTONDOWN:
	{
		int mouseX = GET_X_LPARAM(lParam);
		int mouseY = GET_Y_LPARAM(lParam);
		
		//if (mouseState != HOLD) {
		//	mouseState = PRESS;
		//}
	}break;

	case WM_LBUTTONUP:
	{
		int mouseX = GET_X_LPARAM(lParam);
		int mouseY = GET_Y_LPARAM(lParam);

		//mouseState = RELEASE;
		//MouseDown(mouseX, mouseY);
	}break;

	case WM_SYSKEYDOWN:
	case WM_SYSKEYUP:
	case WM_KEYDOWN:
	case WM_KEYUP: {
		int code = wParam;
		bool wasDown = (lParam & (1 << 30)) != 0;
		bool  isDown = (lParam & (1 << 31)) == 0;

		Transform* camera = GlobalScene->cam.transform;

		if (code == 'W') {
			camera->position.z += 0.2;
		}
		else if (code == 'S') {
			camera->position.z -= 0.2;
		}
		else if (code == 'A') {
			camera->position.x -= 0.2;
		}
		else if (code == 'D') {
			camera->position.x += 0.2;
		}
		else if (code == 'Q') {
			camera->position.y += 0.2;
		}
		else if (code == 'Z') {
			camera->position.y -= 0.2;
		}

		//keyStates[code] = StateFromBools(wasDown, isDown);
	}break;


	case WM_PAINT: {
		//WindowsPaintWindow(hwnd);
	}break;


	case WM_ACTIVATEAPP: {

	}break;

	case WM_MOUSEWHEEL: {
		float zDelta = GET_WHEEL_DELTA_WPARAM(wParam);
		//printf("Whell delta: %f\n", zDelta);
	}

	default: {
		result = DefWindowProc(hwnd, message, wParam, lParam);
	} break;

	}

	return result;
}