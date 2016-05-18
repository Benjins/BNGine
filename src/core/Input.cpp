#include "Input.h"

#include "../../ext/CppUtils/assert.h"

Input::Input() {
	cursorDeltaX = 0;
	cursorDeltaY = 0;
	cursorX = 0;
	cursorX = 0;

	for (int i = 0; i < 256; i++) {
		standardKeys[i] = OFF;
	}

	for (int i = 0; i < 64;  i++) {
		mouseButtons[i] = OFF;
	}
}

void Input::SetCursorPos(float x, float y) {
	cursorDeltaX = x - cursorX;
	cursorDeltaY = y - cursorY;

	cursorX = x;
	cursorY = y;
}

void Input::EndFrame() {
	for (int i = 0; i < 256; i++) {
		if (standardKeys[i] == PRESS) {
			standardKeys[i] = HOLD;
		}
		else if (standardKeys[i] == RELEASE) {
			standardKeys[i] = OFF;
		}
	}

	for (int i = 0; i < 64; i++) {
		if (mouseButtons[i] == PRESS) {
			mouseButtons[i] = HOLD;
		}
		else if (mouseButtons[i] == RELEASE) {
			mouseButtons[i] = OFF;
		}
	}
}

void Input::MouseButtonPressed(MouseButton button) {
	ASSERT(button < 64);
	if (mouseButtons[(int)button] == OFF || mouseButtons[(int)button] == RELEASE) {
		mouseButtons[(int)button] = PRESS;
	}
}

void Input::MouseButtonReleased(MouseButton button) {
	ASSERT(button < 64);
	mouseButtons[(int)button] = RELEASE;
}

bool Input::MouseButtonIsDown(MouseButton button) {
	return mouseButtons[(int)button] == HOLD || mouseButtons[(int)button] == PRESS;
}

bool Input::MouseButtonIsPressed(MouseButton button) {
	return mouseButtons[(int)button] == PRESS;
}

bool Input::MouseButtonIsReleased(MouseButton button) {
	return mouseButtons[(int)button] == RELEASE;
}

void Input::KeyPressed(int code) {
	ASSERT(code < 256);
	if (standardKeys[code] == OFF || standardKeys[code] == RELEASE) {
		standardKeys[code] = PRESS;
	}
}

void Input::KeyReleased(int code) {
	ASSERT(code < 256);
	standardKeys[code] = RELEASE;
}

bool Input::KeyIsDown(unsigned char key) {
	return standardKeys[key] == HOLD || standardKeys[key] == PRESS;
}

bool Input::KeyIsPressed(unsigned char key) {
	return standardKeys[key] == PRESS;
}

bool Input::KeyIsReleased(unsigned char key) {
	return standardKeys[key] == RELEASE;
}