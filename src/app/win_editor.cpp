
#if defined(BNS_DEBUG)
#include <io.h>
#include <fcntl.h>
#endif

#include <Windows.h>
#include <Windowsx.h>
#include <gl/GL.h>

#include "../editor/Editor.h"
#define EXT_EXTERN extern
#include "../gfx/GLExtInit.h"

LRESULT CALLBACK MainWindowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);

Editor* GlobalEd = nullptr;

int CALLBACK WinMain(HINSTANCE instance, HINSTANCE hPrev, LPSTR cmdLine, int cmdShow) {

	//Set up a console window for debug.
	// TODO: Why is this not working?
#if defined(BNS_DEBUG) && 0
	AllocConsole();

	HANDLE handle_out = GetStdHandle(STD_OUTPUT_HANDLE);
	int hCrt = _open_osfhandle((long)handle_out, _O_TEXT);
	FILE* hf_out = _fdopen(hCrt, "w");
	setvbuf(hf_out, NULL, _IONBF, 1);
	*stdout = *hf_out;

	HANDLE handle_in = GetStdHandle(STD_INPUT_HANDLE);
	hCrt = _open_osfhandle((long)handle_in, _O_TEXT);
	FILE* hf_in = _fdopen(hCrt, "r");
	setvbuf(hf_in, NULL, _IONBF, 128);
	*stdin = *hf_in;

	printf("%s", "Hahahahahhah printf\n");
	fprintf(stdout, "%s", "Hahahahahhah printf\n");
#endif

	WNDCLASS windowCls = {};
	windowCls.hInstance = instance;
	windowCls.style = CS_OWNDC | CS_HREDRAW | CS_VREDRAW;
	windowCls.lpfnWndProc = MainWindowProc;
	windowCls.lpszClassName = "bngine-window";

	RegisterClass(&windowCls);

	RECT winRect = { 0 };
	winRect.right = 1280;
	winRect.bottom = 720;
	AdjustWindowRect(&winRect, WS_OVERLAPPEDWINDOW | WS_VISIBLE, false);

	float winWidth = winRect.right - winRect.left;
	float winHeight = winRect.bottom - winRect.top;

	HWND window = CreateWindow(windowCls.lpszClassName, "BNgine Editor", WS_OVERLAPPEDWINDOW | WS_VISIBLE, 50, 50, winWidth, winHeight, 0, 0, instance, 0);

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

	Editor ed;
	GlobalEd = &ed;

	glClearColor(0.1f, 0.1f, 0.1f, 0.0f);
	glEnable(GL_DEPTH_TEST);

	ReleaseDC(window, hdc);

	ed.StartUp();

	bool isRunning = true;
	while (isRunning) {
		tagMSG msg;

		while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE | PM_QS_INPUT)) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);

			//TODO: Why the f--k is this 161, not WM_QUIT?
			if (msg.message == 161 && msg.wParam == 20) {
				isRunning = false;
			}
		}

		HDC hdc = GetDC(window);

		ed.Update();

		ed.Render();

		SwapBuffers(hdc);
		ReleaseDC(window, hdc);

		ed.gui.EndFrame();
		ed.scene.input.EndFrame();

		Sleep(16);
	}

	return 0;
}

KeyStrokeCode SystemKeyToKeyStrokeCode(int key);

LRESULT CALLBACK MainWindowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam) {
	LRESULT result = 0;

	switch (message) {

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

	case WM_RBUTTONDOWN:
	{
		GlobalScene->input.MouseButtonPressed(MouseButton::SECONDARY);
	}break;

	case WM_RBUTTONUP:
	{
		GlobalScene->input.MouseButtonReleased(MouseButton::SECONDARY);
	}break;

	case WM_SYSKEYDOWN:
	case WM_SYSKEYUP:
	case WM_KEYDOWN:
	case WM_KEYUP: {
		int code = wParam;
		bool wasDown = (lParam & (1 << 30)) != 0;
		bool  isDown = (lParam & (1 << 31)) == 0;

		KeyStrokeCode keyCode = SystemKeyToKeyStrokeCode(code);

		if (keyCode >= 0 && keyCode < 256) {
			if (wasDown && !isDown) {
				GlobalScene->input.KeyReleased(keyCode);
			}
			else if (isDown && !wasDown) {
				GlobalScene->input.KeyPressed(keyCode);
			}
		}
	}break;

	case WM_SIZE: {
		int width = LOWORD(lParam);
		int height = HIWORD(lParam);

		//When we first create the window, we don't actually have a scene pointer
		//because we have no GL context
		if (GlobalEd) {
			GlobalEd->cam.widthPixels = width;
			GlobalEd->cam.heightPixels = height;
		}
	} break;

	case WM_MOUSEWHEEL: {
		// Todo: yes.
		float zDelta = GET_WHEEL_DELTA_WPARAM(wParam);
	}

	default: {
		result = DefWindowProc(hwnd, message, wParam, lParam);
	} break;

	}

	return result;
}

struct VKDef {
	KeyStrokeCode keyCode;
	int virtualCode;
};

KeyStrokeCode SystemKeyToKeyStrokeCode(int key) {
	if (key >= 'A' && key <= 'Z') {
		return (KeyStrokeCode)(key);
	}
	else if (key >= '0' && key <= '9') {
		return (KeyStrokeCode)(key);
	}
	else {
		static VKDef vkDefs[] = {
			{ KC_Shift,		   VK_SHIFT },
			{ KC_Control,	   VK_CONTROL },
			{ KC_Alt,		   VK_MENU },
			{ KC_Dash,		   -1 },
			{ KC_Equals,		   -1 },
			{ KC_SemiColon,	   -1 },
			{ KC_SingleQuote,   -1 },
			{ KC_Comma,		   VK_OEM_COMMA },
			{ KC_Period,		   VK_OEM_PERIOD },
			{ KC_ForwardSlash,  -1 },
			{ KC_BackSlash,	   -1 },
			{ KC_BackSpace,	   VK_BACK },
			{ KC_Enter,		   VK_RETURN },
			{ KC_Tab,		   VK_TAB },
			{ KC_Escape,		   VK_ESCAPE },
			{ KC_Delete,		   VK_DELETE },
			{ KC_UpArrow,	   VK_UP },
			{ KC_DownArrow,	   VK_DOWN },
			{ KC_LeftArrow,	   VK_LEFT },
			{ KC_RightArrow,   VK_RIGHT },
			{ KC_Space,        VK_SPACE },
			{ KC_Minus,        VK_OEM_MINUS }
			// TODO: Minus/underscore
		};

		for (int i = 0; i < BNS_ARRAY_COUNT(vkDefs); i++) {
			if (vkDefs[i].virtualCode == key) {
				return vkDefs[i].keyCode;
			}
		}
	}

	return KC_Invalid;
}
