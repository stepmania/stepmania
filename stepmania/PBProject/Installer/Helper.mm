#import <sys/types.h>
#import <sys/wait.h>
#import <cstring>
#import <unistd.h>
#import <fcntl.h>
#import "Helper.h"
#import "InstallerWindowController.h"
#import "InstallerFile.h"
#import "Processor.h"
#import "Util.h"

void Error(const char *fmt, ...);

static id c;

void HandleFile(const CString& file, const CString& dir,
				const CString& archivePath)
{
    NSString *filePath = [NSString stringWithCString:dir];

    filePath = [filePath stringByAppendingFormat:@"%s%s",
						 (dir == "/" ? "" : "/"), file.c_str()];
    [c postMessage:filePath];
    /* This is rediculus */
	char path[] = "/bin/cp";
	char arg[] = "-RP";
	CString fromString = archivePath + '/' + file;
	char from[fromString.length() + 1];
	CString toString = dir + '/' + file;
	char to[toString.length() + 1];
	char *const arguments[] = {path, arg, from, to, NULL};
	
	strcpy(from, fromString);
	strcpy(to, toString);
	MakeAllButLast(to);
	
	int fd_dev_null = open("/dev/null", O_RDWR, 0);

    if (CallTool3(true, -1, fd_dev_null, fileno(stderr), path, arguments))
        [c postMessage:@"failed"];
    close(fd_dev_null);
}

const CString GetPath(const CString& ID)
{
    NSOpenPanel *panel = [NSOpenPanel openPanel];

    [panel setCanChooseFiles:NO];
    [panel setCanChooseDirectories:YES];
    [panel setResolvesAliases:YES];
    [panel setAllowsMultipleSelection:NO];
    [panel setPrompt:@"Install"];
    // Only in 10.3 and above
    if ([panel respondsToSelector:@selector(setCanCreateDirectories:)])
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
    //CString archivePath = configPath + "/archive.tar.gz";
	CString archivePath = configPath;
    BOOL success = NO;

    configPath += "/config";
    InstallerFile file(configPath);
    try
    {
        do {
            Processor p(archivePath, HandleFile, GetPath, AskFunc, YES);

            p.SetErrorFunc(Error);
            p.SetEchoFunc(Echo);

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
