#import <Cocoa/Cocoa.h>

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
	[text setFont:[NSFont systemFontOfSize:12]];
	[text setString:@"Initializing Hardware..."];
	[text display];
	
	[window makeKeyAndOrderFront:nil];
}

void DisposeOfCocoaWindow() {
	[window close]; /* Released by setReleasedWhenClosed */
}

void SetCocoaWindowText(const char *s) {
	[text setString:[NSString stringWithCString:s]];
	[text display];
}

/*
 * (c) 2003-2004 Steve Checkoway
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
