#import "global.h"
#import "LowLevelWindow_Cocoa.h"
#import <Cocoa/Cocoa.h>
#import <mach-o/dyld.h>

@interface SMView : NSOpenGLView
{	
}
- (id) initWithFrame:(NSRect)frameRect colorSize:(int)size;
@end

@implementation SMView
- (id) initWithFrame:(NSRect)frameRect colorSize:(int)size
{
    NSOpenGLPixelFormatAttribute attrs[] = {
		NSOpenGLPFANoRecovery, // so we can share with the full screen context
		NSOpenGLPFADoubleBuffer,
		NSOpenGLPFAAccelerated,
		NSOpenGLPFADepthSize, NSOpenGLPixelFormatAttribute(16),
		NSOpenGLPFAColorSize, NSOpenGLPixelFormatAttribute(size),
		NSOpenGLPixelFormatAttribute(0)
	};
	NSOpenGLPixelFormat *pixelFormat = [[NSOpenGLPixelFormat alloc] initWithAttributes:attrs];
	
	[super initWithFrame:frameRect pixelFormat:pixelFormat];
	return self;
}

// Put this here because I always forget about it and it might be needed later.
- (void) dealloc
{
	[super dealloc];
}

@end

LowLevelWindow_Cocoa::LowLevelWindow_Cocoa()
{
	NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
	NSRect rect = { 0, 0, 0, 0 };
	int mask = NSTitledWindowMask | NSClosableWindowMask | NSMiniaturizableWindowMask;
	mWindow = [[NSWindow alloc] initWithContentRect:rect
										  styleMask:mask
											backing:NSBackingStoreNonretained
											  defer:YES];
	[pool release];
}

LowLevelWindow_Cocoa::~LowLevelWindow_Cocoa()
{
	NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
	
	if( mWindow )
	{
		[mWindow orderOut:nil];
		[mWindow release];
	}
	[pool release];
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

CString LowLevelWindow_Cocoa::TryVideoMode( RageDisplay::VideoModeParams p, bool& newDeviceOut )
{
	return "XXX";
}

void LowLevelWindow_Cocoa::SwapBuffers()
{
	// XXX I'm not sure if this is needed yet.
	//NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
	
	[[NSOpenGLContext currentContext] flushBuffer];
	//[pool release];
}

void LowLevelWindow_Cocoa::Update()
{
}
