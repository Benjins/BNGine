#include<X11/X.h>
#include<X11/Xlib.h>
#include <X11/XKBlib.h>

#include "../core/Scene.h"
#include "../gfx/GLExtInit.h"

#include <stdio.h>

#include <unistd.h>

int evtPred(Display* d, XEvent* evt, XPointer xp) {
	return 1;
}

unsigned char KeyStrokeCodeToAscii(Display* display, int keycode) {
	KeySym ksym = XkbKeycodeToKeysym(display, keycode, 0, (keycode & ShiftMask) ? 1 : 0);

	//There's got to be a better way.  In fact there is.  I just haven't done it yet.
	if (ksym >= 'a' && ksym <= 'z') {
		return ksym & ~('a' ^ 'A');
	}
	else {
		return ksym;
	}
}

#include "../app/app_funcs.h"

int main(int argc, char** argv) {
	{
		PreInitEvent evt;
		evt.argc = argc;
		evt.argv = argv;
		AppEventFunction(evt);
	}

	Display* dpy = XOpenDisplay(NULL);

	if (dpy == NULL) {
		printf("\n\tcannot connect to X server\n\n");
		exit(-1);
	}

	Window root = DefaultRootWindow(dpy);

	GLint att[] = { GLX_RGBA, GLX_DEPTH_SIZE, 24, GLX_DOUBLEBUFFER, None };

	XVisualInfo* vi = glXChooseVisual(dpy, 0, att);

	if (vi == NULL) {
		printf("\n\tno appropriate visual found\n\n");
		exit(-1);
	}
	else {
		printf("\n\tvisual %p selected\n", (void *)vi->visualid); /* %p creates hexadecimal output like in glxinfo */
	}

	Colormap cmap = XCreateColormap(dpy, root, vi->visual, AllocNone);

	XSetWindowAttributes swa;
	swa.colormap = cmap;
	swa.event_mask = KeyPressMask | KeyReleaseMask | StructureNotifyMask | ButtonPressMask | ButtonReleaseMask | PointerMotionMask;

	Window win = XCreateWindow(dpy, root, 50, 50, 1280, 720, 0, vi->depth, InputOutput, vi->visual, CWColormap | CWEventMask, &swa);

	XMapWindow(dpy, win);

	Atom WM_DELETE_WINDOW = XInternAtom(dpy, "WM_DELETE_WINDOW", 0);
	XSetWMProtocols(dpy, win, &WM_DELETE_WINDOW, 1);

	XStoreName(dpy, win, "BNGine Runtime");

	GLXContext glc = glXCreateContext(dpy, vi, NULL, GL_TRUE);
	glXMakeCurrent(dpy, win, glc);

	InitGlExts();

	AppEventFunction(PostInitEvent());

	XWindowAttributes gwa;
	bool isRunning = true;
	while (isRunning) {
		XEvent ev;
		while (XCheckIfEvent(dpy, &ev, evtPred, NULL)) {
			MouseButtonEvent mouseEvt;
			if (ev.type == ButtonPress) {
				mouseEvt.pressOrRelease = BNS_BUTTON_PRESS;
				if (ev.xbutton.button == 1) {
					mouseEvt.button = MouseButton::PRIMARY;
					AppEventFunction(mouseEvt);
				}
				else if (ev.xbutton.button == 3) {
					mouseEvt.button = MouseButton::SECONDARY;
					AppEventFunction(mouseEvt);
				}
			}
			else if (ev.type == ButtonRelease) {
				mouseEvt.pressOrRelease = BNS_BUTTON_RELEASE;
				if (ev.xbutton.button == 1) {
					mouseEvt.button = MouseButton::PRIMARY;
					AppEventFunction(mouseEvt);
				}
				else if (ev.xbutton.button == 3) {
					mouseEvt.button = MouseButton::SECONDARY;
					AppEventFunction(mouseEvt);
				}
			}
			else if (ev.type == MotionNotify) {
				MousePosEvent evt;
				evt.x = ev.xmotion.x;
				evt.y = ev.xmotion.y;
				AppEventFunction(evt);
			}
			else if (ev.type == KeyPress) {
				unsigned char key = KeyStrokeCodeToAscii(dpy, ev.xkey.keycode);
				KeyButtonEvent evt;
				evt.keyCode = key;
				evt.pressOrRelease = BNS_BUTTON_PRESS;
				AppEventFunction(evt);
			}
			else if (ev.type == KeyRelease) {
				unsigned char key = KeyStrokeCodeToAscii(dpy, ev.xkey.keycode);
				KeyButtonEvent evt;
				evt.keyCode = key;
				evt.pressOrRelease = BNS_BUTTON_RELEASE;
				AppEventFunction(evt);
			}
			else if (ev.type == ConfigureNotify) {
				// XConfigureEvent xce = ev.xconfigure;

			}
			else if (ev.type == ClientMessage) {
				isRunning = false;
			}
		}

		XGetWindowAttributes(dpy, win, &gwa);
		
		{
			WindowResizeEvent evt;
			evt.width = gwa.width;
			evt.height = gwa.height;
			AppEventFunction(evt);
		}
		{
			UpdateEvent evt;
			evt.shouldContinue = &isRunning;
			AppEventFunction(evt);
		}

		glXSwapBuffers(dpy, win);

		usleep(16 * 1000);

	}

	AppEventFunction(ShutDownEvent());

	glXMakeCurrent(dpy, None, NULL);
	glXDestroyContext(dpy, glc);
	XDestroyWindow(dpy, win);
	XCloseDisplay(dpy);

	return 0;
}
