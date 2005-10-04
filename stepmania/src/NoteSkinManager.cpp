#include "global.h"
#include "NoteSkinManager.h"
#include "RageLog.h"
#include "RageException.h"
#include "GameState.h"
#include "Game.h"
#include "StyleInput.h"
#include "Style.h"
#include "RageUtil.h"
#include "RageDisplay.h"
#include "arch/Dialog/Dialog.h"
#include "PrefsManager.h"
#include "Foreach.h"


NoteSkinManager*	NOTESKIN = NULL;	// global object accessable from anywhere in the program


const CString NOTESKINS_DIR = "NoteSkins/";
const CString GAME_BASE_NOTESKIN_NAME = "default";
const CString GLOBAL_BASE_NOTESKIN_DIR = NOTESKINS_DIR + "common/default/";
static map<CString,CString> g_PathCache;

NoteSkinManager::NoteSkinManager()
{
	m_pCurGame = NULL;
}

NoteSkinManager::~NoteSkinManager()
{
}

void NoteSkinManager::RefreshNoteSkinData( const Game* pGame )
{
	if( m_pCurGame == pGame )
		return;
	m_pCurGame = pGame;

	// clear path cache
	g_PathCache.clear();

	CString sBaseSkinFolder = NOTESKINS_DIR + pGame->m_szName + "/";
	CStringArray asNoteSkinNames;
	GetDirListing( sBaseSkinFolder + "*", asNoteSkinNames, true );

	StripCvs( asNoteSkinNames );

	m_mapNameToData.clear();
	for( unsigned j=0; j<asNoteSkinNames.size(); j++ )
	{
		CString sName = asNoteSkinNames[j];
		sName.MakeLower();
		m_mapNameToData[sName] = NoteSkinData();
		LoadNoteSkinData( sName, m_mapNameToData[sName] );
	}
}

void NoteSkinManager::LoadNoteSkinData( const CString &sNoteSkinName, NoteSkinData& data_out )
{
	data_out.sName = sNoteSkinName;
	data_out.metrics.Clear();
	data_out.vsDirSearchOrder.clear();

	/* Load global NoteSkin defaults */
	data_out.metrics.ReadFile( GLOBAL_BASE_NOTESKIN_DIR+"metrics.ini" );
	data_out.vsDirSearchOrder.push_front( GLOBAL_BASE_NOTESKIN_DIR );

	/* Load game NoteSkin defaults */
	data_out.metrics.ReadFile( GetNoteSkinDir(GAME_BASE_NOTESKIN_NAME)+"metrics.ini" );
	data_out.vsDirSearchOrder.push_front( GetNoteSkinDir(GAME_BASE_NOTESKIN_NAME) );

	/* Read the current NoteSkin and all of its fallbacks */
	LoadNoteSkinDataRecursive( sNoteSkinName, data_out );
}

void NoteSkinManager::LoadNoteSkinDataRecursive( const CString &sNoteSkinName, NoteSkinData& data_out )
{
	static int depth = 0;
	depth++;
	ASSERT_M( depth < 20, "Circular NoteSkin fallback references detected." );

	CString sDir = GetNoteSkinDir(sNoteSkinName);

	// read global fallback the current NoteSkin (if any)
	CString sFallback;
	IniFile ini;
	ini.ReadFile( sDir+"metrics.ini" );

	if( ini.GetValue("Global","FallbackNoteSkin",sFallback) )
		LoadNoteSkinDataRecursive( sFallback, data_out );

	data_out.metrics.ReadFile( sDir+"metrics.ini" );
	data_out.vsDirSearchOrder.push_front( sDir );

	depth--;
}


void NoteSkinManager::GetNoteSkinNames( CStringArray &AddTo )
{
	GetNoteSkinNames( GAMESTATE->m_pCurGame, AddTo );
}

void NoteSkinManager::GetNoteSkinNames( const Game* pGame, CStringArray &AddTo, bool bFilterDefault )
{
	if( pGame == m_pCurGame )
	{
		/* Faster: */
		for( map<CString,NoteSkinData>::const_iterator iter = m_mapNameToData.begin();
				iter != m_mapNameToData.end(); ++iter )
		{
			AddTo.push_back( iter->second.sName );
		}
	}
	else
	{
		CString sBaseSkinFolder = NOTESKINS_DIR + pGame->m_szName + "/";
		GetDirListing( sBaseSkinFolder + "*", AddTo, true );
		StripCvs( AddTo );
	}

	/* Move "default" to the front if it exists. */
	{
		CStringArray::iterator iter = find( AddTo.begin(), AddTo.end(), "default" );
		if( iter != AddTo.end() )
		{
			AddTo.erase( iter );
			if( !bFilterDefault || !PREFSMAN->m_bHideDefaultNoteSkin )
				AddTo.insert( AddTo.begin(), "default" );
		}
	}
}


bool NoteSkinManager::DoesNoteSkinExist( const CString &sSkinName )
{
	CStringArray asSkinNames;	
	GetNoteSkinNames( asSkinNames );
	for( unsigned i=0; i<asSkinNames.size(); i++ )
		if( 0==stricmp(sSkinName, asSkinNames[i]) )
			return true;
	return false;
}

CString NoteSkinManager::GetNoteSkinDir( const CString &sSkinName )
{
	CString sGame = m_pCurGame->m_szName;

	return NOTESKINS_DIR + sGame + "/" + sSkinName + "/";
}

CString NoteSkinManager::GetMetric( const CString &sButtonName, const CString &sValue )
{
	ASSERT( !m_sCurrentNoteSkin.empty() );
	CString sNoteSkinName = m_sCurrentNoteSkin;
	sNoteSkinName.MakeLower();
	map<CString,NoteSkinData>::const_iterator it = m_mapNameToData.find(sNoteSkinName);
	ASSERT_M( it != m_mapNameToData.end(), sNoteSkinName );	// this NoteSkin doesn't exist!
	const NoteSkinData& data = it->second;

	CString sReturn;
	if( data.metrics.GetValue( sButtonName, sValue, sReturn ) )
		return sReturn;
	if( !data.metrics.GetValue( "NoteDisplay", sValue, sReturn ) )
	{
		CString sError = ssprintf(
			"Could not read metric '[%s] %s' or '[NoteDisplay] %s' in '%s'",
			sButtonName.c_str(), sValue.c_str(), sValue.c_str(), sNoteSkinName.c_str() );
		RageException::Throw( sError );
	}
	return sReturn;
}

int NoteSkinManager::GetMetricI( const CString &sButtonName, const CString &sValueName )
{
	return atoi( GetMetric(sButtonName,sValueName) );
}

float NoteSkinManager::GetMetricF( const CString &sButtonName, const CString &sValueName )
{
	return strtof( GetMetric(sButtonName,sValueName), NULL );
}

bool NoteSkinManager::GetMetricB( const CString &sButtonName, const CString &sValueName )
{
	return atoi( GetMetric(sButtonName,sValueName) ) != 0;
}

apActorCommands NoteSkinManager::GetMetricA( const CString &sButtonName, const CString &sValueName )
{
	return apActorCommands( new ActorCommands( GetMetric(sButtonName,sValueName) ) );
}

CString NoteSkinManager::GetPath( const CString &sButtonName, const CString &sElement )
{
try_again:
	const CString CacheString = m_sCurrentNoteSkin + "/" + sButtonName + "/" + sElement;
	map<CString,CString>::iterator it = g_PathCache.find( CacheString );
	if( it != g_PathCache.end() )
		return it->second;

	map<CString,NoteSkinData>::const_iterator iter = m_mapNameToData.find( m_sCurrentNoteSkin );
	ASSERT( iter != m_mapNameToData.end() );
	const NoteSkinData &data = iter->second;

	CString sPath;	// fill this in below
	FOREACHD_CONST( CString, data.vsDirSearchOrder, iter )
	{
		 if( *iter == GLOBAL_BASE_NOTESKIN_DIR )
			 sPath = GetPathFromDirAndFile( *iter, "Fallback "+sElement );
		 else
			 sPath = GetPathFromDirAndFile( *iter, sButtonName+" "+sElement );
		 if( !sPath.empty() )
			 break;	// done searching
	}

	if( sPath.empty() )
	{
		CString message = ssprintf(
			"The NoteSkin element '%s %s' could not be found in '%s', '%s', or '%s'.", 
			sButtonName.c_str(), sElement.c_str(), 
			GetNoteSkinDir(m_sCurrentNoteSkin).c_str(),
			GetNoteSkinDir(GAME_BASE_NOTESKIN_NAME).c_str(),
			GLOBAL_BASE_NOTESKIN_DIR.c_str() );

		if( Dialog::AbortRetryIgnore(message) == Dialog::retry )
		{
			FlushDirCache();
			g_PathCache.clear();
			goto try_again;
		}
		
		RageException::Throw( message ); 
	}

	while( GetExtension(sPath) == "redir" )
	{
		CString sNewFileName;
		GetFileContents( sPath, sNewFileName, true );
		CString sRealPath;

		FOREACHD_CONST( CString, data.vsDirSearchOrder, iter )
		{
			 sRealPath = GetPathFromDirAndFile( *iter, sNewFileName );
			 if( !sRealPath.empty() )
				 break;	// done searching
		}

		if( sRealPath == "" )
		{
			CString message = ssprintf(
					"NoteSkinManager:  The redirect '%s' points to the file '%s', which does not exist. "
					"Verify that this redirect is correct.",
					sPath.c_str(), sNewFileName.c_str());

			if( Dialog::AbortRetryIgnore(message) == Dialog::retry )
			{
				FlushDirCache();
				g_PathCache.clear();
				goto try_again;
			}

			RageException::Throw( message ); 
		}
		
		sPath = sRealPath;
	}

	g_PathCache[CacheString] = sPath;
	return sPath;
}

CString NoteSkinManager::GetPathFromDirAndFile( const CString &sDir, const CString &sFileName )
{
	CStringArray matches;		// fill this with the possible files

	GetDirListing( sDir+sFileName+"*",		matches, false, true );

	if( matches.empty() )
		return CString();

	if( matches.size() > 1 )
	{
		CString sError = "Multiple files match '"+sDir+sFileName+"'.  Please remove all but one of these files.";
		Dialog::OK( sError );
	}
	
	return matches[0];
}

// lua start
#include "LuaBinding.h"

class LunaNoteSkinManager: public Luna<NoteSkinManager>
{
public:
	LunaNoteSkinManager() { LUA->Register( Register ); }

	static int GetPath( T* p, lua_State *L )		{ lua_pushstring(L, p->GetPath(SArg(1),SArg(2)) ); return 1; }

	static void Register(lua_State *L)
	{
		ADD_METHOD( GetPath );

		Luna<T>::Register( L );

		// Add global singleton if constructed already.  If it's not constructed yet,
		// then we'll register it later when we reinit Lua just before 
		// initializing the display.
		if( NOTESKIN )
		{
			lua_pushstring(L, "NOTESKIN");
			NOTESKIN->PushSelf( L );
			lua_settable(L, LUA_GLOBALSINDEX);
		}
	}
};

LUA_REGISTER_CLASS( NoteSkinManager )
// lua end

/*
 * (c) 2003-2004 Chris Danford
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
