/*   SDLMain.m - main entry point for our Cocoa-ized SDL app
       Initial Version: Darrell Walisser <dwaliss1@purdue.edu>
       Non-NIB-Code & other changes: Max Horn <max@quendi.de>

    Feel free to customize this file to suit your needs
*/

#import <Cocoa/Cocoa.h>
#import <SDL.h>
#import <sys/param.h> /* for MAXPATHLEN */
#import <unistd.h>

static int    gArgc;
static char  **gArgv;

@interface SDLApplication : NSApplication
@end

@interface SDLMain : NSObject {}
- (void) startGame:(id)obj;
@end


@implementation SDLApplication
/* Invoked from the Quit menu item */
- (void)terminate:(id)sender
{
    /* Post a SDL_QUIT event */
    SDL_Event event;
    event.type = SDL_QUIT;
    SDL_PushEvent(&event);
}

- (void)sendEvent:(NSEvent *)event
{
	if( [event type] != NSKeyDown )
		[super sendEvent:event];
}

@end

/* The main class of the application, the application's delegate */
@implementation SDLMain

- (void) startGame:(id)obj
{
	NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
	/* Hand off to main application code */
    exit( SDL_main (gArgc, gArgv) );
	[pool release]; // not really needed, but shuts gcc up.
}

/* Called when the internal event loop has just started running */
- (void) applicationDidFinishLaunching: (NSNotification *) note
{
#if 1
	[NSThread detachNewThreadSelector:@selector(startGame:) toTarget:self withObject:nil];
#else
	[self startGame:nil];
#endif
}
@end


static void setupMenus( void )
{
    NSMenu		*windowMenu;
    NSMenuItem	*windowMenuItem;
    NSMenuItem	*menuItem;
	NSMenu *mainMenu = [NSApp mainMenu];
	
	printf("%d\n", [mainMenu numberOfItems]);
	
    windowMenu = [[NSMenu alloc] initWithTitle:@"Window"];
    
    /* "Minimize" item */
    menuItem = [[NSMenuItem alloc] initWithTitle:@"Minimize" action:@selector(performMiniaturize:)
								   keyEquivalent:@"m"];
    [windowMenu addItem:menuItem];
    [menuItem release];
    
    /* Put menu into the menubar */
    windowMenuItem = [[NSMenuItem alloc] initWithTitle:@"Window" action:nil keyEquivalent:@""];
    [windowMenuItem setSubmenu:windowMenu];
    [mainMenu addItem:windowMenuItem];
    
    /* Tell the application object that this is now the window menu */
    [NSApp setWindowsMenu:windowMenu];
	
    /* Finally give up our references to the objects */
    [windowMenu release];
    [windowMenuItem release];
}

/* Replacement for NSApplicationMain */
static void CustomApplicationMain( int argc, char **argv )
{
    NSAutoreleasePool	*pool = [[NSAutoreleasePool alloc] init];
    SDLMain				*sdlMain;
	
    /* Ensure the application object is initialised */
    [SDLApplication sharedApplication];
	
	//[NSApp setMainMenu:[[NSMenu alloc] initWithTitle:@""]];
    /* Set up the menubar */
    setupMenus();
    
    /* Create SDLMain and make it the app delegate */
    sdlMain = [[SDLMain alloc] init];
    [NSApp setDelegate:sdlMain];
    
    /* Start the main event loop */
    [NSApp run];
    
    [sdlMain release];
    [pool release];
}

#ifdef main
#  undef main
#endif


/* Main entry point to executable - should *not* be SDL_main! */
int main( int argc, char **argv )
{

    /* Copy the arguments into a global variable */
    int i;
    
    /* This is passed if we are launched by double-clicking */
    if ( argc >= 2 && strncmp (argv[1], "-psn", 4) == 0 )
        gArgc = 1;
	else
        gArgc = argc;
    gArgv = (char**) malloc (sizeof(*gArgv) * (gArgc+1));
    assert (gArgv != NULL);
    for (i = 0; i < gArgc; i++)
        gArgv[i] = argv[i];
    gArgv[i] = NULL;

    [SDLApplication poseAsClass:[NSApplication class]];
    CustomApplicationMain( argc, argv );
    return 0;
}
