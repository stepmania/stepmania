/*
 *  LoadingWindow_Cocoa.mm
 *  stepmania
 *
 *  Created by Steve Checkoway on Thu Jul 03 2003.
 *  Copyright (c) 2003 Steve Checkoway. All rights reserved.
 *
 */

#import <Cocoa/Cocoa.h>

static NSWindow *window;
static NSTextView *text;

void MakeNewCocoaWindow() {
	NSImage *image = [NSImage imageNamed:@"loading"];
	NSSize size = [image size];
	float height = size.height;
	NSImageView *iView = [[[NSImageView alloc] initWithFrame:NSMakeRect(0, height+1, size.width, height)] autorelease];
	NSView *view;
	NSRect rect;
	rect.origin = NSMakePoint(0, height);
	rect.size = size;
	rect.size.height += height;

	text = [[[NSTextView alloc] initWithFrame:NSMakeRect(0, 0, rect.size.width, height)] autorelease];
	[text setEditable:NO];
	[text setSelectable:NO];
	[text setDrawsBackground:YES];
	[text setBackgroundColor:[NSColor lightGrayColor]];
	[text setAlignment:NSCenterTextAlignment];

	[iView setImage:image];
	[iView setImageFrameStyle:NSImageFrameNone];

	window = [[NSWindow alloc] initWithContentRect:rect styleMask:NSBorderlessWindowMask backing:NSBackingStoreBuffered defer:YES];
	[window setOneShot:YES];
	[window setReleasedWhenClosed:YES];
	[window center];
	[window useOptimizedDrawing:YES];
	[window setViewsNeedDisplay:YES];

	view = [window contentView];
	[view addSubview:text]; /* This retains text */
	[view addSubview:iView]; /* This retains iView */
	[text setString:@"Initializing Hardware..."];
	
	[window makeKeyAndOrderFront:nil];
}

void DisposeOfCocoaWindow() {
	[window close]; /* Released by setReleasedWhenClosed */
}

void SetCocoaWindowText(const char *s) {
	[text setString:[NSString stringWithCString:s]];
	[text display];
}
