//
//  Helper.mm
//  stepmania
//
//  Created by Steve Checkoway on Mon Sep 08 2003.
//  Copyright (c) 2003 Steve Checkoway. All rights reserved.
//

#import "Helper.h"
#import "InstallerWindowController.h"
#include "InstallerFile.h"
#include "Processor.h"

static id c;

void HandleFile(const CString& file, const CString& archivePath, bool overwrite)
{
    [c postMessage:[NSString stringWithCString:file]];
}

const CString GetPath(const CString& ID)
{
    [c postMessage:[NSString stringWithFormat:@"Get path for id: %s", ID.c_str()]];
    return "";
}

void Error(const char *fmt, ...)
{
    [c postMessage:@"error of some sort"];
}

bool AskFunc(const CString& question)
{
    return YES;
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

    if(!file.ReadFile())
    {
        [c postMessage:@"Couldn't read the config file"];
        return;
    }

    unsigned nextLine = 0;
    while (nextLine < file.GetNumLines())
        p.ProcessLine(file.GetLine(nextLine), nextLine);

    [c postMessage:@"Installation complete"];
    [pool release];
}
    
@end
