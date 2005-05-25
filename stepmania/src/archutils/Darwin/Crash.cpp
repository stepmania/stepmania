#include "StdString.h"
#include "Crash.h"
#include "archutils/Unix/CrashHandler.h"
#include <Carbon/Carbon.h>

void InformUserOfCrash( const CString &sPath )
{
	CFStringRef error;
	
	error = CFStringCreateWithCString(kCFAllocatorDefault,
									  "StepMania has crashed.  Debug "
									  "information has been output to\n\n    "
									  + sPath + "\n\nPlease report a bug at:"
									  "\n\nhttp://sourceforge.net/tracker/"
									  "?func=add&group_id=37892&atid=421366",
									  kCFStringEncodingASCII);
    CFStringRef OK = CFSTR("Open crashinfo");
    CFStringRef Cancel = CFSTR("Cancel");
    struct AlertStdCFStringAlertParamRec params = {kStdCFStringAlertVersionOne,
		true, false, OK, Cancel, NULL, kAlertStdAlertOKButton,
		kAlertStdAlertCancelButton, kWindowAlertPositionParentWindowScreen,
		NULL};
    DialogRef dialog;
    SInt16 button;

    CreateStandardAlert(kAlertStopAlert, error, NULL, &params, &dialog);
    AutoSizeDialog(dialog);
    RunStandardAlert(dialog, NULL, &button);
    switch (button)
    {
        case kAlertStdAlertOKButton:
            // This is lazy, and easy, and it seems to work.
            if (system(NULL))
                system("/usr/bin/open '" + sPath + "'");
            else
                SysBeep(30);
            break;
    }
    CFRelease(OK);
    CFRelease(Cancel);
    CFRelease(error);
}

/*
 * (c) 2003-2005 Steve Checkoway
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
