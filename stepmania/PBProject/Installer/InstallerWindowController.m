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
    [self postMessage:@"Starting installation."];
    [NSThread detachNewThreadSelector:@selector(startHelper:) toTarget:helper withObject:self];
}

- (void)postMessage:(NSString *)message
{
    NSTextStorage *storage = [textView textStorage];
    NSAttributedString *temp = [[[NSAttributedString alloc] initWithString:
        [NSString stringWithFormat:@"%@\n", message]] autorelease];

    [storage appendAttributedString:temp];
}

- (void)finishedInstalling:(BOOL)success
{
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

@end
