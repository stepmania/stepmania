#import "global.h"
#import "LowLevelWindow_Cocoa.h"
#import "DisplayResolutions.h"
#import "RageLog.h"
#import "RageUtil.h"
#import "RageThreads.h"
#import "arch/ArchHooks/ArchHooks.h"
#import "archutils/Darwin/SMMainThread.h"

#import <Cocoa/Cocoa.h>
#import <OpenGl/OpenGl.h>
#import <mach-o/dyld.h>

static const unsigned int g_iStyleMask = NSTitledWindowMask | NSClosableWindowMask |
					 NSMiniaturizableWindowMask | NSResizableWindowMask;
static bool g_bResized;
static int g_iWidth;
static int g_iHeight;
static RageMutex g_ResizeLock( "Window resize lock." );

// Simple helper class
class AutoreleasePool
{
	NSAutoreleasePool *m_Pool;
	
public:
	AutoreleasePool() { m_Pool = [[NSAutoreleasePool alloc] init]; }
	~AutoreleasePool() { [m_Pool release]; }
};

// These can nest.
#define POOL2(x) AutoreleasePool pool ## x
#define POOL1(x) POOL2(x)
#define POOL POOL1(__LINE__)

// Window delegate class
@interface SMWindowDelegate : NSObject
- (void) windowDidBecomeKey:(NSNotification *)aNotification;
- (void) windowDidResignKey:(NSNotification *)aNotification;
- (void) windowWillClose:(NSNotification *)aNotification;
- (void) windowDidResize:(NSNotification *)aNotification;
// XXX maybe use whichever screen contains the window? Hard for me to test though.
//- (void) windowDidChangeScreen:(NSNotification *)aNotification;
@end

@implementation SMWindowDelegate
- (void) windowDidBecomeKey:(NSNotification *)aNotification
{
	HOOKS->SetHasFocus( true );
}

- (void) windowDidResignKey:(NSNotification *)aNotification
{
	HOOKS->SetHasFocus( false );
}

- (void) windowWillClose:(NSNotification *)aNotification
{
	ArchHooks::SetUserQuit();
}

- (void) windowDidResize:(NSNotification *)aNotification
{
	id window = [aNotification object];
	NSSize size = [NSWindow contentRectForFrameRect:[window frame] styleMask:g_iStyleMask].size;
	
	LockMut( g_ResizeLock );
	g_bResized = true;
	g_iWidth = int( size.width );
	g_iHeight = int( size.height );
}
	
@end

LowLevelWindow_Cocoa::LowLevelWindow_Cocoa() : m_WindowContext(nil), m_FullScreenContext(nil),
	 m_bSharingContexts(false), m_CurrentDisplayMode(NULL)
{
#if CONCURRENT_RENDERING
	m_ConcurrentWindowContext = nil;
	m_ConcurrentFullScreenContext = nil;
#endif
	POOL;
	NSRect rect = { {0, 0}, {0, 0} };
	SMMainThread *mt = [[SMMainThread alloc] init];
	
	m_Window = [[NSWindow alloc] initWithContentRect:rect
					      styleMask:g_iStyleMask
						backing:NSBackingStoreBuffered
						  defer:YES];
	
	ADD_ACTIONb( mt, m_Window, setExcludedFromWindowsMenu:, YES );
	ADD_ACTIONb( mt, m_Window, useOptimizedDrawing:, YES );
	ADD_ACTIONb( mt, m_Window, setReleasedWhenClosed:, NO );
	// setDelegate: does not retain the delegate; however, we don't (auto)release it.
	ADD_ACTION1( mt, m_Window, setDelegate:, [[SMWindowDelegate alloc] init] );
	
	[mt performOnMainThread];
	[mt release];
	m_CurrentParams.windowed = true; // We are essentially windowed to begin with.
}

LowLevelWindow_Cocoa::~LowLevelWindow_Cocoa()
{
	POOL;
	ShutDownFullScreen();
	
	SMMainThread *mt = [[SMMainThread alloc] init];
	
	// We need to release the window's delegate now.
	id delegate = [m_Window delegate];
	
	ADD_ACTION1( mt, m_Window, setDelegate:, nil );
	ADD_ACTION1( mt, m_Window, orderOut:, nil );
	ADD_ACTION0( mt, m_Window, release );
	
	[m_WindowContext clearDrawable];
	[mt performOnMainThread];
	[delegate release];
	[mt release];
	[m_WindowContext release];
	[m_FullScreenContext release];
}

void *LowLevelWindow_Cocoa::GetProcAddress( CString s )
{
	// http://developer.apple.com/qa/qa2001/qa1188.html
	// Both functions mentioned in there are deprecated in 10.4.
	const CString& symbolName( '_' + s );
	const uint32_t count = _dyld_image_count();
	NSSymbol symbol = NULL;
	const uint32_t options = NSLOOKUPSYMBOLINIMAGE_OPTION_RETURN_ON_ERROR;
	
	for( uint32_t i = 0; i < count && !symbol; ++i )
		symbol = NSLookupSymbolInImage( _dyld_get_image_header(i), symbolName, options );
	return symbol ? NSAddressOfSymbol( symbol ) : NULL;
}

static void SetOGLParameters( NSOpenGLContext *context )
{
	long val = 64;
	
	[context setValues:&val forParameter:NSOpenGLContextParameter(280)];
	[context setValues:&val forParameter:NSOpenGLContextParameter(284)];
}

CString LowLevelWindow_Cocoa::TryVideoMode( const VideoModeParams& p, bool& newDeviceOut )
{
	// Always set these params.
	m_CurrentParams.bSmoothLines = p.bSmoothLines;
	m_CurrentParams.bTrilinearFiltering = p.bTrilinearFiltering;
	m_CurrentParams.bAnisotropicFiltering = p.bAnisotropicFiltering;
	m_CurrentParams.interlaced = p.interlaced;
	m_CurrentParams.PAL = p.PAL;
	m_CurrentParams.fDisplayAspectRatio = p.fDisplayAspectRatio;
	
#define X(x) p.x == m_CurrentParams.x
	if( X(windowed) && X(bpp) && X(width) && X(height) && X(rate) && X(vsync) )
		return 0;
#undef X
	
	POOL;
	newDeviceOut = false;
	
	NSRect contentRect = { { 0, 0 }, { p.width, p.height } };
	SMMainThread *mt = [[[SMMainThread alloc] init] autorelease];
	
	// Change the window and the title
	ADD_ACTIONn( mt, m_Window, setContentSize:, 1, &contentRect.size );
	ADD_ACTION1( mt, m_Window, setTitle:, [NSString stringWithUTF8String:p.sWindowTitle.c_str()] );
		
	m_CurrentParams.width = p.width;
	m_CurrentParams.height = p.height;
	
	ASSERT( p.bpp == 16 || p.bpp == 32 );
	
	if( p.bpp != m_CurrentParams.bpp || !m_WindowContext )
	{
		NSOpenGLPixelFormat *pixelFormat;
		NSOpenGLPixelFormatAttribute attrs[] = {
			NSOpenGLPFANoRecovery, // so we can share with the full screen context
			NSOpenGLPFADoubleBuffer,
			NSOpenGLPFAAccelerated,
			NSOpenGLPFADepthSize, NSOpenGLPixelFormatAttribute(16),
			NSOpenGLPFAColorSize, NSOpenGLPixelFormatAttribute(p.bpp == 16 ? 16 : 24),
			NSOpenGLPixelFormatAttribute(0)
		};
		
		pixelFormat = [[[NSOpenGLPixelFormat alloc] initWithAttributes:attrs] autorelease];
		
		if( pixelFormat == nil )
		{
			[mt performOnMainThread];
			return "Failed to set the windowed pixel format.";
		}
		id nextContext = [[NSOpenGLContext alloc] initWithFormat:pixelFormat shareContext:nil];

		if( nextContext == nil )
		{
			[mt performOnMainThread];
			return "Failed to create windowed OGL context.";
		}
		SetOGLParameters( nextContext );
		newDeviceOut = true;
		[m_WindowContext clearDrawable];
		[m_WindowContext release];
		m_WindowContext = nextContext;
		
#if CONCURRENT_RENDERING
		[m_ConcurrentWindowContext release];
		m_ConcurrentWindowContext = [[NSOpenGLContext alloc] initWithFormat:pixelFormat
								      shareContext:m_WindowContext];
		if( m_ConcurrentWindowContext )
			SetOGLParameters( m_ConcurrentWindowContext );
#endif

		// We need to recreate the full screen context as well.
		[m_FullScreenContext clearDrawable];
		[m_FullScreenContext release];
		m_FullScreenContext = nil;
		m_CurrentParams.bpp = p.bpp;
		m_bSharingContexts = false;
	}
	if( p.windowed )
	{		
		ShutDownFullScreen();
		
		ADD_ACTION0( mt, m_Window, center );
		ADD_ACTION1( mt, m_Window, makeKeyAndOrderFront:, nil );
		
		[mt performOnMainThread];
		[m_WindowContext setView:[m_Window contentView]];
		[m_WindowContext update];
		[m_WindowContext makeCurrentContext];
		m_CurrentParams.windowed = true;
		SetActualParamsFromMode( CGDisplayCurrentMode(kCGDirectMainDisplay) );
		m_CurrentParams.vsync = p.vsync; // hack

		newDeviceOut = newDeviceOut || !m_bSharingContexts;
		return CString();
	}
	[mt performOnMainThread];
	int result = ChangeDisplayMode( p );
	
	if( result )
	{
		return ssprintf( "Failed to switch to full screen:%d x %d @ %d. Error %d.",
				 p.width, p.height, p.rate, result );
	}
	if( m_FullScreenContext == nil )
	{
		NSOpenGLPixelFormatAttribute attrs[] = {
			NSOpenGLPFAFullScreen,
			// Choose which screen to use: the main one.
			NSOpenGLPFAScreenMask,
			NSOpenGLPixelFormatAttribute( CGDisplayIDToOpenGLDisplayMask(kCGDirectMainDisplay) ),
			
			NSOpenGLPFADoubleBuffer,
			NSOpenGLPFAAccelerated,
			NSOpenGLPFADepthSize, NSOpenGLPixelFormatAttribute(16),
			NSOpenGLPFAColorSize, NSOpenGLPixelFormatAttribute(p.bpp == 16 ? 16 : 24),
			NSOpenGLPixelFormatAttribute(0)
		};
		NSOpenGLPixelFormat *pixelFormat;
		
		// Autorelease to simplify logic.
		pixelFormat = [[[NSOpenGLPixelFormat alloc] initWithAttributes:attrs] autorelease];
		if( pixelFormat == nil )
		{
			ShutDownFullScreen(); // If this fails, we need to leave full screen.
			return "Failed to set full screen pixel format.";
		}
		
		m_FullScreenContext = [[NSOpenGLContext alloc] initWithFormat:pixelFormat shareContext:m_WindowContext];
		if( !(m_bSharingContexts = m_FullScreenContext != nil) )
		{
			LOG->Warn( "Failed to share openGL contexts." );
			m_FullScreenContext = [[NSOpenGLContext alloc] initWithFormat:pixelFormat shareContext:nil];
			if( !m_FullScreenContext )
			{
				ShutDownFullScreen(); // Same here.
				return "Failed to create full screen openGL context.";
			}
		}
		SetOGLParameters( m_FullScreenContext );
		
#if CONCURRENT_RENDERING
		[m_ConcurrentFullScreenContext release];
		m_ConcurrentFullScreenContext = [[NSOpenGLContext alloc] initWithFormat:pixelFormat
									  shareContext:m_WindowContext];
		if( m_ConcurrentFullScreenContext )
			SetOGLParameters( m_ConcurrentFullScreenContext );
#endif
	}
	long swap = p.vsync ? 1 : 0;
	
	[m_FullScreenContext setValues:&swap forParameter:NSOpenGLCPSwapInterval];
	[m_FullScreenContext setFullScreen];
	[m_FullScreenContext update];
	[m_FullScreenContext makeCurrentContext];
	m_CurrentParams.vsync = p.vsync;
	
	newDeviceOut = newDeviceOut || !m_bSharingContexts;
	return CString();
}

void LowLevelWindow_Cocoa::ShutDownFullScreen()
{
	if( m_CurrentParams.windowed )
		return;
	ASSERT( m_CurrentDisplayMode );
	[m_FullScreenContext clearDrawable];
	
#if CONCURRENT_RENDERING
	[m_ConcurrentFullScreenContext clearDrawable];
#endif
	
	CGDisplayErr err = CGDisplaySwitchToMode( kCGDirectMainDisplay, m_CurrentDisplayMode );
	
	ASSERT( err == kCGErrorSuccess );
	CGDisplayShowCursor( kCGDirectMainDisplay );
	err = CGReleaseAllDisplays();
	ASSERT( err == kCGErrorSuccess );
	SetActualParamsFromMode( m_CurrentDisplayMode );
	// We don't own this so we cannot release it.
	m_CurrentDisplayMode = NULL;
	m_CurrentParams.windowed = true;
}

#if CONCURRENT_RENDERING
bool LowLevelWindow_Cocoa::SupportsThreadedRendering()
{
	return m_CurrentParams.windowed ? m_ConcurrentWindowContext : m_ConcurrentFullScreenContext;
}

void LowLevelWindow_Cocoa::BeginConcurrentRendering()
{
	if( m_CurrentParams.windowed )
	{
		[m_ConcurrentWindowContext setView:[m_Window contentView]];
		[m_ConcurrentWindowContext update];
		[m_ConcurrentWindowContext makeCurrentContext];
	}
	else
	{
		[m_ConcurrentFullScreenContext setFullScreen];
		[m_ConcurrentFullScreenContext update];
		[m_ConcurrentFullScreenContext makeCurrentContext];
	}
}

void LowLevelWindow_Cocoa::EndConcurrentRendering()
{
	[NSOpenGLContext clearCurrentContext];
}
#endif

int LowLevelWindow_Cocoa::ChangeDisplayMode( const VideoModeParams& p )
{	
	CFDictionaryRef mode = NULL;
	CFDictionaryRef newMode;
	CGDisplayErr err;
	
	if( m_CurrentParams.windowed )
	{
		if( (err = CGCaptureAllDisplays()) != kCGErrorSuccess )
			return err;
		// Only hide the first time we go to full screen.
		CGDisplayHideCursor( kCGDirectMainDisplay );	
		mode = CGDisplayCurrentMode( kCGDirectMainDisplay );
	}
	
	if( p.rate == REFRESH_DEFAULT )
		newMode = CGDisplayBestModeForParameters( kCGDirectMainDisplay, p.bpp, p.width, p.height, NULL );
	else
		newMode = CGDisplayBestModeForParametersAndRefreshRate( kCGDirectMainDisplay, p.bpp,
									p.width, p.height, p.rate, NULL );
	
	
	err = CGDisplaySwitchToMode( kCGDirectMainDisplay, newMode );
	
	if( err != kCGErrorSuccess )
		return err; // We don't own mode, don't release it.
	SetActualParamsFromMode( newMode );
	
	if( m_CurrentParams.windowed )
	{
		m_CurrentDisplayMode = mode;
		m_CurrentParams.windowed = false;
	}
	
	return 0;
}

void LowLevelWindow_Cocoa::SetActualParamsFromMode( CFDictionaryRef mode )
{
	SInt32 width, height, rate, bpp;
	
#define X(prop,var) CFNumberGetValue( CFNumberRef(CFDictionaryGetValue(mode, CFSTR(prop))), kCFNumberSInt32Type, &var )
	X( "Width", width );
	X( "Height", height );
	X( "RefreshRate", rate );
	X( "BitsPerPixel", bpp );
#undef X
	if( !m_CurrentParams.windowed )
	{
		long swap;
		
		m_CurrentParams.width = width;
		m_CurrentParams.height = height;
		CGLGetParameter( CGLGetCurrentContext(), kCGLCPSwapInterval, &swap );
		m_CurrentParams.vsync = swap != 0;
	}
	m_CurrentParams.bpp = bpp;
	m_CurrentParams.rate = rate;
}

void LowLevelWindow_Cocoa::GetDisplayResolutions( DisplayResolutions &dr ) const
{
	CFArrayRef modes = CGDisplayAvailableModes( kCGDirectMainDisplay );
	ASSERT( modes );
	const CFIndex count = CFArrayGetCount( modes );
	
	for( CFIndex i = 0; i < count; ++i )
	{
		CFDictionaryRef dict = (CFDictionaryRef)CFArrayGetValueAtIndex( modes, i );
		CFNumberRef num = (CFNumberRef)CFDictionaryGetValue( dict, CFSTR("Width") );
		SInt32 width, height;
		
		if( !num || !CFNumberGetValue(num, kCFNumberSInt32Type, &width) )
			continue;
		num = (CFNumberRef)CFDictionaryGetValue( dict, CFSTR("Height") );
		if( !num || !CFNumberGetValue(num, kCFNumberSInt32Type, &height) )
			continue;
		
		DisplayResolution res = { width, height };
		dr.s.insert( res );
	}
	// Do not release modes! We don't own it here.
}

void LowLevelWindow_Cocoa::SwapBuffers()
{
	[(m_CurrentParams.windowed ? m_WindowContext : m_FullScreenContext) flushBuffer];
}

void LowLevelWindow_Cocoa::Update()
{
	LockMutex lock( g_ResizeLock );
	if( likely(!g_bResized) )
		return;
	g_bResized = false;
	if( m_CurrentParams.width == g_iWidth && m_CurrentParams.height == g_iHeight )
		return;
	m_CurrentParams.width = g_iWidth;
	m_CurrentParams.height = g_iHeight;
	lock.Unlock(); // Unlock before calling ResolutionChanged().
	[m_WindowContext update];
	DISPLAY->ResolutionChanged();
}
