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

void ArchHooks::MessageBoxOK( CString sMessage, CString ID )
{
	// don't show MessageBox if windowed
	if( !DISPLAY->IsWindowed() )
		ArchHooks::MessageBoxOKPrivate( sMessage, ID );
	else
		this->MessageBoxOKPrivate( sMessage, ID );	// call derived version
}

ArchHooks::MessageBoxResult ArchHooks::MessageBoxAbortRetryIgnore( CString sMessage, CString ID )
{
	// don't show MessageBox if windowed
	if( !DISPLAY->IsWindowed() )
		return ArchHooks::MessageBoxAbortRetryIgnorePrivate( sMessage, ID );
	else
		return this->MessageBoxAbortRetryIgnorePrivate( sMessage, ID );	// call derived version
}

ArchHooks::MessageBoxResult ArchHooks::MessageBoxRetryCancel( CString sMessage, CString ID )
{
	// don't show MessageBox if windowed
	if( !DISPLAY->IsWindowed() )
		return ArchHooks::MessageBoxRetryCancelPrivate( sMessage, ID );
	else
		return this->MessageBoxRetryCancelPrivate( sMessage, ID );	// call derived version
}
