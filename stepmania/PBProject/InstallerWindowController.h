/* InstallerWindowController */

#import <Cocoa/Cocoa.h>
#import "Helper.h"

@interface InstallerWindowController : NSWindowController
{
    IBOutlet NSButton *button;
    IBOutlet NSTextView *textView;
    BOOL installing;
    Helper *helper;
}
- (void)awakeFromNib;
- (void)dealloc;
- (IBAction)pushedButton:(id)sender;
- (void)postMessage:(NSString *)message;
@end
