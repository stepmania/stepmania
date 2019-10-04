#include "global.h"
#include "RageUtil.h"
#include "RageThreads.h"
#include "RageLog.h"
#include "CommandLineActions.h"

#import <Cocoa/Cocoa.h>
#include "ProductInfo.h"
#include "arch/ArchHooks/ArchHooks.h"
#include "StepMania.h"

@interface NSApplication (PrivateShutUpWarning)
- (void) setAppleMenu:(NSMenu *)menu;
@end

/* Replacement NSApplication class.
 * Replaces sendEvent so that key down events are only sent to the
 * menu bar. We handle all other input by reading HID events from the
 * keyboard directly. */
@interface SMApplication : NSApplication
- (void) fullscreen:(id)sender;
@end

@implementation SMApplication
- (void)fullscreen:(id)sender
{
	// don't use ArchHooks::SetToggleWindowed(), it makes the screen black
	[[self mainWindow] toggleFullScreen:nil];
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
@interface SMMain : NSObject
{
	int	m_iArgc;
	char	**m_pArgv;
	BOOL	m_bApplicationLaunched;
}
- (id) initWithArgc:(int)argc argv:(char **)argv;
- (void) startGame:(id)sender;
- (NSApplicationTerminateReply) applicationShouldTerminate:(NSApplication *)sender;
- (BOOL) application:(NSApplication *)app openFile:(NSString *)file;
- (void) application:(NSApplication *)app openFiles:(NSArray *)files;
- (void) setForInstall:(NSArray *)files;
@end

@implementation SMMain

- (id) initWithArgc:(int)argc argv:(char **)argv
{
	self = [super init];
	if( argc == 2 && !strncmp(argv[1], "-psn_", 5) )
		argc = 1;
	m_iArgc = argc;
	m_pArgv = new char*[argc];
	for( int i = 0; i < argc; ++i )
		m_pArgv[i] = argv[i];
	m_bApplicationLaunched = NO;
	return self;
}

- (void) startGame:(id)sender
{
	// Hand off to main application code.
	exit( sm_main(m_iArgc, m_pArgv) );
}

/* From here:
 * http://www.cocoadev.com/index.pl?HowToRegisterURLHandler */
- (void) getUrl:(NSAppleEventDescriptor *)event withReplyEvent:(NSAppleEventDescriptor *)replyEvent
{
	const char *url = [[[event paramDescriptorForKeyword:keyDirectObject] stringValue] UTF8String];

	LOG->Info("Parsing URL: %s", url);

	// I'm not sure this handles everything it needs to. - Colby
	CommandLineActions::CommandLineArgs args;
	args.argv.push_back(url);

	CommandLineActions::ToProcess.push_back(args);
}

// Called when the internal event loop has just started running.
- (void) applicationDidFinishLaunching:(NSNotification *)note
{
	m_bApplicationLaunched = YES;
	[NSThread detachNewThreadSelector:@selector(startGame:) toTarget:self withObject:nil];

	// Register ourselves as a URL handler.
	[
		[NSAppleEventManager sharedAppleEventManager] setEventHandler:self
		andSelector:@selector(getUrl:withReplyEvent:)
		forEventClass:kInternetEventClass
		andEventID:kAEGetURL
	];
}

- (BOOL) application:(NSApplication *)app openFile:(NSString *)file
{
	NSArray *files = [NSArray arrayWithObject:file];
	if( m_bApplicationLaunched )
		[NSTask launchedTaskWithLaunchPath:[NSString stringWithUTF8String:m_pArgv[0]] arguments:files];
	else
		[self setForInstall:files];
	return YES;
}

- (void) application:(NSApplication *)app openFiles:(NSArray *)files
{
	if( m_bApplicationLaunched )
		[NSTask launchedTaskWithLaunchPath:[NSString stringWithUTF8String:m_pArgv[0]] arguments:files];
	else
		[self setForInstall:files];
	[app replyToOpenOrPrint:NSApplicationDelegateReplySuccess];
}

- (void) setForInstall:(NSArray *)files
{
	char **temp = new char*[[files count] + m_iArgc];
	for( int i = 0; i < m_iArgc; ++i )
		temp[i] = m_pArgv[i];
	for( unsigned i = 0; i < [files count]; ++i, ++m_iArgc )
	{
		const char *p = [[files objectAtIndex:i] fileSystemRepresentation];
		temp[m_iArgc] = new char[strlen(p)+1];
		strcpy( temp[m_iArgc], p );
	}
	delete[] m_pArgv;
	m_pArgv = temp;
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

	[[mainMenu addItemWithTitle:[appMenu title] action:nil keyEquivalent:@""] setSubmenu:appMenu];
	[[mainMenu addItemWithTitle:[windowMenu title] action:nil keyEquivalent:@""] setSubmenu:windowMenu];

	[NSApp setMainMenu:mainMenu];
	[NSApp setAppleMenu:appMenu]; // This isn't the apple menu, but it doesn't work without this.
	[NSApp setWindowsMenu:windowMenu];
}

#undef main

int main( int argc, char **argv )
{
	RageThreadRegister guiThread( "GUI thread" );
	NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
	SMMain *sm;

	// Ensure the application object is initialised, this sets NSApp.
	[SMApplication sharedApplication];

	// Set up NSException handler.
	NSSetUncaughtExceptionHandler( HandleNSException );

	// Set up the menubar.
	SetupMenus();

	// Create SMMain and make it the app delegate.
	sm = [[SMMain alloc] initWithArgc:argc argv:argv];
	[NSApp setDelegate:static_cast<id<NSApplicationDelegate>>(sm)];

	[pool release];
	// Start the main event loop.
	[NSApp run];
	[sm release];
	return 0;
}

/*
 * (c) 2005-2009 Steve Checkoway
 * All rights reserved.
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, and/or sell copies of the Software, and to permit persons to
 * whom the Software is furnished to do so, provided that the above
 * copyright notice(s) and this permission notice appear in all copies of
 * the Software and that both the above copyright notice(s) and this
 * permission notice appear in supporting documentation.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT OF
 * THIRD PARTY RIGHTS. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR HOLDERS
 * INCLUDED IN THIS NOTICE BE LIABLE FOR ANY CLAIM, OR ANY SPECIAL INDIRECT
 * OR CONSEQUENTIAL DAMAGES, OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS
 * OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR
 * OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 */
