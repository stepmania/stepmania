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
#import <OpenGL/gl.h>
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

LowLevelWindow_Cocoa::LowLevelWindow_Cocoa() : m_Context(nil), m_BGContext(nil), m_CurrentDisplayMode(NULL)
{
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
	
	[m_Context clearDrawable];
	[m_Context release];
	[mt performOnMainThread];
	[delegate release];
	[mt release];
}

void *LowLevelWindow_Cocoa::GetProcAddress( RString s )
{
	// http://developer.apple.com/qa/qa2001/qa1188.html
	// Both functions mentioned in there are deprecated in 10.4.
	const RString& symbolName( '_' + s );
	const uint32_t count = _dyld_image_count();
	NSSymbol symbol = NULL;
	const uint32_t options = NSLOOKUPSYMBOLINIMAGE_OPTION_RETURN_ON_ERROR;
	
	for( uint32_t i = 0; i < count && !symbol; ++i )
		symbol = NSLookupSymbolInImage( _dyld_get_image_header(i), symbolName, options );
	return symbol ? NSAddressOfSymbol( symbol ) : NULL;
}

static NSOpenGLContext *CreateOGLContext( int iColorDepth, bool bWindowed, NSOpenGLContext *share, bool &bShared )
{
	NSOpenGLPixelFormat *pixelFormat;
	
	if( bWindowed )
	{
		NSOpenGLPixelFormatAttribute attrs[] = {
			NSOpenGLPFANoRecovery, // so we can share with the full screen context
			NSOpenGLPFADoubleBuffer,
			NSOpenGLPFAAccelerated,
			NSOpenGLPFADepthSize, NSOpenGLPixelFormatAttribute(16),
			NSOpenGLPFAColorSize, NSOpenGLPixelFormatAttribute(iColorDepth == 16 ? 16 : 24),
			NSOpenGLPixelFormatAttribute(0)
		};
		
		pixelFormat = [[NSOpenGLPixelFormat alloc] initWithAttributes:attrs];
	}
	else
	{
		NSOpenGLPixelFormatAttribute attrs[] = {
			NSOpenGLPFAFullScreen,
			// Choose which screen to use: the main one.
			NSOpenGLPFAScreenMask,
			NSOpenGLPixelFormatAttribute( CGDisplayIDToOpenGLDisplayMask(kCGDirectMainDisplay) ),
			
			NSOpenGLPFADoubleBuffer,
			NSOpenGLPFAAccelerated,
			NSOpenGLPFADepthSize, NSOpenGLPixelFormatAttribute(16),
			NSOpenGLPFAColorSize, NSOpenGLPixelFormatAttribute(iColorDepth == 16 ? 16 : 24),
			NSOpenGLPixelFormatAttribute(0)
		};
		pixelFormat = [[NSOpenGLPixelFormat alloc] initWithAttributes:attrs];
	}
	
	if( !pixelFormat )
		return nil;
	
	NSOpenGLContext *context = [[NSOpenGLContext alloc] initWithFormat:pixelFormat shareContext:share];
	
	bShared = share && context;
	if( !context )
		context = [[NSOpenGLContext alloc] initWithFormat:pixelFormat shareContext:nil];
	
	[pixelFormat release];
	
	return context;
}		

RString LowLevelWindow_Cocoa::TryVideoMode( const VideoModeParams& p, bool& newDeviceOut )
{
	// Always set these params.
	m_CurrentParams.bSmoothLines = p.bSmoothLines;
	m_CurrentParams.bTrilinearFiltering = p.bTrilinearFiltering;
	m_CurrentParams.bAnisotropicFiltering = p.bAnisotropicFiltering;
	m_CurrentParams.interlaced = p.interlaced;
	m_CurrentParams.PAL = p.PAL;
	m_CurrentParams.fDisplayAspectRatio = p.fDisplayAspectRatio;
	
#define X(x) p.x != m_CurrentParams.x
	const bool bRebuildContext = X(bpp) || X(windowed) || !m_Context;
	const bool bChangeMode = X(width) || X(height) || X(rate) || bRebuildContext;
	const bool bChangeVsync = X(vsync) || bRebuildContext;
#undef X
	
	if( !bChangeMode && !bChangeVsync )
		return RString();
	
	POOL;
	newDeviceOut = false;
	
	NSRect contentRect = { { 0, 0 }, { p.width, p.height } };	
	
	ASSERT( p.bpp == 16 || p.bpp == 32 );
	
	if( p.windowed )
	{		
		if( bRebuildContext )
		{
			bool bShared;
			NSOpenGLContext *newContext = CreateOGLContext( p.bpp, true, m_Context, bShared );
			
			if( !newContext )
				return "Failed to create OGL context.";
			ShutDownFullScreen();
			[m_Context release];
			m_Context = newContext;
			newDeviceOut = !bShared;
			m_CurrentParams.bpp = p.bpp;
			[m_BGContext release];
			m_BGContext = nil;
#if 0
			m_BGContext = CreateOGLContext( p.bpp, true, m_Context, bShared );
			
			if( m_BGContext && !bShared )
			{
				[m_BGContext release];
				m_BGContext = nil;
			}
#endif
		}
		SMMainThread *mt = [[SMMainThread alloc] init];
		
		// Change the window and the title
		ADD_ACTIONn( mt, m_Window, setContentSize:, 1, &contentRect.size );
		ADD_ACTION1( mt, m_Window, setTitle:, [NSString stringWithUTF8String:p.sWindowTitle.c_str()] );		
		ADD_ACTION0( mt, m_Window, center );
		ADD_ACTION1( mt, m_Window, makeKeyAndOrderFront:, nil );
		
		[mt performOnMainThread];
		[mt release];
		[m_Context setView:[m_Window contentView]];
		[m_Context update];
		[m_Context makeCurrentContext];
		m_CurrentParams.windowed = true;
		SetActualParamsFromMode( CGDisplayCurrentMode(kCGDirectMainDisplay) );
		m_CurrentParams.vsync = p.vsync; // hack

		return RString();
	}
	if( bChangeMode )
	{
		int result = ChangeDisplayMode( p );
	
		if( result )
			return ssprintf( "Failed to switch to full screen:%d x %d @ %d. Error %d.",
					 p.width, p.height, p.rate, result );
	}

	if( bRebuildContext )
	{
		bool bShared;
		NSOpenGLContext *newContext = CreateOGLContext( p.bpp, false, m_Context, bShared );
		
		if( !newContext )
			return "Failed to create full screen OGL context.";
		[m_Context clearDrawable];
		[m_Context release];
		m_Context = newContext;
		newDeviceOut = !bShared;
		m_CurrentParams.bpp = p.bpp;
		[m_BGContext release];
		m_BGContext = CreateOGLContext( p.bpp, false, m_Context, bShared );
		
		if( m_BGContext && !bShared )
		{
			[m_BGContext release];
			m_BGContext = nil;
		}
	}
	
	[m_Context setFullScreen];
	[m_Context update];
	[m_Context makeCurrentContext];
	
	if( bChangeVsync )
	{
		long swap = p.vsync ? 1 : 0;

		[m_Context setValues:&swap forParameter:NSOpenGLCPSwapInterval];
		m_CurrentParams.vsync = p.vsync;
	}
	
	return RString();
}

void LowLevelWindow_Cocoa::ShutDownFullScreen()
{
	if( m_CurrentParams.windowed )
		return;
	ASSERT( m_CurrentDisplayMode );
	
	// Clear the front and back framebuffers before switching out of FullScreen mode.
	// (This is not strictly necessary, but avoids an untidy flash of garbage.)
	glClearColor( 0.0f, 0.0f, 0.0f, 0.0f );
	glClear( GL_COLOR_BUFFER_BIT );
	[m_Context flushBuffer];
	glClear( GL_COLOR_BUFFER_BIT );
	[m_Context flushBuffer];	
	
	[NSOpenGLContext clearCurrentContext];
	[m_Context clearDrawable];
	
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

int LowLevelWindow_Cocoa::ChangeDisplayMode( const VideoModeParams& p )
{	
	CFDictionaryRef mode = NULL;
	CFDictionaryRef newMode;
	CGDisplayErr err;
	
	if( !m_CurrentDisplayMode )
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
	
	if( !m_CurrentDisplayMode )
		m_CurrentDisplayMode = mode;
	m_CurrentParams.windowed = false;
	SetActualParamsFromMode( newMode );
	
	return 0;
}

void LowLevelWindow_Cocoa::SetActualParamsFromMode( CFDictionaryRef mode )
{
	SInt32 rate;
	
	if( CFNumberGetValue( (CFNumberRef)CFDictionaryGetValue(mode, CFSTR("RefreshRate")), kCFNumberSInt32Type, &rate) )
		m_CurrentParams.rate = rate;

	if( !m_CurrentParams.windowed )
	{
		long swap;

		m_CurrentParams.width = CGDisplayPixelsWide( kCGDirectMainDisplay );
		m_CurrentParams.height = CGDisplayPixelsHigh( kCGDirectMainDisplay );
		CGLGetParameter( CGLGetCurrentContext(), kCGLCPSwapInterval, &swap );
		m_CurrentParams.vsync = swap != 0;
	}
	else
	{
		NSSize size = [[m_Window contentView] frame].size;
		
		m_CurrentParams.width = int(size.width);
		m_CurrentParams.height = int(size.height);
	}

	m_CurrentParams.bpp = CGDisplayBitsPerPixel( kCGDirectMainDisplay );
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
	CGLFlushDrawable( CGLGetCurrentContext() );
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
	[m_Context update];
	DISPLAY->ResolutionChanged();
}

void LowLevelWindow_Cocoa::BeginConcurrentRendering()
{
	if( m_CurrentParams.windowed )
		[m_BGContext setView:[m_Window contentView]];
	else
		[m_BGContext setFullScreen];
	[m_BGContext makeCurrentContext];
}

void LowLevelWindow_Cocoa::EndConcurrentRendering()
{
	[NSOpenGLContext clearCurrentContext];
	[m_BGContext clearDrawable];
}
