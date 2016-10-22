#include<X11/X.h>
#include<X11/Xlib.h>
#include <X11/XKBlib.h>

#include "../core/Scene.h"
#include "../gfx/GLExtInit.h"

#include <stdio.h>

#include <unistd.h>

int evtPred(Display* d, XEvent* evt, XPointer xp){
	return 1;
}

unsigned char KeyStrokeCodeToAscii(Display* display, int keycode){
	KeySym ksym = XkbKeycodeToKeysym( display, keycode, 0, keycode & ShiftMask ? 1 : 0);
	
	//There's got to be a better way.  In fact there is.  I just haven't done it yet.
	if(ksym >= 'a' && ksym <= 'z'){
		return ksym & ~('a' ^ 'A');
	}
	else{	
		return ksym;
	}
}

int main(int argc, char** argv){

	Display* dpy = XOpenDisplay(NULL);
	
	if(dpy == NULL) {
		printf("\n\tcannot connect to X server\n\n");
		exit(-1);
	}	
	
	Window root = DefaultRootWindow(dpy);
	
	GLint att[] = { GLX_RGBA, GLX_DEPTH_SIZE, 24, GLX_DOUBLEBUFFER, None };
	
	XVisualInfo* vi = glXChooseVisual(dpy, 0, att);
	
	if(vi == NULL) {
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

	Scene scn;

	glClearColor(1.0f, 0.0f, 1.0f, 1.0f);
	glEnable(GL_DEPTH_TEST);
	glViewport(0, 0, 1280, 720);
	glLoadIdentity();

	XWindowAttributes gwa;

	scn.StartUp();

	bool isRunning = true;
	while(isRunning) {
		XEvent ev;
		while(XCheckIfEvent(dpy, &ev, evtPred, NULL)){
			if(ev.type == ButtonPress){
		    	if(ev.xbutton.button == 1){
		    		//mouseState = 2;
		    	}
		    }
		    else if(ev.type == ButtonRelease){
		    	if(ev.xbutton.button == 1){
		    		//mouseState = 1;
		    	}
		    }
		    else if(ev.type == MotionNotify){
		    	int currMouseX = ev.xmotion.x;
		    	int currMouseY = ev.xmotion.y;
		    	GlobalScene->input.SetCursorPos(currMouseX, currMouseY);
		    }
		    else if (ev.type == KeyPress){
		    	unsigned char key = KeyStrokeCodeToAscii(dpy, ev.xkey.keycode);
				GlobalScene->input.KeyPressed(key);
		    }
		    else if (ev.type == KeyRelease){
		    	unsigned char key = KeyStrokeCodeToAscii(dpy, ev.xkey.keycode);
				GlobalScene->input.KeyReleased(key);
		    }
		    else if (ev.type == ConfigureNotify) {
		       // XConfigureEvent xce = ev.xconfigure;
		        
		    }
		    else if (ev.type == ClientMessage){
		    	isRunning = 0;
		    }
		}
		
		XGetWindowAttributes(dpy, win, &gwa);
		GlobalScene->cam.widthPixels = gwa.width;
		GlobalScene->cam.heightPixels = gwa.height;
		
		scn.Update();
		scn.Render();
		
		glXSwapBuffers(dpy, win);		
		
		usleep(16*1000);
		
	}

	scn.ShutDown();

	glXMakeCurrent(dpy, None, NULL);
	glXDestroyContext(dpy, glc);
	XDestroyWindow(dpy, win);
	XCloseDisplay(dpy);
			
	return 0;
}
