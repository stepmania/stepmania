#import <Cocoa/Cocoa.h>
#import "ProductInfo.h"
#import "archutils/Darwin/SMMainThread.h"

static NSWindow *window;
static NSTextView *text;

void MakeNewCocoaWindow( const void *data, unsigned length )
{
	NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
	NSImage *image = nil;
	
	if( length )
	{
		NSData *d = [NSData dataWithBytes:data length:length];
		
		image = [[[NSImage alloc] initWithData:d] autorelease];
	}
	
	if( !image )
		image = [NSImage imageNamed:@"splash.png"];
	
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
	[text setFont:[NSFont systemFontOfSize:12]];
	[text setString:@"Initializing Hardware..."];

	viewRect = NSMakeRect(0, height, size.width, height);
	NSImageView *iView = [[[NSImageView alloc] initWithFrame:viewRect] autorelease];
	[iView setImage:image];
	[iView setImageFrameStyle:NSImageFrameNone];

	windowRect = NSMakeRect(0, 0, size.width, height + height);
	window = [[NSWindow alloc] initWithContentRect:windowRect
					     styleMask:NSTitledWindowMask
					       backing:NSBackingStoreBuffered
						 defer:YES];
	
	SMMainThread *mt = [[SMMainThread alloc] init];
	NSNumber *bYES = [NSNumber numberWithBool:YES];
	view = [window contentView];

	// Set some properties.
	[mt addAction:@selector(setOneShot:) withTarget:window andObject:bYES];
	[mt addAction:@selector(setReleasedWhenClosed:) withTarget:window andObject:bYES];
	[mt addAction:@selector(setExcludedFromWindowsMenu:) withTarget:window andObject:bYES];
	[mt addAction:@selector(useOptimizedDrawing:) withTarget:window andObject:bYES];
	[mt addAction:@selector(center) withTarget:window];
	[mt addAction:@selector(setTitle:) withTarget:window andObject:[NSString stringWithUTF8String:PRODUCT_NAME]];
	// Set subviews.
	[mt addAction:@selector(addSubview:) withTarget:view andObject:text];
	[mt addAction:@selector(addSubview:) withTarget:view andObject:iView];
	// Make key and order front.
	[mt addAction:@selector(makeKeyAndOrderFront:) withTarget:window andObject:nil];
	// Perform all of the actions in order on the main thread.
	[mt performOnMainThread];
	[mt release];
	
	[pool release];
}

void DisposeOfCocoaWindow()
{
	// Just in case performSelectorOnMainThread:withObject:waitUntilDone: needs an autorelease pool.
	NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
	
	 // Released when closed as controlled by setReleasedWhenClosed.
	[window performSelectorOnMainThread:@selector(close) withObject:nil waitUntilDone:NO];
	[pool release];
}

void SetCocoaWindowText(const char *s)
{
	NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
	NSString *str = [NSString stringWithUTF8String:s];
	[text performSelectorOnMainThread:@selector(setString:) withObject:(str ? str : @"") waitUntilDone:NO];
	[pool release];
}

/*
 * (c) 2003-2006 Steve Checkoway
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
