#include "global.h"
#include "ArchHooks.h"
#include "PrefsManager.h"
#include "RageUtil.h"

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
