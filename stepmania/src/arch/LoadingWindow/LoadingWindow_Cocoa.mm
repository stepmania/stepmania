#import <Cocoa/Cocoa.h>
#import "ProductInfo.h"
#import "LoadingWindow_Cocoa.h"
#import "RageUtil.h"
#import "RageFile.h"

@interface LoadingWindowHelper : NSObject
{
	@public
	NSWindow *m_window;
	NSTextView *m_text;
	NSAutoreleasePool *m_Pool;
}
- (void) setupWindow:(NSImage *)image;
@end

@implementation LoadingWindowHelper
- (void) setupWindow:(NSImage *)image
{
	NSSize size = [image size];
	NSRect viewRect, windowRect;
	float height = 0.0f;
	
	NSFont *font = [NSFont systemFontOfSize:0.0f];
	NSRect textRect;
	// Just give it a size until it is created.
	textRect = NSMakeRect( 0, 0, size.width, size.height );
	m_text = [[NSTextView alloc] initWithFrame:textRect];
	[m_text setFont:font];
	height = [[m_text layoutManager] defaultLineHeightForFont:font]*3 + 4;
	textRect = NSMakeRect( 0, 0, size.width, height );
	
	[m_text setFrame:textRect];
	[m_text setEditable:NO];
	[m_text setSelectable:NO];
	[m_text setDrawsBackground:YES];
	[m_text setBackgroundColor:[NSColor lightGrayColor]];
	[m_text setAlignment:NSCenterTextAlignment];
	[m_text setHorizontallyResizable:NO];
	[m_text setVerticallyResizable:NO];
	[m_text setString:@"Initializing Hardware..."];
	
	viewRect = NSMakeRect( 0, height, size.width, size.height );
	NSImageView *iView = [[NSImageView alloc] initWithFrame:viewRect];
	[iView setImage:image];
	[iView setImageFrameStyle:NSImageFrameNone];
	
	windowRect = NSMakeRect( 0, 0, size.width, size.height + height );
	m_window = [[NSWindow alloc] initWithContentRect:windowRect
					       styleMask:NSTitledWindowMask
						 backing:NSBackingStoreBuffered
						   defer:YES];
	
	NSView *view = [m_window contentView];
	
	// Set some properties.
	[m_window setOneShot:YES];
	[m_window setReleasedWhenClosed:YES];
	[m_window setExcludedFromWindowsMenu:YES];
	[m_window useOptimizedDrawing:YES];
	[m_window setTitle:@PRODUCT_FAMILY];
	[m_window center];

	// Set subviews.
	[view addSubview:m_text];
	[view addSubview:iView];
	[m_text release];
	[iView release];

	// Display the window.
	[m_window makeKeyAndOrderFront:nil];
}	
@end

static LoadingWindowHelper *g_helper = nil;

LoadingWindow_Cocoa::LoadingWindow_Cocoa()
{
	RageFile f;
	RString data;
	
	vector<RString> vs;
	GetDirListing( "Data/splash*.png", vs, false, true );
	if( vs.empty() || !f.Open(vs[0]) )
		return;
	f.Read( data );
	if( data.empty() )
		return;

	NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
	NSImage *image = nil;
	NSData *d = [[NSData alloc] initWithBytes:data.data() length:data.length()];
	image = [[NSImage alloc] initWithData:d];
	[d release];
	if( !image )
	{
		[pool release];
		return;
	}
	
	g_helper = [[LoadingWindowHelper alloc] init];
	g_helper->m_Pool = pool;
	[g_helper performSelectorOnMainThread:@selector(setupWindow:) withObject:image waitUntilDone:YES];
	[image release];
}

LoadingWindow_Cocoa::~LoadingWindow_Cocoa()
{
	if( !g_helper )
		return;
	NSAutoreleasePool *pool = g_helper->m_Pool;
	[g_helper->m_window performSelectorOnMainThread:@selector(close) withObject:nil waitUntilDone:YES];
	[g_helper release];
	g_helper = nil;
	[pool release];
}

void LoadingWindow_Cocoa::SetText( RString str )
{
	if( !g_helper )
		return;
	NSString *s = [[NSString alloc] initWithUTF8String:str];
	[g_helper->m_text performSelectorOnMainThread:@selector(setString:) withObject:(s ? s : @"") waitUntilDone:NO];
	[s release];
}

/*
 * (c) 2003-2006, 2008 Steve Checkoway
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
