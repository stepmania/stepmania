/*
 *  LoadingWindow_Cocoa.mm
 *  stepmania
 *
 *  Created by Steve Checkoway on Thu Jul 03 2003.
 *  Copyright (c) 2003 Steve Checkoway. All rights reserved.
 *
 */

/* This is really dumb. I don't know why it is needed all of a sudden */
#include <math.h>
#ifndef scalb
#define scalb(x, n) scalbn(x, n)
#endif
#ifndef gamma
# define gamma(x) lgamma(x)
#endif
#ifndef _POSIX_SOURCE
# define _POSIX_SOURCE
# define POSIX_DEFINED
#endif
#import <Cocoa/Cocoa.h>
#ifdef POSIX_DEFINED
# undef _POSIX_SOURCE
# undef POSIX_DEFINED
#endif


static NSWindow *window;
static NSTextView *text;

void MakeNewCocoaWindow() {
	NSImage *image = [NSImage imageNamed:@"splash"];
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
