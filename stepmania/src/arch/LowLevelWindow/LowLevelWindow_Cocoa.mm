#import "global.h"
#import "LowLevelWindow_Cocoa.h"
#import "DisplayResolutions.h"
#import "RageLog.h"
#import "RageUtil.h"
#import "arch/ArchHooks/ArchHooks.h"
#import "archutils/Darwin/SMMainThread.h"

#import <Cocoa/Cocoa.h>
#import <OpenGl/OpenGl.h>
#import <mach-o/dyld.h>

static const int g_iStyleMask = NSTitledWindowMask | NSClosableWindowMask | NSMiniaturizableWindowMask;

// Simple helper class
class AutoreleasePool
{
	NSAutoreleasePool *mPool;
	
public:
	AutoreleasePool() { mPool = [[NSAutoreleasePool alloc] init]; }
	~AutoreleasePool() { [mPool release]; }
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
@end

LowLevelWindow_Cocoa::LowLevelWindow_Cocoa() : mView(nil), mFullScreenContext(nil), mCurrentDisplayMode(NULL)
{
	POOL;
	NSRect rect = { {0, 0}, {0, 0} };
	SMMainThread *mt = [[SMMainThread alloc] init];
	
	mWindow = [[NSWindow alloc] initWithContentRect:rect
					      styleMask:g_iStyleMask
						backing:NSBackingStoreNonretained
						  defer:YES];
	ASSERT( mWindow != nil );
	
	ADD_ACTIONb( mt, mWindow, setExcludedFromWindowsMenu:, YES );
	ADD_ACTIONb( mt, mWindow, useOptimizedDrawing:, YES );
	ADD_ACTIONb( mt, mWindow, setReleasedWhenClosed:, NO );
	// setDelegate: does not retain the delegate; however, we don't (auto)release it.
	ADD_ACTION1( mt, mWindow, setDelegate:, [[SMWindowDelegate alloc] init] );
	
	[mt performOnMainThread];
	[mt release];
	mCurrentParams.windowed = true; // We are essentially windowed to begin with.
}

LowLevelWindow_Cocoa::~LowLevelWindow_Cocoa()
{
	POOL;
	ShutDownFullScreen();
	
	SMMainThread *mt = [[SMMainThread alloc] init];
	
	// We need to release the window's delegate now.
	id delegate = [mWindow delegate];
	
	ADD_ACTION1( mt, mWindow, setDelegate:, nil );
	ADD_ACTION1( mt, mWindow, orderOut:, nil );
	ADD_ACTION0( mt, mWindow, release );
	
	[mt performOnMainThread];
	[delegate release];
	[mt release];
	[mFullScreenContext release];
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
	mCurrentParams.bSmoothLines = p.bSmoothLines;
	mCurrentParams.bTrilinearFiltering = p.bTrilinearFiltering;
	mCurrentParams.bAnisotropicFiltering = p.bAnisotropicFiltering;
	mCurrentParams.interlaced = p.interlaced;
	mCurrentParams.PAL = p.PAL;
	
#define X(x) p.x == mCurrentParams.x
	if( X(windowed) && X(bpp) && X(width) && X(height) && X(rate) && X(vsync) )
		return 0;
#undef X
	
	POOL;
	newDeviceOut = false;
	
	NSRect contentRect = { { 0, 0 }, { p.width, p.height } };
	SMMainThread *mt = [[[SMMainThread alloc] init] autorelease];
	
	// Change the window and the title
	ADD_ACTIONn( mt, mWindow, setContentSize:, 1, &contentRect.size );
	ADD_ACTION1( mt, mWindow, setTitle:, [NSString stringWithUTF8String:p.sWindowTitle.c_str()] );
		
	mCurrentParams.width = p.width;
	mCurrentParams.height = p.height;
	
	ASSERT( p.bpp == 16 || p.bpp == 32 );
	
	if( p.bpp != mCurrentParams.bpp || !mView )
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
		id nextView;
		
		pixelFormat = [[NSOpenGLPixelFormat alloc] initWithAttributes:attrs];
		
		if( pixelFormat == nil )
		{
			[mt performOnMainThread];
			return "Failed to set the windowed pixel format.";
		}
		nextView = [[[NSOpenGLView alloc] initWithFrame:contentRect pixelFormat:pixelFormat] autorelease];
		[pixelFormat release];
		if( nextView == nil )
		{
			[mt performOnMainThread];
			return "Failed to create windowed OGL context.";
		}
		SetOGLParameters( [mView openGLContext] );
		newDeviceOut = true;
		mView = nextView;
		ADD_ACTION1( mt, mWindow, setContentView:, mView );

		// We need to recreate the full screen context as well.
		[mFullScreenContext clearDrawable];
		[mFullScreenContext release];
		mFullScreenContext = nil;
		mCurrentParams.bpp = p.bpp;
	}
	if( p.windowed )
	{
		id context = [mView openGLContext];
		
		ShutDownFullScreen();
		
		ADD_ACTION0( mt, mWindow, center );
		ADD_ACTION1( mt, mWindow, makeKeyAndOrderFront:, nil );
		
		[mt performOnMainThread];
		[context update];
		[context makeCurrentContext];
		mCurrentParams.windowed = true;
		SetActualParamsFromMode( CGDisplayCurrentMode(kCGDirectMainDisplay) );
		
		newDeviceOut = newDeviceOut || !mSharingContexts;
		return CString();
	}
	[mt performOnMainThread];
	int result = ChangeDisplayMode( p );
	
	if( result )
	{
		return ssprintf( "Failed to switch to full screen:%d x %d @ %d. Error %d.",
				 p.width, p.height, p.rate, result );
	}
	if( mFullScreenContext == nil )
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
		
		mFullScreenContext = [[NSOpenGLContext alloc] initWithFormat:pixelFormat
							shareContext:[mView openGLContext]];
		if( !(mSharingContexts = mFullScreenContext != nil) )
		{
			LOG->Warn( "Failed to share openGL contexts." );
			mFullScreenContext = [[NSOpenGLContext alloc] initWithFormat:pixelFormat shareContext:nil];
			if( !mFullScreenContext )
			{
				ShutDownFullScreen(); // Same here.
				return "Failed to create full screen openGL context.";
			}
		}
		SetOGLParameters( mFullScreenContext );
	}
	long swap = p.vsync ? 1 : 0;
	
	[mFullScreenContext setValues:&swap forParameter:NSOpenGLCPSwapInterval];
	[mFullScreenContext setFullScreen];
	[mFullScreenContext update];
	[mFullScreenContext makeCurrentContext];
	mCurrentParams.vsync = p.vsync;
	
	newDeviceOut = newDeviceOut || !mSharingContexts;
	return CString();
}

void LowLevelWindow_Cocoa::ShutDownFullScreen()
{
	if( mCurrentParams.windowed )
		return;
	ASSERT( mCurrentDisplayMode );
	[mFullScreenContext clearDrawable];
	
	CGDisplayErr err = CGDisplaySwitchToMode( kCGDirectMainDisplay, mCurrentDisplayMode );
	
	ASSERT( err == kCGErrorSuccess );
	CGDisplayShowCursor( kCGDirectMainDisplay );
	err = CGReleaseAllDisplays();
	ASSERT( err == kCGErrorSuccess );
	SetActualParamsFromMode( mCurrentDisplayMode );
	// We don't own this so we cannot release it.
	mCurrentDisplayMode = NULL;
	mCurrentParams.windowed = true;
}

int LowLevelWindow_Cocoa::ChangeDisplayMode( const VideoModeParams& p )
{	
	CFDictionaryRef mode = NULL;
	CFDictionaryRef newMode;
	CGDisplayErr err;
	
	if( mCurrentParams.windowed )
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
	
	if( mCurrentParams.windowed )
	{
		mCurrentDisplayMode = mode;
		mCurrentParams.windowed = false;
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
	if( mCurrentParams.windowed )
	{
		mCurrentParams.vsync = false;
	}
	else
	{
		long swap;
		
		mCurrentParams.width = width;
		mCurrentParams.height = height;
		CGLGetParameter( CGLGetCurrentContext(), kCGLCPSwapInterval, &swap );
		mCurrentParams.vsync = swap != 0;
	}
	mCurrentParams.bpp = bpp;
	mCurrentParams.rate = rate;
	// XXX should this be the actual DAR of the display or of the window, if windowed?
	mCurrentParams.fDisplayAspectRatio = float(mCurrentParams.width)/mCurrentParams.height;
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
	[[NSOpenGLContext currentContext] flushBuffer];
}
