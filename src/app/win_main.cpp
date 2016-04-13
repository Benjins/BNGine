
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

#include "../assets/AssetFile.h"

#include "../../ext/3dbasics/Vector4.h"

LRESULT CALLBACK MainWindowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);

int CALLBACK WinMain(HINSTANCE instance, HINSTANCE hPrev, LPSTR cmdLine, int cmdShow) {

	//MessageBox(0, "And text here", "MessageBox caption", MB_OK);

	//ASSERT(1 == 2);

	WNDCLASS windowCls = {};
	windowCls.hInstance = instance;
	windowCls.style = CS_OWNDC | CS_HREDRAW | CS_VREDRAW;
	windowCls.lpfnWndProc = MainWindowProc;
	windowCls.lpszClassName = "bngine-window";

	RegisterClass(&windowCls);

	//MessageBox(0, "Step 2", "MessageBox caption", MB_OK);

	HWND window = CreateWindow(windowCls.lpszClassName, "BNgine Runtime", WS_OVERLAPPEDWINDOW | WS_VISIBLE, 50, 50, 1280, 720, 0, 0, instance, 0);

	//MessageBox(0, "Step 3", "MessageBox caption", MB_OK);

	HDC hdc = GetDC(window);

	//MessageBox(window, "Step 4", "MessageBox caption", MB_OK);

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

	//MessageBox(0, "Step 5", "MessageBox caption", MB_OK);

	ReleaseDC(window, hdc);

	//MessageBox(0, "Step 6", "MessageBox caption", MB_OK);

	float x = 0;

	PackAssetFile("assets", "assets.bna");

	scn.gfx.LoadAssetFile("assets.bna");

	//ASSERT(1 == 2);

	Program* prog = scn.gfx.programs.CreateAndAdd();
	prog->vertShader = 0;
	prog->fragShader = 1;
	prog->CompileProgram();

	Material* mat = scn.gfx.materials.CreateAndAdd();
	mat->programId = prog->id;

	int scribbleTex = -1;
	scn.gfx.assetIdMap.LookUp("scribble.bmp", &scribbleTex);
	mat->AddTexture(scribbleTex);

	int boxMesh = -1;
	scn.gfx.assetIdMap.LookUp("test_2.obj", &boxMesh);

	int floorMesh = -1;
	scn.gfx.assetIdMap.LookUp("floor.obj", &floorMesh);

	DrawCall* dc = scn.gfx.drawCalls.CreateAndAdd();
	dc->meshId = boxMesh;
	dc->matId = mat->id;

	Transform* trans = scn.transforms.CreateAndAdd();
	trans->parent = -1;

	Entity* ent = scn.entities.CreateAndAdd();
	ent->transform = trans->id;

	Transform* camTrans = scn.transforms.CreateAndAdd();
	camTrans->position = Vector3(0, 0, -4);
	camTrans->rotation = QUAT_IDENTITY;
	camTrans->scale = Vector3(1, 1, 1);
	camTrans->parent = -1;

	GlobalScene->cam.transform = camTrans;

	dc->entId = ent->id;

	DrawCall* dc2 = scn.gfx.drawCalls.CreateAndAdd();

	Transform* trans2 = scn.transforms.CreateAndAdd();
	trans2->parent = trans->id;

	Entity* ent2 = scn.entities.CreateAndAdd();
	ent2->transform = trans2->id;

	dc2->entId = ent2->id;
	dc2->matId = mat->id;
	dc2->meshId = boxMesh;

	{
		DrawCall* floorDc = scn.gfx.drawCalls.CreateAndAdd();
		floorDc->matId = mat->id;

		floorDc->meshId = floorMesh;

		Entity* floorEnt = scn.entities.CreateAndAdd();
		Transform* floorTrans = scn.transforms.CreateAndAdd();
		floorTrans->parent = -1;
		floorTrans->position.y = -1;
		floorEnt->transform = floorTrans->id;
		floorDc->entId = floorEnt->id;

		//floorTrans->rotation = Quaternion(X_AXIS, 0.2f);
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
		
		trans->rotation = Quaternion(Y_AXIS, 20) * Quaternion(X_AXIS, x);
		trans->scale = Vector3(0.3f, .3f, 0.4f);

		scn.cam.transform->rotation = Quaternion(Y_AXIS, scn.input.cursorX / 80) * Quaternion(X_AXIS, scn.input.cursorY / 80 - 2);

		Vector3 moveVec;

		if (scn.input.KeyIsDown('W')) {
			moveVec = moveVec + scn.cam.transform->Forward() / 50.0f;
		}
		if (scn.input.KeyIsDown('S')) {
			moveVec = moveVec - scn.cam.transform->Forward() / 50.0f;
		}
		if (scn.input.KeyIsDown('A')) {
			moveVec = moveVec - scn.cam.transform->Right() / 50.0f;
		}
		if (scn.input.KeyIsDown('D')) {
			moveVec = moveVec + scn.cam.transform->Right() / 50.0f;
		}

		moveVec.y = 0;

		scn.cam.transform->position = scn.cam.transform->position + moveVec;

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

		GlobalScene->input.SetCursorPos(mouseX, mouseY);
	}break;

	case WM_LBUTTONDOWN:
	{
		GlobalScene->input.MouseButtonPressed(MouseButton::PRIMARY);
	}break;

	case WM_LBUTTONUP:
	{
		GlobalScene->input.MouseButtonReleased(MouseButton::PRIMARY);
	}break;

	case WM_SYSKEYDOWN:
	case WM_SYSKEYUP:
	case WM_KEYDOWN:
	case WM_KEYUP: {
		int code = wParam;
		bool wasDown = (lParam & (1 << 30)) != 0;
		bool  isDown = (lParam & (1 << 31)) == 0;

		if (code < 256) {
			if (wasDown && !isDown) {
				GlobalScene->input.KeyReleased(code);
			}
			else if (isDown && !wasDown) {
				GlobalScene->input.KeyPressed(code);
			}
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