#import "global.h"
#import "LowLevelWindow_Cocoa.h"
#import "DisplayResolutions.h"
#import "RageLog.h"
#import "RageUtil.h"
#import "arch/ArchHooks/ArchHooks.h"

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

static NSOpenGLPixelFormat *CreatePixelFormat( int bbp, bool windowed )
{
	NSOpenGLPixelFormatAttribute attrs[] = {
		NSOpenGLPFANoRecovery, // so we can share with the full screen context
		NSOpenGLPFADoubleBuffer,
		NSOpenGLPFAAccelerated,
		NSOpenGLPFADepthSize, NSOpenGLPixelFormatAttribute(16),
		NSOpenGLPFAColorSize, NSOpenGLPixelFormatAttribute(bbp == 16 ? 16 : 24),
		NSOpenGLPixelFormatAttribute(0)
	};
	
	return [[[NSOpenGLPixelFormat alloc] initWithAttributes:attrs] autorelease];
}


LowLevelWindow_Cocoa::LowLevelWindow_Cocoa() : mView(nil), mFullScreenContext(nil), mCurrentDisplayMode(NULL)
{
	POOL;
	NSRect rect = { {0, 0}, {0, 0} };
	mWindow = [[NSWindow alloc] initWithContentRect:rect
					      styleMask:g_iStyleMask
						backing:NSBackingStoreNonretained
						  defer:YES];
	ASSERT( mWindow != nil );
	[mWindow setExcludedFromWindowsMenu:YES];
	[mWindow useOptimizedDrawing:YES];
	[mWindow setReleasedWhenClosed:NO];
	// setDelegate: does not retain the delegate; however, we didn't (auto)release it.
	[mWindow setDelegate:[[SMWindowDelegate alloc] init]];
	mCurrentParams.windowed = true; // We are essentially windowed to begin with.
}

LowLevelWindow_Cocoa::~LowLevelWindow_Cocoa()
{
	POOL;
	ShutDownFullScreen();
	// We need to release the window's delegate now.
	// Autorelease to prevent a race condition.
	[[mWindow delegate] autorelease];
	[mWindow setDelegate:nil];
	[mWindow orderOut:nil];
	[mWindow release];
	[mFullScreenContext release];
}

void *LowLevelWindow_Cocoa::GetProcAddress( CString s )
{
	// http://developer.apple.com/qa/qa2001/qa1188.html
	const CString& symbolName( '_' + s );
	NSSymbol symbol = NULL;
	
	if( NSIsSymbolNameDefined(symbolName) )
		symbol = NSLookupAndBindSymbol( symbolName );
	return symbol ? NSAddressOfSymbol( symbol ) : NULL;
}

CString LowLevelWindow_Cocoa::TryVideoMode( const VideoModeParams& p, bool& newDeviceOut )
{
#define X(x) p.x == mCurrentParams.x
	if( X(windowed) && X(bpp) && X(width) && X(height) && X(rate) )
		return 0;
#undef X
	
	POOL;
	newDeviceOut = false;
	
	NSRect contentRect = { { 0, 0 }, { p.width, p.height } };
	
	// Change the window and the title
	[mWindow setContentSize:contentRect.size];
	[mWindow setTitle:[NSString stringWithUTF8String:p.sWindowTitle.c_str()]];
	
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
			return "Failed to set the windowed pixel format.";
		nextView = [[NSOpenGLView alloc] initWithFrame:contentRect pixelFormat:pixelFormat];
		[pixelFormat release];
		if( nextView == nil )
			return "Failed to create windowed OGL context.";
		newDeviceOut = true;
		mView = nextView;
		[mWindow setContentView:mView];

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
		[mWindow center];
		[mWindow makeKeyAndOrderFront:nil];
		
		[context update];
		[context makeCurrentContext];
		mCurrentParams.windowed = true;
		SetActualParamsFromMode( CGDisplayCurrentMode(kCGDirectMainDisplay) );
		
		newDeviceOut = newDeviceOut || !mSharingContexts;
		return CString();
	}
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
	}
	[mFullScreenContext setFullScreen];
	[mFullScreenContext update];
	[mFullScreenContext makeCurrentContext];
	// Copy the rest of the state
	mCurrentParams = p;
	
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
	// XXX I'm not sure if this is needed yet.
	POOL;
	[[NSOpenGLContext currentContext] flushBuffer];
}
