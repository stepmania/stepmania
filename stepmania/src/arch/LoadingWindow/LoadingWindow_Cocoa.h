/* LoadingWindow_Cocoa - Loading window for OSX */

#ifndef LOADING_WINDOW_COCOA_H
#define LOADING_WINDOW_COCOA_H

#include "LoadingWindow.h"
#include "RageFile.h"

extern "C"
{
	extern void MakeNewCocoaWindow( const void *data, unsigned length );
	extern void DisposeOfCocoaWindow();
	extern void SetCocoaWindowText( const char *s );
}

class LoadingWindow_Cocoa : public LoadingWindow
{
public:
	LoadingWindow_Cocoa()
	{
		RageFile f;
		CString data;
		
		if( f.Open("Data/splash.png") )
			f.Read( data );
		MakeNewCocoaWindow( data.data(), data.length() );
	}
	~LoadingWindow_Cocoa() { DisposeOfCocoaWindow(); }

	void Paint() { } /* Not needed but pure virtual*/
	void SetText( CString str ) { SetCocoaWindowText( str ); }
};
#define USE_LOADING_WINDOW_COCOA

#endif

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
