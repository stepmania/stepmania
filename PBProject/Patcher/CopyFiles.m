#import "CopyFiles.h"
#include <Security/Authorization.h>
#include <Security/AuthorizationTags.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <dirent.h>
#include <unistd.h>

static NSString *COPY_FAILED;
static NSString *EXECUTE_FAILED;
static NSString *AUTH_FAILED;

static void LoadStrings()
{
	static bool bLoaded = false;
	if( bLoaded )
		return;
	COPY_FAILED	= NSLocalizedString( @"Copy failed.", nil );
	EXECUTE_FAILED	= NSLocalizedString( @"Execute failed.", nil );
	AUTH_FAILED	= NSLocalizedString( @"Authentication failed. File not patched.", nil );
	bLoaded = true;
}

static BOOL CheckDir( const char *path )
{
	DIR *dir = opendir( path );
	int fd = dirfd( dir );
	struct dirent entry, *ep;
	int err;
	
	fchdir( fd );
	for( err = readdir_r(dir, &entry, &ep); ep && !err; err = readdir_r(dir, &entry, &ep) )
	{
		if( !strcmp(".", entry.d_name) || !strcmp("..", entry.d_name) )
			continue;
		if( access(entry.d_name, W_OK|R_OK) )
		{
			closedir( dir );
			return YES;
		}
		if( entry.d_type == DT_DIR )
		{
			if( CheckDir(entry.d_name) )
			{
				closedir( dir );
				return YES;
			}
			fchdir( fd );
		}
	}
	closedir( dir );
	return err;
}

BOOL NeedsPrivs( NSString *path )
{
	const char *p = [path UTF8String];
	struct stat sb;
	
	if( stat(p, &sb) )
		return YES;
	if( access(p, W_OK|R_OK) ) // I don't care to parse stat output.
		return YES;
	if( (sb.st_mode & S_IFDIR) && CheckDir(p) )
		return YES;
	
	return NO;
}

NSString *CopyWithPrivs( NSString *src, NSString *dest )
{
	LoadStrings();
	OSStatus error;
	static AuthorizationRef authRef;
	static BOOL bCreated = false;
	
	if( !bCreated )
	{
		AuthorizationFlags flags = kAuthorizationFlagDefaults;
		AuthorizationItem item = { kAuthorizationRightExecute, 0, NULL, 0 };
		AuthorizationRights rights = { 1, &item };

		error = AuthorizationCreate( NULL, kAuthorizationEmptyEnvironment, flags, &authRef );
		
		if (error != errAuthorizationSuccess)
			return @"Failed to create authorization.";
		
		
		flags = kAuthorizationFlagDefaults |
			kAuthorizationFlagInteractionAllowed |
			kAuthorizationFlagPreAuthorize |
			kAuthorizationFlagExtendRights;
		
		error = AuthorizationCopyRights( authRef, &rights, NULL, flags, NULL );
		
		if (error != errAuthorizationSuccess)
			return AUTH_FAILED;
		bCreated = YES;
	}

	
	// Don't pass path as the first argument. AuthorizationExecuteWithPrivileges must do it.
	char path[] = "/bin/cp";
	const char *argv[] = { "-Rf", [src UTF8String], [dest UTF8String], NULL };
	NSString *result = nil;
	
	error = AuthorizationExecuteWithPrivileges( authRef, path, kAuthorizationFlagDefaults, (char **)argv, NULL );
	
	if (error == errAuthorizationSuccess)
	{
		int status = 0;
		pid_t pid = wait( &status );
		
		if( pid == -1 )
			result = [NSString stringWithUTF8String:strerror(errno)];
		else if( !WIFEXITED(status) || WEXITSTATUS(status) )
			result = COPY_FAILED;
	}
	else
	{
		result = EXECUTE_FAILED;
	}
	
	
	return result;
}

NSString *Copy( NSString *src, NSString *dest )
{
	LoadStrings();
	// Again, don't pass the path.
	NSArray *argv = [NSArray arrayWithObjects:@"-Rf", src, dest, nil];
	NSTask *task = [NSTask launchedTaskWithLaunchPath:@"/bin/cp" arguments:argv];
	
	[task waitUntilExit];
	return [task terminationStatus] ? COPY_FAILED : nil;
}

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
