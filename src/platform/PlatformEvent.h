#ifndef PLATFORM_EVENT_H
#define PLATFORM_EVENT_H

#pragma once

#include "../../ext/CppUtils/disc_union.h"

#define BNS_BUTTON_PRESS 0
#define BNS_BUTTON_RELEASE 1

struct PreInitEvent {
	int argc;
	char** argv;
};
struct PostInitEvent { };
struct UpdateEvent {
	bool* shouldContinue;
};
struct ShutDownEvent { };
struct MousePosEvent {
	float x;
	float y;
};
struct MouseButtonEvent {
	int button;
	int pressOrRelease;
};
struct KeyButtonEvent {
	int keyCode;
	int pressOrRelease;
};
struct WindowResizeEvent {
	int width;
	int height;
};

#define DISC_LIST(mac)      \
	mac(PreInitEvent)       \
	mac(PostInitEvent)      \
	mac(UpdateEvent)        \
	mac(ShutDownEvent)      \
	mac(MousePosEvent)      \
	mac(MouseButtonEvent)   \
	mac(KeyButtonEvent)     \
	mac(WindowResizeEvent)

DEFINE_DISCRIMINATED_UNION(PlatformEvent, DISC_LIST)

#undef DISC_LIST


#endif
