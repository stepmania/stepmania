#import "InstallerWindowController.h"

@implementation InstallerWindowController
- (void)awakeFromNib
{
    helper = [[Helper alloc] initWithPath:@"files"];
}

/* This whole method might be unneeded, I can't remember */
- (void)dealloc
{
    [helper autorelease];
    helper = nil;
}

- (IBAction)pushedButton:(id)sender
{
    [sender setEnabled:NO];
    [NSThread detachNewThreadSelector:@selector(startHelper:) toTarget:helper withObject:self];
}

- (void)postMessage:(NSString *)message
{
    NSString *temp = [NSString stringWithFormat:@"%@\n", message];

    [textView insertText:temp];
}

@end
