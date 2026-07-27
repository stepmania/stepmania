#import <Cocoa/Cocoa.h>
#import "ProductInfo.h"
#import "LoadingWindow_MacOSX.h"
#import "RageUtil.h"
#import "RageFile.h"
#include "ThemeManager.h"

#include <vector>


@interface LoadingWindowHelper : NSObject
{
	@public
	NSWindow *m_Window;
	NSTextView *m_Text;
	NSAutoreleasePool *m_Pool;
	NSProgressIndicator *m_ProgressIndicator;
}
- (void) setupWindow:(NSImage *)image;
- (void) setProgress:(NSNumber *)progress;
- (void) setTotalWork:(NSNumber *)totalWork;
- (void) setIndeterminate:(NSNumber *)indeterminate;
@end

@implementation LoadingWindowHelper
- (void) setupWindow:(NSImage *)image
{
	NSSize size = [image size];
	NSRect viewRect, windowRect;
	float height = 0.0f;
	float padding = 5.0f;

	NSRect progressIndicatorRect;
	progressIndicatorRect = NSMakeRect(padding, padding, size.width-padding*2.0f, 0);
	m_ProgressIndicator = [[NSProgressIndicator alloc] initWithFrame:progressIndicatorRect];
	[m_ProgressIndicator sizeToFit];
	[m_ProgressIndicator setIndeterminate:YES];
	[m_ProgressIndicator setMinValue:0];
	[m_ProgressIndicator setMaxValue:1];
	[m_ProgressIndicator setDoubleValue:0];
	progressIndicatorRect = [m_ProgressIndicator frame];
	float progressHeight = progressIndicatorRect.size.height;

	NSFont *font = [NSFont systemFontOfSize:0.0f];
	NSRect textRect;
	// Just give it a size until it is created.
	textRect = NSMakeRect( 0, progressHeight + padding, size.width, size.height );
	m_Text = [[NSTextView alloc] initWithFrame:textRect];
	[m_Text setFont:font];
	height = [[m_Text layoutManager] defaultLineHeightForFont:font]*3 + 4;
	textRect = NSMakeRect( 0, progressHeight + padding, size.width, height );

	[m_Text setFrame:textRect];
	[m_Text setEditable:NO];
	[m_Text setSelectable:NO];
	[m_Text setDrawsBackground:NO];
	[m_Text setBackgroundColor:[NSColor lightGrayColor]];
	[m_Text setAlignment:NSTextAlignmentCenter];
	[m_Text setHorizontallyResizable:NO];
	[m_Text setVerticallyResizable:NO];
	[m_Text setString:@"Initializing Hardware..."];

	viewRect = NSMakeRect( 0, height + progressHeight + padding, size.width, size.height );
	NSImageView *iView = [[NSImageView alloc] initWithFrame:viewRect];
	[iView setImage:image];
	[iView setImageFrameStyle:NSImageFrameNone];

	windowRect = NSMakeRect( 0, 0, size.width, size.height + height + progressHeight + padding);
	m_Window = [[NSWindow alloc] initWithContentRect:windowRect
							styleMask:NSWindowStyleMaskTitled
							backing:NSBackingStoreBuffered
							defer:YES];

	NSView *view = [m_Window contentView];

	// Set some properties.
	[m_Window setReleasedWhenClosed:YES];
	[m_Window setExcludedFromWindowsMenu:YES];
	[m_Window setTitle:@PRODUCT_FAMILY];
	[m_Window center];

	// Set subviews.
	[view addSubview:m_Text];
	[m_Text release];

	[view addSubview:iView];
	[iView release];

	[view addSubview:m_ProgressIndicator];

	// Display the window.
	[m_Window makeKeyAndOrderFront:nil];
}

- (void) setProgress:(NSNumber *)progress
{
	[m_ProgressIndicator setDoubleValue:[progress doubleValue]];
}

- (void) setTotalWork:(NSNumber *)totalWork
{
	[m_ProgressIndicator setMaxValue:[totalWork doubleValue]];
}

- (void) setIndeterminate:(NSNumber *)indeterminate
{
	[m_ProgressIndicator setIndeterminate:([indeterminate doubleValue] > 0 ? YES : NO)];
}

@end

static LoadingWindowHelper *g_Helper = nil;

LoadingWindow_MacOSX::LoadingWindow_MacOSX() {}

LoadingWindow_MacOSX::~LoadingWindow_MacOSX()
{
	if( !g_Helper )
		return;
	NSAutoreleasePool *pool = g_Helper->m_Pool;
	[g_Helper->m_Window performSelectorOnMainThread:@selector(close) withObject:nil waitUntilDone:YES];
	[g_Helper release];
	g_Helper = nil;
	[pool release];
}

void LoadingWindow_MacOSX::SetText( RString str )
{
	if( !g_Helper )
		return;
	NSString *s = [[NSString alloc] initWithUTF8String:str];
	[g_Helper->m_Text performSelectorOnMainThread:@selector(setString:) withObject:(s ? s : @"") waitUntilDone:NO];
	[s release];
}

void LoadingWindow_MacOSX::SetSplash( const RageSurface *pSplash )
{
	RageFile f;
	RString data;
	std::vector<RString> vs;

	// Try to load a custom splash from the current theme, first.
	GetDirListing( THEME->GetPathG( "Common", "splash"), vs, false, true );

	//	if no Common splash file was found in the theme...
	if( vs.empty() || !f.Open(vs[0]) )
	{
		// then try the stock splash.png from ./Data/
		GetDirListing( "Data/splash*.png", vs, false, true );
	}

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

	g_Helper = [[LoadingWindowHelper alloc] init];
	g_Helper->m_Pool = pool;
	[g_Helper performSelectorOnMainThread:@selector(setupWindow:) withObject:image waitUntilDone:NO];
	[image release];
}

void LoadingWindow_MacOSX::SetProgress( const int progress )
{
	[g_Helper performSelectorOnMainThread:@selector(setProgress:) withObject:[NSNumber numberWithDouble:(double)progress] waitUntilDone:NO];
}

void LoadingWindow_MacOSX::SetTotalWork( const int totalWork )
{
	[g_Helper performSelectorOnMainThread:@selector(setTotalWork:) withObject:[NSNumber numberWithDouble:(double)totalWork] waitUntilDone:NO];
}

void LoadingWindow_MacOSX::SetIndeterminate( bool indeterminate )
{
	double tmp = indeterminate ? 1 : 0;
	[g_Helper performSelectorOnMainThread:@selector(setIndeterminate:) withObject:[NSNumber numberWithDouble:tmp] waitUntilDone:NO];
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
