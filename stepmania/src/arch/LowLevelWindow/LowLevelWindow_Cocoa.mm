#import "global.h"
#import "LowLevelWindow_Cocoa.h"
#import "DisplayResolutions.h"
#import "RageLog.h"
#import "RageUtil.h"

#import <Cocoa/Cocoa.h>
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

static NSOpenGLPixelFormat *CreatePixelFormat( int bbp, bool windowed )
{
	NSOpenGLPixelFormatAttribute attrs[] = {
		NSOpenGLPFANoRecovery, // so we can share with the full screen context
		NSOpenGLPFADoubleBuffer,
		NSOpenGLPFAAccelerated,
		NSOpenGLPFADepthSize, NSOpenGLPixelFormatAttribute(16),
		NSOpenGLPFAColorSize, NSOpenGLPixelFormatAttribute(bbp == 16 ? 16 : 24),
		windowed ? NSOpenGLPixelFormatAttribute(0) : NSOpenGLPFAFullScreen,
		NSOpenGLPixelFormatAttribute(0)
	};
	
	return [[[NSOpenGLPixelFormat alloc] initWithAttributes:attrs] autorelease];
}


LowLevelWindow_Cocoa::LowLevelWindow_Cocoa() : mView(nil), mFullScreenContext(nil)
{
	POOL;
	NSRect rect = { {0, 0}, {0, 0} };
	mWindow = [[NSWindow alloc] initWithContentRect:rect
										  styleMask:g_iStyleMask
											backing:NSBackingStoreNonretained
											  defer:YES];
	[mWindow setExcludedFromWindowsMenu:YES];
	[mWindow useOptimizedDrawing:YES];
}

LowLevelWindow_Cocoa::~LowLevelWindow_Cocoa()
{
	POOL;
	if( mWindow )
	{
		[mWindow orderOut:nil];
		[mWindow release];
	}
	if( mFullScreenContext )
	{
		if( !mCurrentParams.windowed )
		{
			[mFullScreenContext clearDrawable]; // Exit full screen.
			CGReleaseAllDisplays();
		}
		[mFullScreenContext release];
	}
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
	POOL;
	
	newDeviceOut = false;
	
	NSRect contentRect = { { 0, 0 }, { p.width, p.height } };
	const bool changedWindowed = p.windowed != mCurrentParams.windowed;
	const bool changedView = p.bpp != mCurrentParams.bpp || !mView;
	
	LOG->Trace( "changedWindowed = %d; changedView = %d;", int(changedWindowed), int(changedView) );
	
	if( changedWindowed )
	{
		if( p.windowed )
		{
			// No need to reset the size if it changes here.
			[mWindow setContentSize:contentRect.size];
			[mWindow setTitle:[NSString stringWithUTF8String:p.sWindowTitle.c_str()]];
			
			if( changedView )
			{
				// Create view.
				NSOpenGLPixelFormat *pixelFormat = CreatePixelFormat( p.bpp, true );
				
				if( pixelFormat == nil )
					return "Failed to set Pixel Format";
				id nextView = [[NSOpenGLView alloc] initWithFrame:contentRect pixelFormat:pixelFormat];
				if( nextView == nil )
					return "Failed to create OGL context.";
				mView = nextView;
				[mWindow setContentView:mView]; // This retains the view and releases the old one.
				[mView release];
				newDeviceOut = true;
			}
			// Shut down full screen. Nothing else should fail.
			if( mFullScreenContext )
			{
				[mFullScreenContext clearDrawable];
				CGDisplayErr err = CGReleaseAllDisplays();
				
				ASSERT( err = kCGErrorSuccess );
				if( changedView )
				{
					[mFullScreenContext release];
					mFullScreenContext = nil;
				}
				newDeviceOut = !mSharingContexts;
			}
			// Set the current context
			[[mView openGLContext] makeCurrentContext];
			[mWindow center];
			[mWindow performSelectorOnMainThread:@selector(makeKeyAndOrderFront:)
									  withObject:nil waitUntilDone:YES];		
			return CString();
		}
		// Okay, we're going full screen here.
		// If the bpp has changed (or nothing has been created) we need to recreate everything.
		id nextView = changedView ? nil : mView;
		if( changedView )
		{
			NSOpenGLPixelFormat *pixelFormat = CreatePixelFormat( p.bpp, true );
			
			if( pixelFormat == nil )
				return "Failed to set pixel format.";
			nextView = [[NSOpenGLView alloc] initWithFrame:contentRect pixelFormat:pixelFormat];
			if( nextView == nil )
				return "Failed to create OGL context.";
			newDeviceOut = true;
			// We need to recreate the full screen context.
			[mFullScreenContext release];
			mFullScreenContext = nil;
			
			// Autorelease nextView in case we fail later on.
			[nextView autorelease];
		}
		id newContext = mFullScreenContext;
		
		if( newContext == nil )
		{
			NSOpenGLPixelFormat *pixelFormat = CreatePixelFormat( p.bpp, false );
			
			if( pixelFormat == nil )
				return "Failed to set pixel format.";
				
			newContext = [[NSOpenGLContext alloc] initWithFormat:pixelFormat
													shareContext:[nextView openGLContext]];
			if( !(mSharingContexts = newContext != nil) )
			{
				LOG->Warn( "Failed to share openGL contexts." );
				newContext = [[NSOpenGLContext alloc] initWithFormat:pixelFormat shareContext:nil];
				if( !mFullScreenContext )
					return "Failed to create full screen openGL context.";
				newDeviceOut = true;
			}
			// Autorelease to aid cleanup in case of failure
			[newContext autorelease];
		}
		// Now we need to actually go full screen.
		CGDisplayErr err = CGCaptureAllDisplays();
		
		if( err != kCGErrorSuccess )
			return ssprintf( "Failed to capture all displays: %d.", int(err) );
		// We have the display, change the screen size if possible.
		CFDictionaryRef params;
		
		if( p.rate == REFRESH_DEFAULT )
			params = CGDisplayBestModeForParameters( kCGDirectMainDisplay, p.bpp, p.width, p.height, NULL );
		else
			params = CGDisplayBestModeForParametersAndRefreshRate( kCGDirectMainDisplay, p.bpp,
																   p.width, p.height, p.rate, NULL );
		err = CGDisplaySwitchToMode( kCGDirectMainDisplay, params );
		// XXX the docs don't show releasing params...

		if( err != kCGErrorSuccess )
		{
			CGDisplayErr e = CGReleaseAllDisplays();
			
			ASSERT( e == kCGErrorSuccess );
			return ssprintf( "Failed to switch mode: %d.", int(err) );
		}
		
		// We've succeeded!
		mView = nextView;
		[mWindow setContentView:mView];
		if( !mFullScreenContext )
			mFullScreenContext = [newContext retain]; // We autoreleased above so retain here.
		[mFullScreenContext setFullScreen];
		[mFullScreenContext makeCurrentContext];
		return CString();
	}
	// We're not changing windowed here
	if( p.windowed )
	{
		if( changedView )
		{
			// Create view.
			NSOpenGLPixelFormat *pixelFormat = CreatePixelFormat( p.bpp, true );
			
			if( pixelFormat == nil )
				return "Failed to set Pixel Format";
			id nextView = [[NSOpenGLView alloc] initWithFrame:contentRect pixelFormat:pixelFormat];
			if( nextView == nil )
				return "Failed to create OGL context.";
			mView = [nextView autorelease];
			newDeviceOut = true;
			[mFullScreenContext release];
			mFullScreenContext = nil;
		}
		[mWindow setContentSize:contentRect.size];
		[mWindow setTitle:[NSString stringWithUTF8String:p.sWindowTitle.c_str()]];
		[mWindow setContentView:mView]; // This retains the view and releases the old one.
		[[mView openGLContext] makeCurrentContext];
		[mWindow center];
		[mWindow performSelectorOnMainThread:@selector(makeKeyAndOrderFront:)
								  withObject:nil waitUntilDone:YES];		
		return CString();
	}
	// Just full screen left here.
	// If the bpp has changed (or nothing has been created) we need to recreate everything.
	id nextView = changedView ? nil : mView;
	if( changedView )
	{
		NSOpenGLPixelFormat *pixelFormat = CreatePixelFormat( p.bpp, true );
		
		if( pixelFormat == nil )
			return "Failed to set pixel format.";
		nextView = [[NSOpenGLView alloc] initWithFrame:contentRect pixelFormat:pixelFormat];
		if( nextView == nil )
			return "Failed to create OGL context.";
		newDeviceOut = true;
		// We need to recreate the full screen context.
		[mFullScreenContext release];
		mFullScreenContext = nil;
		
		// Autorelease nextView in case we fail later on.
		[nextView autorelease];
	}
	id newContext = mFullScreenContext;
	
	if( newContext == nil )
	{
		NSOpenGLPixelFormat *pixelFormat = CreatePixelFormat( p.bpp, false );
		
		if( pixelFormat == nil )
			return "Failed to set pixel format.";
		
		newContext = [[NSOpenGLContext alloc] initWithFormat:pixelFormat
												shareContext:[nextView openGLContext]];
		if( !(mSharingContexts = newContext != nil) )
		{
			LOG->Warn( "Failed to share openGL contexts." );
			newContext = [[NSOpenGLContext alloc] initWithFormat:pixelFormat shareContext:nil];
			if( !mFullScreenContext )
				return "Failed to create full screen openGL context.";
			newDeviceOut = true;
		}
		// Autorelease to aid cleanup in case of failure
		[newContext autorelease];
	}
	CFDictionaryRef params;
	
	if( p.rate == REFRESH_DEFAULT )
		params = CGDisplayBestModeForParameters( kCGDirectMainDisplay, p.bpp, p.width, p.height, NULL );
	else
		params = CGDisplayBestModeForParametersAndRefreshRate( kCGDirectMainDisplay, p.bpp,
															   p.width, p.height, p.rate, NULL );
	CGDisplayErr err = CGDisplaySwitchToMode( kCGDirectMainDisplay, params );
	// XXX the docs don't show releasing params...
	
	if( err != kCGErrorSuccess )
		return ssprintf( "Failed to switch mode: %d.", int(err) );
	
	// We've succeeded!
	mView = nextView;
	[mWindow setContentSize:contentRect.size];
	[mWindow setContentView:mView];
	if( !mFullScreenContext )
	{
		mFullScreenContext = [newContext retain]; // We autoreleased above so retain here.
		[mFullScreenContext setFullScreen];
		[mFullScreenContext makeCurrentContext];
	}
	else
	{
		[mFullScreenContext update];
	}
	return CString();
}
#if 0

	if( changedBpp )
	{
		mView = nil; // Create a new one, setContentView: will release it
		if( mFullScreenContext )
		{
			[mFullScreenContext clearDrawable];
			[mFullScreenContext release];
			mFullScreenContext = nil;
			CGReleaseAllDisplays();
		}
	}
	
	
	if( p.windowed )
	{
		if( !mCurrentParams.windowed )
		{
			[mFullScreenContext clearDrawable];
			CGReleaseAllDisplays();
		}
		[[mView openGLContext] makeCurrentContext];
		[mWindow setTitle:[NSString stringWithUTF8String:p.sWindowTitle.c_str()]];
		[mWindow center];
		[mWindow performSelectorOnMainThread:@selector(makeKeyAndOrderFront:)
								  withObject:nil waitUntilDone:YES];		
	}
	else
	{
		if( !mFullScreenContext )
		{
			NSOpenGLPixelFormat *pixelFormat = CreatePixelFormat( p.bpp, false );
			
			ASSERT( pixelFormat );
			mFullScreenContext = [[NSOpenGLContext alloc] initWithFormat:pixelFormat
															shareContext:[mView openGLContext]];
			if( !mFullScreenContext )
			{
				LOG->Warn( "Failed to share openGL contexts." );
				mFullScreenContext = [[NSOpenGLContext alloc] initWithFormat:pixelFormat shareContext:nil];
				ASSERT( mFullScreenContext );
				newDeviceOut = true;
			}
		}
		else if( changedSize )
		{
			[mFullScreenContext update];
		}
		if( mCurrentParams.windowed )
		{
			CGCaptureAllDisplays(); // Do not notify other apps of any changes.
			[mFullScreenContext setFullScreen];
			[mFullScreenContext makeCurrentContext];
		}
	}
	
	mCurrentParams = p;
	return "";
}
#endif

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
	//POOL;
	[[NSOpenGLContext currentContext] flushBuffer];
}
