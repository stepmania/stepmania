//
//  Helper.mm
//  stepmania
//
//  Created by Steve Checkoway on Mon Sep 08 2003.
//  Copyright (c) 2003 Steve Checkoway. All rights reserved.
//

#import <Security/Authorization.h>
#import <Security/AuthorizationTags.h>
#import <sys/types.h>
#import <sys/wait.h>
#import <string.h>
#import <unistd.h>
#import <fcntl.h>
#import "Helper.h"
#import "InstallerWindowController.h"
#import "InstallerFile.h"
#import "Processor.h"
#import "Util.h"

void Error(const char *fmt, ...);

static id c;
static AuthorizationRef auth;
static BOOL privilegedInstall;

void HandleFile(const CString& file, const CString& dir, const CString& archivePath, bool overwrite)
{
    NSString *filePath = [NSString stringWithCString:dir];

    filePath = [filePath stringByAppendingFormat:@"%s%s", (dir == "/" ? "" : "/"), file.c_str()];
    if (!overwrite && DoesFileExist([filePath cString]))
        return;
    [c postMessage:filePath];
    /* This is rediculus */
    char f[file.length() + 1];
    char d[dir.length() + 1];
    char p[archivePath.length() + 1];
    char path[] = "/usr/bin/tar";
    char arg1[] = "-xPf";
    char arg3[] = "-C";
    char *const arguments[] = {path, arg1, p, arg3, d, f, NULL};

    strcpy(p, archivePath);
    strcpy(d, dir);
    strcpy(f, file);

    if (privilegedInstall)
    {
        AuthorizationFlags flags = kAuthorizationFlagDefaults | kAuthorizationFlagInteractionAllowed |
                                   kAuthorizationFlagExtendRights;
        AuthorizationItem item = {kAuthorizationRightExecute, 0, NULL, 0};
        AuthorizationRights rights = {1, &item};
        OSStatus status = AuthorizationCopyRights(auth, &rights, NULL, flags, NULL);

        if (status != errAuthorizationSuccess)
        {
            [c postMessage:@"Couldn't authorize"];
            return;
        }
        status = AuthorizationExecuteWithPrivileges(auth, path, kAuthorizationFlagDefaults, arguments+1, NULL);
        if (status != errAuthorizationSuccess)
            [c postMessage:@"failed"];
        wait((int*)&status);
    }
    else
    {
        int fd_dev_null = open("/dev/null", O_RDWR, 0);

        if (CallTool3(true, -1, fd_dev_null, fileno(stderr), path, arguments))
            [c postMessage:@"failed"];
        close(fd_dev_null);
    }
}

const CString GetPath(const CString& ID)
{
    NSOpenPanel *panel = [NSOpenPanel openPanel];

    [panel setCanChooseFiles:NO];
    [panel setCanChooseDirectories:YES];
    [panel setResolvesAliases:YES];
    [panel setAllowsMultipleSelection:NO];
    [panel setPrompt:@"Install"];
    [panel setCanCreateDirectories:YES];
    [panel setTitle:@"Choose a location to install."];
    int result = [panel runModalForTypes:[NSArray array]]; // No extensions

    if (result != NSOKButton)
    {
        [c postMessage:@"Canceled install."];
        [c finishedInstalling:NO];
        [NSThread exit];
    }

    NSString *path = [panel filename]; //No need for NSOpenPanel's filenames method

    [c postMessage:[NSString stringWithFormat:@"Chose file \"%@\".", path]];
    
    return [path cString];
}

void Error(const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    NSString *message = [[NSString alloc] initWithFormat:[NSString stringWithCString:fmt] arguments:ap];
    va_end(ap);
    [c postMessage:[message autorelease]];
}

bool AskFunc(const CString& question)
{
    NSString *q = [NSString stringWithCString:question.c_str()];
    BOOL answer = [c askQuestion:q];

    [c postMessage:q];
    [c postMessage:(answer ? @"YES" : @"NO")];
    return answer;
}

bool Auth(void)
{
    AuthorizationFlags flags = kAuthorizationFlagDefaults | kAuthorizationFlagInteractionAllowed |
                               kAuthorizationFlagPreAuthorize | kAuthorizationFlagExtendRights;
    AuthorizationItem item = {kAuthorizationRightExecute, 0, NULL, 0};
    AuthorizationRights rights = {1, &item};
    OSStatus status = AuthorizationCopyRights(auth, &rights, NULL, flags, NULL);
    return status == errAuthorizationSuccess;
}

void Echo(const CString& message, bool loud)
{
    if (!loud)
    {
        [c postMessage:[NSString stringWithCString:message.c_str()]];
        return;
    }

    NSBeginCriticalAlertSheet(@"Warning", @"OK", nil, nil, [c window], c, nil, nil, NULL,
                              [NSString stringWithCString:message.c_str()]);
}

void Priv(bool privileged)
{
    privilegedInstall = privileged;
}
    

@implementation Helper
- (id)initWithPath:(NSString *)path
{
    [super init];
    mPath = [path retain];

    OSStatus status = AuthorizationCreate(NULL, kAuthorizationEmptyEnvironment,
                                          kAuthorizationFlagDefaults, &auth);
    if (status != errAuthorizationSuccess)
    {
        auth = NULL;
        [self autorelease];
        return nil;
    }

    privilegedInstall = NO;

    return self;
}

- (void)dealloc
{
    if (auth)
        AuthorizationFree(auth, kAuthorizationFlagDestroyRights);
    [mPath autorelease];
    [super dealloc];
}

- (void)startHelper:(id)caller
{
    c = caller;
    NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
    CString configPath = [mPath cString];
    CString archivePath = configPath + "/archive.tar.gz";
    BOOL success = NO;

    configPath += "/config";
    InstallerFile file(configPath);
    try
    {
        do {
            Processor p(archivePath, HandleFile, GetPath, AskFunc, YES);

            p.SetErrorFunc(Error);
            p.SetAuthFunc(Auth);
            p.SetEchoFunc(Echo);
            p.SetPrivFunc(Priv);

            [c postMessage:@"Reading from the config file."];

            if(!file.ReadFile())
            {
                [c postMessage:@"Couldn't read the config file"];
                break;
            }

            unsigned nextLine = 0;
            while (nextLine < file.GetNumLines())
                p.ProcessLine(file.GetLine(nextLine), nextLine);
            success = YES;
        }
        while(0);
    }
    catch (CString& e)
    {
        [c postMessage:[NSString stringWithCString:e]];
    }

    [c postMessage:@"Installation complete"];
    [c finishedInstalling:YES];
    [pool release];
}
    
@end
