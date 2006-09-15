/* Upgrader */

#import <Cocoa/Cocoa.h>

@interface Upgrader : NSObject
{
    IBOutlet NSButton *m_Choose;
    IBOutlet NSButton *m_Okay;
    IBOutlet NSPanel *m_Panel;
    IBOutlet NSProgressIndicator *m_ProgressBar;
    IBOutlet NSProgressIndicator *m_Searching;
    IBOutlet NSTextField *m_StatusText;
    IBOutlet NSTextField *m_SelectionText;
    IBOutlet NSTextField *m_PatchingText;
    IBOutlet NSWindow *m_Window;
    NSString *m_sAppID;
    NSArray *m_vVersions;
    NSString *m_sName;
    NSString *m_sError;
    NSString *m_sPath;
}
- (IBAction) chooseFile:(id)sender;
- (IBAction) upgrade:(id)sender;
- (void) findApp:(id)obj;
- (void) foundApp:(id)obj;
- (void) upgradeFile:(id)path;
- (void) sheetEnded:(NSWindow *)sheet returnCode:(int)returnCode contextInfo:(void *)contextInfo;
- (BOOL) panel:(id)sender isValidFilename:(NSString *)filename;
- (BOOL) panel:(id)sender shouldShowFilename:(NSString *)filename;
- (BOOL) checkPath:(NSString *)path;
@end

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
