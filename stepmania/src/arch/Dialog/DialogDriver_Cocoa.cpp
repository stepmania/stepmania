#include "global.h"
#include "DialogDriver_Cocoa.h"
#include "RageThreads.h"
#include "ProductInfo.h"
#include <Carbon/Carbon.h>

static CFOptionFlags ShowAlert( CFOptionFlags flags, CFStringRef message,
								CFStringRef OK, CFStringRef cancel = NULL )
{
	CFOptionFlags result;
	
	CFUserNotificationDisplayAlert( 0.0, flags, NULL, NULL, NULL, CFSTR(PRODUCT_NAME_VER),
									message, OK, cancel, NULL, &result );
	return result;
}

void DialogDriver_Cocoa::OK( CString sMessage, CString sID )
{
	CFStringRef message = CFStringCreateWithCString( NULL, sMessage, kCFStringEncodingASCII );
	CFOptionFlags result = ShowAlert( kCFUserNotificationNoteAlertLevel, message,
									  CFSTR("OK"), CFSTR("Don't show again") );

	CFRelease( message );
	if( result == kCFUserNotificationAlternateResponse )
		Dialog::IgnoreMessage( sID );
}

void DialogDriver_Cocoa::Error( CString sError, CString sID )
{
	CFStringRef error = CFStringCreateWithCString( NULL, sError, kCFStringEncodingASCII );
	ShowAlert( kCFUserNotificationStopAlertLevel, error, CFSTR("OK") );

	CFRelease( error );
}

Dialog::Result DialogDriver_Cocoa::AbortRetryIgnore( CString sMessage, CString sID )
{
	CFStringRef error = CFStringCreateWithCString( NULL, sMessage, kCFStringEncodingASCII );
	CFOptionFlags result = ShowAlert( kCFUserNotificationNoteAlertLevel, error, CFSTR("Retry"), CFSTR("Ignore") );
	Dialog::Result ret;

	CFRelease( error );
	switch( result )
	{
	case kCFUserNotificationDefaultResponse:
		ret = Dialog::retry;
		break;
	case kCFUserNotificationAlternateResponse:
	case kCFUserNotificationCancelResponse:
		ret = Dialog::ignore;
		break;
	default:
		ASSERT(0);
		ret = Dialog::ignore;
	}
    
    return ret;
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
