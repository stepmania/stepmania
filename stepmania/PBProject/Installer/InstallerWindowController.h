/* InstallerWindowController */

#import <Cocoa/Cocoa.h>
#import "Helper.h"

@interface InstallerWindowController : NSWindowController
{
    IBOutlet NSButton *button;
    IBOutlet NSTextView *textView;
    IBOutlet NSTextField *questionField;
    IBOutlet NSPanel *questionPanel;
    BOOL installing;
    Helper *helper;
    BOOL finished;
}
- (void)awakeFromNib;
- (void)dealloc;
- (IBAction)pushedButton:(id)sender;
- (void)postMessage:(NSString *)message;
- (void)finishedInstalling:(BOOL)success;
- (BOOL)askQuestion:(NSString *)question;
- (IBAction)pushedQuestionButton:(id)sender;
@end
