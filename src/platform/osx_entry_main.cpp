#include <OpenGl/gl.h>
#include <OpenGl/glu.h>
#include <OpenGL/glext.h>
#include <OpenGL/OpenGL.h>
#include <Carbon/Carbon.h>

#include <AGL/agl.h>

#include <stdio.h>

#include "../gfx/GLExtInit.h"
#include "../app/app_funcs.h"

static pascal OSErr quitEventHandler( const AppleEvent *appleEvt, AppleEvent *reply, SInt32 refcon )
{
	exit( 0 );
	return false;
}

#define QZ_ESCAPE		0x35
#define QZ_PAGEUP		0x74
#define QZ_PAGEDOWN		0x79
#define QZ_RETURN		0x24
#define QZ_UP			0x7E
#define QZ_SPACE		0x31
#define QZ_LEFT			0x7B
#define QZ_DOWN			0x7D
#define QZ_RIGHT		0x7C
#define QZ_W			0x0D
#define QZ_A		    0x00
#define QZ_S			0x01
#define QZ_D			0x02
#define QZ_ONE			0x12
#define QZ_TWO			0x13
#define QZ_THREE		0x14
#define QZ_FOUR			0x15
#define QZ_FIVE			0x17
#define QZ_SIX			0x16
#define QZ_SEVEN		0x1A
#define QZ_EIGHT		0x1C
#define QZ_NINE			0x19
#define QZ_ZERO			0x1D

pascal OSStatus keyboardEventHandler( EventHandlerCallRef nextHandler, EventRef event, void * userData )
{
	UInt32 eventClass = GetEventClass( event );
	UInt32 eventKind = GetEventKind( event );
	
	if ( eventClass == kEventClassKeyboard )
	{
		char macCharCodes;
		UInt32 macKeyCode;
		UInt32 macKeyModifiers;

		GetEventParameter( event, kEventParamKeyMacCharCodes, typeChar, NULL, sizeof(macCharCodes), NULL, &macCharCodes );
		GetEventParameter( event, kEventParamKeyCode, typeUInt32, NULL, sizeof(macKeyCode), NULL, &macKeyCode );
		GetEventParameter( event, kEventParamKeyModifiers, typeUInt32, NULL, sizeof(macKeyModifiers), NULL, &macKeyModifiers );

		if ( eventKind == kEventRawKeyDown )
		{
			switch ( macKeyCode )
			{
				case QZ_SPACE: /*TODO*/ break;
				case QZ_W: AppKeyDown('W'); break;
				case QZ_A: AppKeyDown('A'); break;
				case QZ_S: AppKeyDown('S'); break;
				case QZ_D: AppKeyDown('D'); break;
				
				default:
				{
					// note: for "discovering" keycodes for new keys :)
					/*
					char title[] = "Message";
					char text[64];
					sprintf( text, "key=%x", (int) macKeyCode );
					Str255 msg_title;
					Str255 msg_text;
					c2pstrcpy( msg_title, title );
					c2pstrcpy( msg_text, text );
					StandardAlert( kAlertStopAlert, (ConstStr255Param) msg_title, (ConstStr255Param) msg_text, NULL, NULL);
					*/
					return eventNotHandledErr;
				}
			}
		}
		else if ( eventKind == kEventRawKeyUp )
		{
			switch ( macKeyCode )
			{
				case QZ_SPACE: /*TODO*/ break;
				case QZ_W: AppKeyUp('W'); break;
				case QZ_A: AppKeyUp('A'); break;
				case QZ_S: AppKeyUp('S'); break;
				case QZ_D: AppKeyUp('D'); break;

				default: return eventNotHandledErr;
			}
		}
	}
	else if (eventClass == kEventClassMouse)
	{
		EventMouseButton	button;
		Point	point;
		
		if (eventKind == kEventMouseDown ){
			GetEventParameter(event, kEventParamMouseButton, typeMouseButton, NULL, sizeof(EventMouseButton), NULL, &button);
			GetEventParameter(event, kEventParamMouseLocation, typeQDPoint, NULL, sizeof(Point), NULL, &point);
			if (button == kEventMouseButtonPrimary){
				AppMouseDown(0);
			}
		}
		else if(eventKind == kEventMouseUp){
			GetEventParameter(event, kEventParamMouseButton, typeMouseButton, NULL, sizeof(EventMouseButton), NULL, &button);
			GetEventParameter(event, kEventParamMouseLocation, typeQDPoint, NULL, sizeof(Point), NULL, &point);
			if (button == kEventMouseButtonPrimary){
				AppMouseUp(0);
			}
		}
		else if(eventKind == kEventMouseMoved){
			GetEventParameter(event, kEventParamMouseLocation, typeQDPoint, NULL, sizeof(Point), NULL, &point);
			AppMouseMove(point.h, point.v);
		}
		else if(eventKind == kEventMouseDragged){
			GetEventParameter(event, kEventParamMouseLocation, typeQDPoint, NULL, sizeof(Point), NULL, &point);
			AppMouseMove(point.h, point.v);
		}
	}

	return noErr;
}

static AGLContext setupAGL( WindowRef window, int width, int height )
{	
	if ( (Ptr) kUnresolvedCFragSymbolAddress == (Ptr) aglChoosePixelFormat )
		return 0;

	GLint attributes[] = { AGL_RGBA, 
						   AGL_DOUBLEBUFFER, 
						   AGL_DEPTH_SIZE, 32, 
						   AGL_NO_RECOVERY,
						   AGL_NONE,
						   AGL_NONE };	

	AGLPixelFormat format = NULL;

	format = aglChoosePixelFormat( NULL, 0, attributes );

	if ( !format ) 
		return 0;

	AGLContext context = aglCreateContext( format, 0 );

	if ( !context )
		return 0;

	aglDestroyPixelFormat( format );

	aglSetWindowRef( context, window );

	aglSetCurrentContext( context );

	return context;
}

static void cleanupAGL( AGLContext context )
{
	aglDestroyContext( context );
}

WindowRef window;
AGLContext context;

int GetDisplayWidth()
{
	return CGDisplayPixelsWide( kCGDirectMainDisplay );
}

int GetDisplayHeight()
{
	return CGDisplayPixelsHigh( kCGDirectMainDisplay );
}

bool OpenDisplay( const char title[], int width, int height )
{
	int screenWidth = GetDisplayWidth();
	int screenHeight = GetDisplayHeight();

	Rect rect;
	rect.left = ( screenWidth - width ) / 2;
	rect.top = ( screenHeight - height ) / 2;
	rect.right = rect.left + width;
	rect.bottom = rect.top + height;
	
	OSErr result = CreateNewWindow( kDocumentWindowClass, 
					                ( kWindowStandardDocumentAttributes |
//									  kWindowNoTitleBarAttribute |
									  kWindowStandardHandlerAttribute ) &
									 ~kWindowResizableAttribute,
					                &rect, &window );
	
	if ( result != noErr )
		return false;

	SetWindowTitleWithCFString( window, CFStringCreateWithCString( 0, title, CFStringGetSystemEncoding() ) );

	context = setupAGL( window, width, height );

	if ( !context )
		return false;
	
	ShowWindow( window );
	
	SelectWindow( window );

	// install standard event handlers
	
    InstallStandardEventHandler( GetWindowEventTarget( window ) );

	// install keyboard handler
	{
		EventTypeSpec eventTypes[6] = {
		  {kEventClassMouse,	 kEventMouseDown},
		  {kEventClassMouse,	 kEventMouseUp},
		  {kEventClassMouse,	 kEventMouseMoved},
		  {kEventClassMouse,	 kEventMouseDragged},
		  {kEventClassKeyboard,	 kEventRawKeyDown},
		  {kEventClassKeyboard,	 kEventRawKeyUp}
		};
		EventHandlerUPP handlerUPP = NewEventHandlerUPP( keyboardEventHandler );

		InstallApplicationEventHandler( handlerUPP, 6, eventTypes, NULL, NULL );
	}
	
	// install quit handler (via apple events)

	AEInstallEventHandler( kCoreEventClass, kAEQuitApplication, NewAEEventHandlerUPP(quitEventHandler), 0, false );

	return true;
}

void UpdateDisplay( int interval = 1 )
{
	GLint swapInterval = interval;
	aglSetInteger( context, AGL_SWAP_INTERVAL, &swapInterval );
	aglSwapBuffers( context );


	// process events
	
	EventRef event = 0; 
	OSStatus status = ReceiveNextEvent( 0, NULL, 0.0f, kEventRemoveFromQueue, &event ); 
	if ( status == noErr && event )
	{ 
		bool sendEvent = true;

		// note: required for menu bar to work properly
		if ( GetEventClass( event ) == kEventClassMouse && GetEventKind( event ) == kEventMouseDown )
		{
			WindowRef window;
			Point location;
			GetEventParameter( event, kEventParamMouseLocation, typeQDPoint, NULL, sizeof(Point), NULL, &location );
			if ( MacFindWindow( location, &window ) == inMenuBar )
			{
				sendEvent = false;
				MenuSelect( location );
			}
		}

		if ( sendEvent )
			SendEventToEventTarget( event, GetEventDispatcherTarget() ); 
		
		ReleaseEvent( event );
	}
}

void CloseDisplay()
{	
    cleanupAGL( context );
	context = 0;
	
	DisposeWindow( (WindowPtr) window );
	window = 0;
}

int main( int argc, char * argv[] ){
	AppPreInit(argc, argv);
	
	const int DisplayWidth = 1280;
	const int DisplayHeight = 720;
	
	if (!OpenDisplay("BNgine", DisplayWidth, DisplayHeight)){
		fprintf( stderr, "failed to open display!\n" );
		return 1;
	}
	
	InitGlExts();

	AppPostInit(argc, argv);
	
	bool isRunning = true;
	while (isRunning){
		
		isRunning &= AppUpdate(argc, argv);
		
		UpdateDisplay();
	}
	
	AppShutdown(argc, argv);
	
	CloseDisplay();
	
	return 0;
}