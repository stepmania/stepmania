#include "global.h"
#include "RageUtil.h"

#import <Cocoa/Cocoa.h>
#include "ProductInfo.h"
#include "GameLoop.h"

// XXX remove this
extern "C"
{
	extern int SDL_main(int, char **);
}

@interface SMApplication : NSApplication
@end

@interface SMMain : NSObject
{
	int mArgc;
	char **mArgv;
}
- (id) initWithArgc:(int)argc argv:(char **)argv;
- (void) startGame:(id)sender;
@end


@implementation SMApplication
// Invoked from the Quit menu item.
- (void)terminate:(id)sender
{
    ExitGame();
}

- (void)sendEvent:(NSEvent *)event
{
	if( [event type] == NSKeyDown )
		[[self mainMenu] performKeyEquivalent:event];
	else
		[super sendEvent:event];
}
@end

// The main class of the application, the application's delegate.
@implementation SMMain

- (id) initWithArgc:(int)argc argv:(char **)argv
{
	[super init];
	mArgc = argc;
	mArgv = argv;
	return self;
}

- (void) startGame:(id)sender
{
	NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
	// Hand off to main application code.
    exit( SDL_main(mArgc, mArgv) );
	[pool release]; // not really needed, but shuts gcc up.
}

// Called when the internal event loop has just started running.
- (void) applicationDidFinishLaunching: (NSNotification *) note
{
	[NSThread detachNewThreadSelector:@selector(startGame:) toTarget:self withObject:nil];
}
@end

static void HandleNSException( NSException *exception )
{
	FAIL_M( ssprintf("%s raised: %s", [[exception name] UTF8String], [[exception reason] UTF8String]) );
}

static NSMenuItem *MenuItem( const char *title, SEL action, NSString *code )
{
	// Autorelease these because they'll be retained by the NSMenu.
	return [[[NSMenuItem alloc] initWithTitle:[NSString stringWithUTF8String:title]
									   action:action keyEquivalent:code] autorelease];
}

static void setupMenus( void )
{
	NSMenu *mainMenu = [[[NSMenu alloc] initWithTitle:@""] autorelease];
	NSMenu *appMenu = [[[NSMenu alloc] initWithTitle:@PRODUCT_NAME] autorelease];
	NSMenu *windowMenu = [[[NSMenu alloc] initWithTitle:@"Window"] autorelease];
	NSMenuItem *hideOthers = MenuItem( "Hide Others", @selector(hideOtherApplications:), @"h" );
	
	[hideOthers setKeyEquivalentModifierMask:NSAlternateKeyMask | NSCommandKeyMask ];
	
	[appMenu addItem:MenuItem( "About " PRODUCT_NAME, @selector(orderFrontStandardAboutPanel:), @"" )];
	[appMenu addItem:[NSMenuItem separatorItem]];
	[appMenu addItem:MenuItem( "Hide " PRODUCT_NAME, @selector(hide:), @"h" )];
	[appMenu addItem:hideOthers];
	[appMenu addItem:MenuItem( "Show All", @selector(unhideAllApplications:), @"" )];
	[appMenu addItem:[NSMenuItem separatorItem]];
	[appMenu addItem:MenuItem( "Quit " PRODUCT_NAME, @selector(terminate:), @"q" )];
	
	[windowMenu addItem:MenuItem( "Minimize", @selector(performMiniaturize:), @"m" )];
	[windowMenu addItem:[NSMenuItem separatorItem]];
	[windowMenu addItem:MenuItem( "Full Screen", @selector(fullscreen:), @"" )];

    [[mainMenu addItemWithTitle:[appMenu title] action:NULL keyEquivalent:@""] setSubmenu:appMenu];
    [[mainMenu addItemWithTitle:[windowMenu title] action:NULL keyEquivalent:@""] setSubmenu:windowMenu];
	
	[NSApp setMainMenu:mainMenu];
	[NSApp setAppleMenu:appMenu]; // This isn't the apple menu, but it doesn't work without this.
    [NSApp setWindowsMenu:windowMenu];
}

int main( int argc, char **argv )
{
    [SMApplication poseAsClass:[NSApplication class]];
	
    NSAutoreleasePool	*pool = [[NSAutoreleasePool alloc] init];
    SMMain				*sm;
	
    // Ensure the application object is initialised, this sets NSApp.
    [SMApplication sharedApplication];
	
	// Set up NSException handler.
	NSSetUncaughtExceptionHandler( HandleNSException );
	
    // Set up the menubar.
    setupMenus();
    
    // Create SDLMain and make it the app delegate.
    sm = [[SMMain alloc] initWithArgc:argc argv:argv];
    [NSApp setDelegate:sm];
    
    [pool release];
    // Start the main event loop.
    [NSApp run];
	[sm release];
    return 0;
}
