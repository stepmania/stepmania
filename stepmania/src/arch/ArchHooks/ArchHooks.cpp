#include "global.h"
#include "ArchHooks.h"
#include "PrefsManager.h"
#include "RageUtil.h"
#include "RageDisplay.h"	// for IsWindowed()

ArchHooks *HOOKS = NULL;

bool ArchHooks::MessageIsIgnored( CString ID )
{
  vector<CString> list;
  split( PREFSMAN->m_sIgnoredMessageWindows, ",", list );
  for( unsigned i = 0; i < list.size(); ++i )
    if( !ID.CompareNoCase(list[i]) )
      return true;
  return false;
}

void ArchHooks::IgnoreMessage( CString ID )
{
  if( MessageIsIgnored(ID) )
    return;
  vector<CString> list;
  split( PREFSMAN->m_sIgnoredMessageWindows, ",", list );
  list.push_back( ID );
  PREFSMAN->m_sIgnoredMessageWindows = join( ",", list );
  PREFSMAN->SaveGlobalPrefsToDisk();
}

void ArchHooks::MessageBoxError( CString sMessage, CString ID )
{
	if( ID != "" && MessageIsIgnored( ID ) )
		return;

	MessageBoxErrorPrivate( sMessage, ID );
}

void ArchHooks::MessageBoxOK( CString sMessage, CString ID )
{
	if( ID != "" && MessageIsIgnored( ID ) )
		return;

	// don't show MessageBox if windowed
	if( !DISPLAY->IsWindowed() )
		ArchHooks::MessageBoxOKPrivate( sMessage, ID );
	else
		this->MessageBoxOKPrivate( sMessage, ID );	// call derived version
}

ArchHooks::MessageBoxResult ArchHooks::MessageBoxAbortRetryIgnore( CString sMessage, CString ID )
{
	if( ID != "" && MessageIsIgnored( ID ) )
		return ArchHooks::MessageBoxAbortRetryIgnorePrivate( sMessage, ID );

	// don't show MessageBox if windowed
	if( DISPLAY && !DISPLAY->IsWindowed() )
		return ArchHooks::MessageBoxAbortRetryIgnorePrivate( sMessage, ID );
	else
		return this->MessageBoxAbortRetryIgnorePrivate( sMessage, ID );	// call derived version
}

ArchHooks::MessageBoxResult ArchHooks::MessageBoxRetryCancel( CString sMessage, CString ID )
{
	if( ID != "" && MessageIsIgnored( ID ) )
		return ArchHooks::MessageBoxRetryCancelPrivate( sMessage, ID );

	// don't show MessageBox if windowed
	if( !DISPLAY->IsWindowed() )
		return ArchHooks::MessageBoxRetryCancelPrivate( sMessage, ID );
	else
		return this->MessageBoxRetryCancelPrivate( sMessage, ID );	// call derived version
}

/*
 * (c) 2003-2004 Glenn Maynard, Chris Danford
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
