//
//  Helper.mm
//  stepmania
//
//  Created by Steve Checkoway on Mon Sep 08 2003.
//  Copyright (c) 2003 Steve Checkoway. All rights reserved.
//

#import <string.h>
#import "Helper.h"
#import "InstallerWindowController.h"
#include "InstallerFile.h"
#include "Processor.h"

void Error(const char *fmt, ...);

static id c;

void HandleFile(const CString& file, const CString& dir, const CString& archivePath, bool overwrite)
{
    [c postMessage:[NSString stringWithCString:file]];
    
    CString command = "tar zxfPC '" + archivePath + "' '" + dir + "' '" + file + "'";
    if (system(command))
        Error(strerror(errno));
}

const CString GetPath(const CString& ID)
{
    NSOpenPanel *panel = [NSOpenPanel openPanel];

    [panel setCanChooseFiles:NO];
    [panel setCanChooseDirectories:YES];
    [panel setResolvesAliases:YES];
    [panel setAllowsMultipleSelection:NO];
    int result = [panel runModalForTypes:[NSArray array]]; // No extensions

    if (result != NSOKButton)
    {
        [c postMessage:@"Canceled install."];
        [c finishedInstalling:NO];
        [NSThread exit];
    }

    NSString *path = [panel filename]; //No need for NSOpenPanel's filenames method

    [c postMessage:[NSString stringWithFormat:@"Choose file \"%@\".", path]];
    
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

@implementation Helper
- (id)initWithPath:(NSString *)path
{
    [super init];
    mPath = [path retain];
    return self;
}

- (void)dealloc
{
    [mPath autorelease];
    [super dealloc];
}

- (void)startHelper:(id)caller
{
    c = caller;
    NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
    CString configPath = [mPath cString];
    CString archivePath = configPath + "/archive.tar.gz";

    configPath += "/config";
    InstallerFile file(configPath);
    Processor p(archivePath, HandleFile, GetPath, AskFunc, YES);

    p.SetErrorFunc(Error);

    [c postMessage:@"Reading from the config file."];

    if(!file.ReadFile())
    {
        [c postMessage:@"Couldn't read the config file"];
        [c finishedInstalling:NO];
        [pool release];
        return;
    }

    unsigned nextLine = 0;
    while (nextLine < file.GetNumLines())
        p.ProcessLine(file.GetLine(nextLine), nextLine);

    [c postMessage:@"Installation complete"];
    [c finishedInstalling:YES];
    [pool release];
}
    
@end
