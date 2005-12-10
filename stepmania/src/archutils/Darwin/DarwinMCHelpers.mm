#include "global.h"
#include "DarwinMCHelpers.h"
#include "RageThreads.h"

#import <Cocoa/Cocoa.h>

static bool g_bChange;
static RageMutex g_Lock( "USB devices changed lock" );

@interface MemoryCardHelper : NSObject
- (void) devicesChanged:(NSNotification *)notification;
@end
@implementation MemoryCardHelper
- (void) devicesChanged:(NSNotification *)notification
{
	LockMut(g_Lock);
	g_bChange = true;
}
@end

static MemoryCardHelper *g_MCH = nil;

void DarwinMCHelpers::Start()
{
	ASSERT( g_MCH == nil );
	NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
	NSNotificationCenter *nc = [[NSWorkspace sharedWorkspace] notificationCenter];
	
	g_bChange = true;
	g_MCH = [[MemoryCardHelper alloc] init];
	[nc addObserver:g_MCH selector:@selector(devicesChanged:)
			   name:@"NSWorkspaceDidMountNotification" object:nil];
	[nc addObserver:g_MCH selector:@selector(devicesChanged:)
			   name:@"NSWorkspaceDidUnmountNotification" object:nil];
	
	[pool release];
}

void DarwinMCHelpers::Stop()
{
	NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
	NSNotificationCenter *nc = [[NSWorkspace sharedWorkspace] notificationCenter];

	[nc removeObserver:g_MCH];
	[g_MCH release];
	g_MCH = nil;
	[pool release];
}

bool DarwinMCHelpers::DevicesChanged()
{
	LockMut( g_Lock );
	return g_bChange;
}

void DarwinMCHelpers::GetRemovableDevicePaths( vector<CString>& vDevicePaths )
{
	LockMut( g_Lock );
	NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
	NSWorkspace *ws = [NSWorkspace sharedWorkspace];
	NSArray *paths = [ws mountedRemovableMedia];
	NSEnumerator *i = [paths objectEnumerator];
	NSString *path;
	
	while( (path = [i nextObject]) )
		vDevicePaths.push_back( [path UTF8String] );
	[pool release];
	g_bChange = false;
}

/*
 * (c) 2005 Steve Checkoway
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
