#include "global.h"
#include "RageUtil.h"
#include "DialogDriver_Cocoa.h"
#include "RageThreads.h"
#include "ProductInfo.h"
#include "InputFilter.h"
#include <Carbon/Carbon.h>

static CFOptionFlags ShowAlert( CFOptionFlags flags, const RString& sMessage, CFStringRef OK,
				CFStringRef alt = NULL, CFStringRef other = NULL)
{
	CFOptionFlags result;
	CFStringRef text = CFStringCreateWithCString( NULL, sMessage, kCFStringEncodingUTF8 );
	
	CFUserNotificationDisplayAlert( 0.0, flags, NULL, NULL, NULL, CFSTR(PRODUCT_FAMILY),
					text, OK, alt, other, &result );
	CFRelease( text );

	// Flush all input that's accumulated while the dialog box was up.
	if( INPUTFILTER )
	{
		vector<InputEvent> dummy;
		INPUTFILTER->Reset();
		INPUTFILTER->GetInputEvents( dummy );
	}

	return result;
}

#define LSTRING(b,x) CFBundleCopyLocalizedString( (b), CFSTR(x), NULL, CFSTR("Localizable") )

void DialogDriver_Cocoa::OK( RString sMessage, RString sID )
{
	CFBundleRef bundle = CFBundleGetMainBundle();
	CFStringRef sDSA = LSTRING( bundle, "Don't show again" );
	CFOptionFlags result = ShowAlert( kCFUserNotificationNoteAlertLevel, sMessage, CFSTR("OK"), sDSA );

	CFRelease( sDSA );
	if( result == kCFUserNotificationAlternateResponse )
		Dialog::IgnoreMessage( sID );
}

void DialogDriver_Cocoa::Error( RString sError, RString sID )
{
	ShowAlert( kCFUserNotificationStopAlertLevel, sError, CFSTR("OK") );
}

Dialog::Result DialogDriver_Cocoa::AbortRetryIgnore( RString sMessage, RString sID )
{
	CFBundleRef bundle = CFBundleGetMainBundle();
	CFStringRef sIgnore = LSTRING( bundle, "Ignore" );
	CFStringRef sRetry = LSTRING( bundle, "Retry" );
	CFStringRef sAbort = LSTRING( bundle, "Abort" );
	CFOptionFlags result = ShowAlert( kCFUserNotificationNoteAlertLevel, sMessage, sIgnore, sRetry, sAbort );

	CFRelease( sIgnore );
	CFRelease( sRetry );
	CFRelease( sAbort );
	switch( result )
	{
	case kCFUserNotificationDefaultResponse:
		Dialog::IgnoreMessage( sID );
		return Dialog::ignore;
	case kCFUserNotificationAlternateResponse:
		return Dialog::retry;
	case kCFUserNotificationOtherResponse:
	case kCFUserNotificationCancelResponse:
		return Dialog::abort;
	default:
		FAIL_M( ssprintf("Invalid response: %d.", int(result)) );
	}
}

Dialog::Result DialogDriver_Cocoa::AbortRetry( RString sMessage, RString sID )
{
	CFBundleRef bundle = CFBundleGetMainBundle();
	CFStringRef sRetry = LSTRING( bundle, "Retry" );
	CFStringRef sAbort = LSTRING( bundle, "Abort" );
	CFOptionFlags result = ShowAlert( kCFUserNotificationNoteAlertLevel, sMessage, sRetry, sAbort );

	CFRelease( sRetry );
	CFRelease( sAbort );
	switch( result )
	{
	case kCFUserNotificationDefaultResponse:
	case kCFUserNotificationCancelResponse:
		return Dialog::abort;
	case kCFUserNotificationAlternateResponse:
		return Dialog::retry;
	default:
		FAIL_M( ssprintf("Invalid response: %d.", int(result)) );
	}
}

/*
 * (c) 2003-2006 Steve Checkoway
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
