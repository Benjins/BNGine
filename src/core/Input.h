#ifndef INPUT_H
#define INPUT_H

#pragma once

enum KeyState {
	OFF = 0,  
	PRESS,	  
	RELEASE,	
	HOLD	   
};

enum MouseButton {
	PRIMARY = 0,
	SECONDARY,
	TERTIARY,
	CUSTOM
};

inline KeyState KeyStateFromBools(bool isDown, bool wasDown) {
	return (KeyState)((wasDown ? 1 : 0) | (isDown ? 2 : 0));
}

#if 0
static_assert(KeyStateFromBools(false, false) == OFF, "OFF key state");
static_assert(KeyStateFromBools(false, true) == PRESS, "PRESS key state");
static_assert(KeyStateFromBools(true, true) == HOLD, "HOLD key state");
static_assert(KeyStateFromBools(true, false) == RELEASE, "RELEASE key state");
#endif

enum KeyStrokeCode {
	KC_Invalid = -1,
	KC_Shift,
	KC_Control,
	KC_Alt,
	KC_Dash,
	KC_Equals,
	KC_SemiColon,
	KC_SingleQuote,
	KC_Comma,
	KC_Period,
	KC_ForwardSlash,
	KC_BackSlash,
	KC_BackSpace,
	KC_Enter,
	KC_Tab,
	KC_Escape,
	KC_Delete,
	KC_LeftArrow,
	KC_RightArrow,
	KC_UpArrow,
	KC_DownArrow,
	KC_Space,
	KC_LastNonAlphaNum,
	KC_A = 'A',
	KC_Z = 'Z',
	KC_Zero = '0',
	KC_Nine = '9'
};

static_assert(KC_LastNonAlphaNum < KC_Zero, "Need to rearrange virtual keys.");

struct Input {
	KeyState standardKeys[256];
	KeyState mouseButtons[64];

	float cursorX;
	float cursorY;

	float cursorDeltaX;
	float cursorDeltaY;

	Input();

	void SetCursorPos(float x, float y);

	void EndFrame();

	void MouseButtonPressed(MouseButton button);
	void MouseButtonReleased(MouseButton button);

	bool MouseButtonIsDown(MouseButton button);
	bool MouseButtonIsPressed(MouseButton button);
	bool MouseButtonIsReleased(MouseButton button);

	void KeyPressed(int code);
	void KeyReleased(int code);

	bool KeyIsDown(unsigned char key);
	bool KeyIsPressed(unsigned char key);
	bool KeyIsReleased(unsigned char key);
};


#endif