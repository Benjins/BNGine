#include<X11/X.h>
#include<X11/Xlib.h>

#include "../core/Scene.h"
#include "../gfx/GLExtInit.h"

#include <stdio.h>

#include <unistd.h>

int evtPred(Display* d, XEvent* evt, XPointer xp){
	return 1;
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
	swa.event_mask = ExposureMask | KeyPressMask;
	
	Window win = XCreateWindow(dpy, root, 50, 50, 1280, 720, 0, vi->depth, InputOutput, vi->visual, CWColormap | CWEventMask, &swa);
	
	XMapWindow(dpy, win);
	
	XStoreName(dpy, win, "BNGine Runtime");
	
	GLXContext glc = glXCreateContext(dpy, vi, NULL, GL_TRUE);
	glXMakeCurrent(dpy, win, glc);
	
	InitGlExts();

	Scene scn;

	glClearColor(1.0f, 0.0f, 1.0f, 1.0f);
	glEnable(GL_DEPTH_TEST);
	glViewport(0, 0, 1280, 720);
	glLoadIdentity();

	XEvent xev;
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
		    	//currMouseX = ev.xmotion.x;
		    	//currMouseY = ev.xmotion.y;
		    }
		    else if (ev.type == KeyPress){
		    	//unsigned char key = KeyCodeToAscii(display, ev.xkey.keycode);
		    	//keyStates[key] = PRESS;
		    }
		    else if (ev.type == KeyRelease){
		    	//unsigned char key = KeyCodeToAscii(display, ev.xkey.keycode);
		    	//keyStates[key] = RELEASE;
		    }
		    else if (ev.type == ConfigureNotify) {
		        //XConfigureEvent xce = ev.xconfigure;
		        
		    }
		    else if (ev.type == ClientMessage){
		    	isRunning = 0;
		    }
		}
		
		XGetWindowAttributes(dpy, win, &gwa);
		glViewport(0, 0, gwa.width, gwa.height);
		
		scn.Update();
		scn.Render();
		
		glXSwapBuffers(dpy, win);		
		
		usleep(16*1000);
		
	}


	glXMakeCurrent(dpy, None, NULL);
	glXDestroyContext(dpy, glc);
	XDestroyWindow(dpy, win);
	XCloseDisplay(dpy);
			
	return 0;
}
