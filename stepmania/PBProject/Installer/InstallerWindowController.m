#import "InstallerWindowController.h"

@implementation InstallerWindowController
- (void)awakeFromNib
{
    NSString *path = [[[[NSBundle mainBundle] resourcePath] stringByAppendingPathComponent:@"files"]
        stringByStandardizingPath];
    helper = [[Helper alloc] initWithPath:path];
    finished = NO;
}

/* This whole method might be unneeded, I can't remember */
- (void)dealloc
{
    [helper autorelease];
    helper = nil;
}

- (IBAction)pushedButton:(id)sender
{
    if (finished)
        [NSApp terminate:self];
    [sender setEnabled:NO];
    [[[self window] standardWindowButton:NSWindowCloseButton] setEnabled:NO];
    [self postMessage:@"Starting installation."];
    [NSThread detachNewThreadSelector:@selector(startHelper:) toTarget:helper withObject:self];
}

- (void)postMessage:(NSString *)message
{
	NSRange range;
	NSString *s = [message stringByAppendingString:@"\n"];
	
	range.location = [[textView textStorage] length];
	range.length = 0;
	[textView replaceCharactersInRange:range withString:s];
	range.length = [s length];
	[textView scrollRangeToVisible:range];
}

- (void)finishedInstalling:(BOOL)success
{
    [[[self window] standardWindowButton:NSWindowCloseButton] setEnabled:YES];
    if (success)
    {
        [button setTitle:@"Quit Installer"];
        [button setEnabled:YES];
        finished = YES;
    }
    else
        [button setEnabled:YES];
}

- (BOOL)askQuestion:(NSString *)question
{
    [questionField setStringValue:question];
    return [NSApp runModalForWindow:questionPanel];
}

- (IBAction)pushedQuestionButton:(id)sender
{
    [questionPanel orderOut:sender];
    [NSApp stopModalWithCode:[sender tag]];
}

- (void)windowWillClose:(NSNotification *)notification
{
    [NSApp terminate:self];
}
@end

/*
 * (c) 2003 Steve Checkoway
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
