#import <Cocoa/Cocoa.h>
#import "ProductInfo.h"

static NSWindow *window;
static NSTextView *text;

void MakeNewCocoaWindow()
{
	NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
	NSImage *image = [NSImage imageNamed:@"splash"];
	NSImageView *iView;
	NSView *view;
	NSSize size = [image size];
	NSRect textRect, viewRect, windowRect;
	float height = size.height;
	
	textRect = NSMakeRect(0, 0, size.width, height);
	text = [[[NSTextView alloc] initWithFrame:textRect] autorelease];
	[text setEditable:NO];
	[text setSelectable:NO];
	[text setDrawsBackground:YES];
	[text setBackgroundColor:[NSColor lightGrayColor]];
	[text setAlignment:NSCenterTextAlignment];

	viewRect = NSMakeRect(0, height, size.width, height);
	iView = [[[NSImageView alloc] initWithFrame:viewRect] autorelease];
	[iView setImage:image];
	[iView setImageFrameStyle:NSImageFrameNone];

	windowRect = NSMakeRect(0, 0, size.width, height + height);
	window = [[NSWindow alloc] initWithContentRect:windowRect
										 styleMask:NSTitledWindowMask
										   backing:NSBackingStoreBuffered
											 defer:YES];
	[window setOneShot:YES];
	[window setReleasedWhenClosed:YES];
	[window center];
	[window useOptimizedDrawing:YES];
	[window setViewsNeedDisplay:YES];
	[window setTitle:[NSString stringWithUTF8String:PRODUCT_NAME_VER]];

	view = [window contentView];
	[view addSubview:text]; /* This retains text */
	[view addSubview:iView]; /* This retains iView */
	[text setFont:[NSFont systemFontOfSize:12]];
	[text setString:@"Initializing Hardware..."];
	[text display];
	
	[window makeKeyAndOrderFront:nil];
	[pool release];
}

void DisposeOfCocoaWindow()
{
	// Just in case
	NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
	
	[window close]; /* Released by setReleasedWhenClosed */
	[pool release];
}

void SetCocoaWindowText(const char *s)
{
	NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
	NSString *str = [NSString stringWithUTF8String:s];
	
	[text setString:( str ? str : @"" )];
	[text setNeedsDisplay:YES];
	[pool release];
}

/*
 * (c) 2003-2005 Steve Checkoway
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
