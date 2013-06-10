#import "Upgrader.h"
#import "CopyFiles.h"
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>

@implementation Upgrader

// Generate English strings using: genstring -o en.lproj Upgrader.m

static NSString *CHOOSE_FILE;
static NSString *PATCH_FILE;
static NSString *PATCH_COMPLETE;
static NSString *QUIT;
static NSString *ERROR;
static NSString *ERROR_MESSAGE;
static NSString *PATCH_FILES_MISSING;
static NSString *PATCHER_CORRUPTED;
static NSString *CORRUPT_TITLE;
static NSString *PATCHING;
static NSString *SEARCHING;
static NSString *SELECTION;
static NSString *WINDOW_TITLE;

static NSOpenPanel *panel = nil;

- (void)awakeFromNib
{
	NSString *path = [[NSBundle mainBundle] pathForResource:@"Identification" ofType:@"txt"];
	NSData *data = [NSData dataWithContentsOfFile:path];
	NSString *stringData = [[[NSString alloc] initWithData:data encoding:NSUTF8StringEncoding] autorelease];
	
	CHOOSE_FILE		= NSLocalizedString( @"Choose file to patch.", nil );
	PATCH_FILE		= NSLocalizedString( @"Patch file.", nil );
	PATCH_COMPLETE		= NSLocalizedString( @"Patch complete.", nil );
	QUIT			= NSLocalizedString( @"Quit", nil );
	ERROR			= NSLocalizedString( @"Error", nil );
	ERROR_MESSAGE		= NSLocalizedString( @"Patching failed:\n%@", nil );
	PATCH_FILES_MISSING	= NSLocalizedString( @"Patch files missing.", nil );
	PATCHER_CORRUPTED	= NSLocalizedString( @"This patcher has become corrupted.", nil );
	CORRUPT_TITLE		= NSLocalizedString( @"Corrupt Patcher", nil );
	PATCHING		= NSLocalizedString( @"Patching", nil );
	SEARCHING		= NSLocalizedString( @"Searching for %@...", nil );
	SELECTION		= NSLocalizedString( @"You have chosen to patch \"%@\" in the folder:\n%@", nil );
	WINDOW_TITLE		= NSLocalizedString( @"StepMania Patcher", nil );
	
	if( !data )
	{
		NSRunCriticalAlertPanel( CORRUPT_TITLE, PATCHER_CORRUPTED, @"OK", nil, nil );
		[NSApp terminate:self];
	}
		
	m_sName = nil;	
	m_sAppID = nil;
	m_vVersions = nil;	
	
	NSArray *lines = [stringData componentsSeparatedByString:@"\n"];
	NSEnumerator *iter = [lines objectEnumerator];
	NSString *line;
	NSMutableArray *versions = [NSMutableArray array];
	
	while( (line = [iter nextObject]) )
	{
		line = [line stringByTrimmingCharactersInSet:[NSCharacterSet whitespaceAndNewlineCharacterSet]];
		if( [line length] == 0 || [line characterAtIndex:0] == '#' )
			continue;
		if( m_sName == nil )
		{
			m_sName = [line retain];
		}
		else if( m_sAppID == nil )
		{
			m_sAppID = [line retain];
		}
		else if( [line isEqualToString:@"-ALL-"] )
		{
			versions = nil;
			break;
		}
		else
		{
			[versions addObject:line];
		}
	}
	
	if( !m_sName || !m_sAppID )
	{
		NSRunCriticalAlertPanel( CORRUPT_TITLE, PATCHER_CORRUPTED, @"OK", nil, nil );
		[NSApp terminate:self];
	}	
	
	if( versions && [versions count] )
		m_vVersions = [versions retain];
	
	// Set strings.
	[m_StatusText setStringValue:[NSString stringWithFormat:SEARCHING, m_sName]];
	[m_SelectionText setStringValue:@""];
	[m_PatchingText setStringValue:PATCHING];
	
	[m_Searching setUsesThreadedAnimation:YES];
	[m_ProgressBar setUsesThreadedAnimation:YES];
	[m_Window setTitle:WINDOW_TITLE];
	
	[NSThread detachNewThreadSelector:@selector(findApp:) toTarget:self withObject:nil];
	[m_Choose setEnabled:YES];
}

- (void) dealloc
{
	[m_sName release];
	[m_sAppID release];
	[m_vVersions release];
	[super dealloc];
}

- (IBAction) chooseFile:(id)sender
{
	if( !panel )
	{
		panel = [[NSOpenPanel openPanel] retain];
		[panel setCanChooseDirectories:YES];
		[panel setAllowsMultipleSelection:NO];
		[panel setDelegate:self];
		[panel setDirectory:@"/Applications"];
	}
	
	if( [panel runModal] == NSFileHandlingPanelOKButton )
	{
		NSString *dir, *app;
		
		[m_StatusText setStringValue:PATCH_FILE];
		[m_Okay setEnabled:YES];
		[m_sPath release];
		m_sPath = [[panel filename] retain];
		dir = [m_sPath stringByDeletingLastPathComponent];
		app = [[m_sPath lastPathComponent] stringByDeletingPathExtension];
		[m_SelectionText setStringValue:[NSString stringWithFormat:SELECTION, app, dir]];
		[m_Okay setTitle:@"OK"];
		[m_Okay setAction:@selector(upgrade:)];
		[m_Okay setTarget:self];
	}
}

- (IBAction) upgrade:(id)sender
{
	[NSApp beginSheet:m_Panel
	   modalForWindow:[sender window]
	    modalDelegate:self
	   didEndSelector:@selector(sheetEnded:returnCode:contextInfo:)
	      contextInfo:NULL];
	[NSThread detachNewThreadSelector:@selector(upgradeFile:) toTarget:self withObject:m_sPath];		
}

- (void) findApp:(id)obj
{
	NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];

	[m_Searching startAnimation:self];
	
	// Find ITG	
	CFStringRef key = CFSTR( "ApplicationBundlePath" );
	CFPropertyListRef list = CFPreferencesCopyAppValue( key, (CFStringRef)m_sAppID );
	
	NSString *sPath = nil;
	
	if( list && CFGetTypeID(list) != CFDictionaryGetTypeID() )
	{
		CFRelease( list );
		list = NULL;
	}
	if( list )
	{
		CFTypeRef value;
		CFDictionaryRef dict = (CFDictionaryRef)list;
		
		if( m_vVersions )
		{
			NSEnumerator *iter = [m_vVersions objectEnumerator];
			id version;
			
			while( (version = [iter nextObject]) )
			{
				if( CFDictionaryGetValueIfPresent(dict, (CFStringRef)version, &value) &&
				    CFGetTypeID(value) == CFStringGetTypeID() &&
				    [self checkPath:(NSString *)value] )
				{
					sPath = (NSString *)CFRetain( value );
					[sPath autorelease];
				}
			}
		}
		else
		{
			NSDictionary *dict = (NSDictionary *)list;
			NSEnumerator *iter = [dict objectEnumerator];
			id path;
			
			while( (path = [iter nextObject]) )
			{
				if( ![path isKindOfClass:[NSString class]] || ![self checkPath:path] )
					continue;
				sPath = [[path retain] autorelease];
				break;
			}
		}
		CFRelease( list );
	}
	if( !sPath )
	{
		NSWorkspace *ws = [NSWorkspace sharedWorkspace];
		
		[ws findApplications];
		sPath = [ws fullPathForApplication:m_sName];
		
		if( ![self checkPath:sPath] )
			sPath = nil;
	}
	[self performSelectorOnMainThread:@selector(foundApp:) withObject:sPath waitUntilDone:YES];
	[m_Searching stopAnimation:self];
	[pool release];
}

- (void) foundApp:(id)obj
{
	NSString *path = (NSString *)obj;
	
	if( path ) // found
	{
		NSString *dir = [path stringByDeletingLastPathComponent];
		NSString *app = [[path lastPathComponent] stringByDeletingPathExtension];
		
		[m_sPath release];
		m_sPath = [path retain];
		[m_Okay setEnabled:YES];
		[m_StatusText setStringValue:PATCH_FILE];
		[m_SelectionText setStringValue:[NSString stringWithFormat:SELECTION, app, dir]];
	}
	else
	{
		m_sPath = @"";
		[m_StatusText setStringValue:CHOOSE_FILE];
	}
	[m_Choose setEnabled:YES];
	[m_Choose setNeedsDisplay:YES];
	[m_Okay setNeedsDisplay:YES];
	[m_StatusText setNeedsDisplay:YES];
	[m_SelectionText setNeedsDisplay:YES];
}

- (void) upgradeFile:(id)path
{
	NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
	NSBundle *bundle = [NSBundle mainBundle];
	NSString *patch = [bundle pathForResource:@"Contents" ofType:@"" inDirectory:@"Patch"];
	NSArray *parentFiles = [bundle pathsForResourcesOfType:@"" inDirectory:@"Patch/ParentContents"];

	if( patch && parentFiles)
	{
		[m_ProgressBar startAnimation:self];
		
		if( NeedsPrivs(path) )
			m_sError = CopyWithPrivs( patch, path );
		else
			m_sError = Copy( patch, path );
		
		path = [path stringByDeletingLastPathComponent];
		NSEnumerator *enumerator = [parentFiles objectEnumerator];
		while( !m_sError && (patch = [enumerator nextObject]) )
		{
			if( NeedsPrivs(path) )
				m_sError = CopyWithPrivs( patch, path );
			else
				m_sError = Copy( patch, path );
		}
				
		[m_ProgressBar stopAnimation:self];
	}
	else
	{
		m_sError = PATCH_FILES_MISSING;
	}
	[NSApp performSelectorOnMainThread:@selector(endSheet:) withObject:m_Panel waitUntilDone:YES];
	[pool release];
}

- (void) sheetEnded:(NSWindow *)sheet returnCode:(int)returnCode contextInfo:(void *)contextInfo
{
	[m_Panel orderOut:self];
	if( m_sError )
	{
		NSRunCriticalAlertPanel( ERROR, ERROR_MESSAGE, @"OK", nil, nil, m_sError );
		return;
	}
	[m_StatusText setStringValue:PATCH_COMPLETE];
	[m_Okay setTitle:QUIT];
	[m_Okay setAction:@selector(terminate:)];
	[m_Okay setTarget:NSApp];
}

- (BOOL) panel:(id)sender isValidFilename:(NSString *)filename
{
	return [filename hasSuffix:@".app"] && [self checkPath:filename];
}

- (BOOL) panel:(id)sender shouldShowFilename:(NSString *)filename
{
	if( [filename hasSuffix:@".app"] )
		return [self checkPath:filename];
	
	struct stat sb;
	
	if( stat([filename UTF8String], &sb) )
		return NO;
	return (sb.st_mode & S_IFDIR) == S_IFDIR;
}

- (BOOL) checkPath:(NSString *)path
{
	NSBundle *b = [NSBundle bundleWithPath:path];
	
	if( !b )
		return NO;
	if( ![[b objectForInfoDictionaryKey:@"CFBundleIdentifier"] isEqualToString:m_sAppID] )
		return NO;
	if( m_vVersions == nil )
		return YES;
	
	NSString *bundleVersion = [b objectForInfoDictionaryKey:@"CFBundleVersion"];
	NSEnumerator *iter = [m_vVersions objectEnumerator];
	NSString *version;
	
	while( (version = [iter nextObject]) )
		if( [bundleVersion isEqualToString:version] )
			return YES;
	return NO;
}

@end

/*
 * (c) 2006 Steve Checkoway
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

