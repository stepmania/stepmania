#import <Cocoa/Cocoa.h>
#import <assert.h>

void SM_ShowCursor( bool b )
{
	if( b )
		[NSCursor unhide];
	else
		[NSCursor hide];
}

void SetupWindow()
{
	NSWindow *window = [[NSApp windows] objectAtIndex:0];
	
	// There's only one window ever so don't display in the menu
	[window setExcludedFromWindowsMenu:YES];
	
	// Maybe make drawing faster (I would hope that SDL does this already)
	[window useOptimizedDrawing:YES];
	
	// Remove the zoom button
	[[window standardWindowButton:NSWindowZoomButton] setEnabled:NO];
	
	// Set the max size to the current size
	NSSize size = [window frame].size;
	
	[window setMaxSize:size];
	[window setMinSize:size];
}
