#include "global.h"
#include "RageUtil.h"
#include "RageThreads.h"

#import <Cocoa/Cocoa.h>
#include "ProductInfo.h"
#include "arch/ArchHooks/ArchHooks.h"

@interface NSApplication (PrivateShutUpWarning)
- (void) setAppleMenu:(NSMenu *)menu;
@end

@interface SMApplication : NSApplication
- (void) fullscreen:(id)sender;
@end

@interface SMMain : NSObject
{
	int	m_iArgc;
	char	**m_pArgv;
}
- (id) initWithArgc:(int)argc argv:(char **)argv;
- (void) startGame:(id)sender;
- (NSApplicationTerminateReply) applicationShouldTerminate:(NSApplication *)sender;
@end


@implementation SMApplication
- (void)fullscreen:(id)sender
{
	ArchHooks::SetToggleWindowed();
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
	m_iArgc = argc;
	m_pArgv = argv;
	return self;
}

- (void) startGame:(id)sender
{
	// Hand off to main application code.
	exit( SM_main(m_iArgc, m_pArgv) );
}

// Called when the internal event loop has just started running.
- (void) applicationDidFinishLaunching:(NSNotification *)note
{
	[NSThread detachNewThreadSelector:@selector(startGame:) toTarget:self withObject:nil];
}

- (NSApplicationTerminateReply) applicationShouldTerminate:(NSApplication *)sender
{
	ArchHooks::SetUserQuit();
	return NSTerminateCancel;
}
@end

static void HandleNSException( NSException *exception )
{
	FAIL_M( ssprintf("%s raised: %s", [[exception name] UTF8String], [[exception reason] UTF8String]) );
}

static NSMenuItem *MenuItem( NSString *title, SEL action, NSString *code )
{
	// Autorelease these because they'll be retained by the NSMenu.
	return [[[NSMenuItem alloc] initWithTitle:title action:action keyEquivalent:code] autorelease];
}

static void SetupMenus( void )
{
	// Get the localized strings from the file.
	NSString *sWindow =          NSLocalizedString( @"Window",                @"Menu title" );
	NSString *sHideOthers =      NSLocalizedString( @"Hide Others",           @"Menu item" );
	NSString *sAbout =           NSLocalizedString( @"About " PRODUCT_FAMILY, @"Menu item" );
	NSString *sHide =            NSLocalizedString( @"Hide " PRODUCT_FAMILY,  @"Menu item" );
	NSString *sShowAll =         NSLocalizedString( @"Show All",              @"Menu item" );
	NSString *sQuit =            NSLocalizedString( @"Quit " PRODUCT_FAMILY,  @"Menu item" );
	NSString *sMinimize =        NSLocalizedString( @"Minimize",              @"Menu item" );
	NSString *sEnterFullScreen = NSLocalizedString( @"Enter Full Screen",     @"Menu item" );
	
	NSMenu *mainMenu = [[[NSMenu alloc] initWithTitle:@""] autorelease];
	NSMenu *appMenu = [[[NSMenu alloc] initWithTitle:@PRODUCT_FAMILY] autorelease];
	NSMenu *windowMenu = [[[NSMenu alloc] initWithTitle:sWindow] autorelease];
	NSMenuItem *hideOthers = MenuItem( sHideOthers, @selector(hideOtherApplications:), @"h" );
	
	[hideOthers setKeyEquivalentModifierMask:NSAlternateKeyMask | NSCommandKeyMask ];
	
	[appMenu addItem:MenuItem( sAbout, @selector(orderFrontStandardAboutPanel:), @"" )];
	[appMenu addItem:[NSMenuItem separatorItem]];
	[appMenu addItem:MenuItem( sHide, @selector(hide:), @"h" )];
	[appMenu addItem:hideOthers];
	[appMenu addItem:MenuItem( sShowAll, @selector(unhideAllApplications:), @"" )];
	[appMenu addItem:[NSMenuItem separatorItem]];
	[appMenu addItem:MenuItem( sQuit, @selector(terminate:), @"q" )];
	
	[windowMenu addItem:MenuItem( sMinimize, @selector(performMiniaturize:), @"m" )];
	[windowMenu addItem:[NSMenuItem separatorItem]];
	
	// Add a Full Screen item.
	NSMenuItem *item = MenuItem( sEnterFullScreen, @selector(fullscreen:), @"\n" );
	
	[item setKeyEquivalentModifierMask:NSAlternateKeyMask]; // opt-enter
	[windowMenu addItem:item];
	
	[[mainMenu addItemWithTitle:[appMenu title] action:NULL keyEquivalent:@""] setSubmenu:appMenu];
	[[mainMenu addItemWithTitle:[windowMenu title] action:NULL keyEquivalent:@""] setSubmenu:windowMenu];
	
	[NSApp setMainMenu:mainMenu];
	[NSApp setAppleMenu:appMenu]; // This isn't the apple menu, but it doesn't work without this.
	[NSApp setWindowsMenu:windowMenu];
}

#undef main

int main( int argc, char **argv )
{
	RageThreadRegister guiThread( "GUI thread" );
	
	[SMApplication poseAsClass:[NSApplication class]];
	
	NSAutoreleasePool	*pool = [[NSAutoreleasePool alloc] init];
	SMMain			*sm;
	
	// Ensure the application object is initialised, this sets NSApp.
	[SMApplication sharedApplication];
	
	// Set up NSException handler.
	NSSetUncaughtExceptionHandler( HandleNSException );
	
	// Set up the menubar.
	SetupMenus();
	
	// Create SDLMain and make it the app delegate.
	sm = [[SMMain alloc] initWithArgc:argc argv:argv];
	[NSApp setDelegate:sm];
	
	[pool release];
	// Start the main event loop.
	[NSApp run];
	[sm release];
	return 0;
}
