#import <Carbon/Carbon.h>

enum
{
    kMacOSX_10_2_8 = 0x1028
};

extern int NSApplicationMain(int argc, const char *argv[]);

int main(int argc, const char *argv[])
{
    long response;
    OSErr err = Gestalt(gestaltSystemVersion, &response);
    CFStringRef error = NULL;
    
    if (err != noErr)
        error = CFSTR("Couldn't determine OS version.");
    else if (response < kMacOSX_10_2_8)
        error = CFSTR("StepMania will not run on any version of Mac OS X "
					  "earlier than 10.2.8.");
    if (error)
    {
        CFStringRef OK = CFSTR("OK");
        struct AlertStdCFStringAlertParamRec params = {kStdCFStringAlertVersionOne,
			TRUE, FALSE, OK, NULL, NULL, kAlertStdAlertOKButton, NULL,
			kWindowAlertPositionParentWindowScreen, NULL};
        DialogRef dialog;
        SInt16 result;

        CreateStandardAlert(kAlertStopAlert, error, NULL, &params, &dialog);
        AutoSizeDialog(dialog);
        RunStandardAlert(dialog, NULL, &result);
        CFRelease(error);
        
        exit(0);
    }
    
    return NSApplicationMain(argc, argv);
}

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
