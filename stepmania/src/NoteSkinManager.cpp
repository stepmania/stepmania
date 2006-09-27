#include "global.h"
#include "NoteSkinManager.h"
#include "RageFileManager.h"
#include "RageLog.h"
#include "RageException.h"
#include "GameState.h"
#include "Game.h"
#include "Style.h"
#include "RageUtil.h"
#include "RageDisplay.h"
#include "arch/Dialog/Dialog.h"
#include "PrefsManager.h"
#include "Foreach.h"
#include "ActorUtil.h"


NoteSkinManager*	NOTESKIN = NULL;	// global object accessable from anywhere in the program


const RString NOTESKINS_DIR = "NoteSkins/";
const RString GLOBAL_BASE_NOTESKIN_DIR = NOTESKINS_DIR + "common/default/";
static map<RString,RString> g_PathCache;

NoteSkinManager::NoteSkinManager()
{
	GAME_BASE_NOTESKIN_NAME.Load( "NoteSkinManager", "GameBaseNoteSkin" );
	m_pCurGame = NULL;

	// Register with Lua.
	{
		Lua *L = LUA->Get();
		lua_pushstring( L, "NOTESKIN" );
		this->PushSelf( L );
		lua_settable( L, LUA_GLOBALSINDEX );
		LUA->Release( L );
	}
}

NoteSkinManager::~NoteSkinManager()
{
	// Unregister with Lua.
	LUA->UnsetGlobal( "NOTESKIN" );
}

void NoteSkinManager::RefreshNoteSkinData( const Game* pGame )
{
	if( m_pCurGame == pGame )
		return;
	m_pCurGame = pGame;

	// clear path cache
	g_PathCache.clear();

	RString sBaseSkinFolder = NOTESKINS_DIR + pGame->m_szName + "/";
	vector<RString> asNoteSkinNames;
	GetDirListing( sBaseSkinFolder + "*", asNoteSkinNames, true );

	StripCvs( asNoteSkinNames );

	m_mapNameToData.clear();
	for( unsigned j=0; j<asNoteSkinNames.size(); j++ )
	{
		RString sName = asNoteSkinNames[j];
		sName.MakeLower();
		m_mapNameToData[sName] = NoteSkinData();
		LoadNoteSkinData( sName, m_mapNameToData[sName] );
	}
}

void NoteSkinManager::LoadNoteSkinData( const RString &sNoteSkinName, NoteSkinData& data_out )
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

void NoteSkinManager::LoadNoteSkinDataRecursive( const RString &sNoteSkinName, NoteSkinData& data_out )
{
	static int depth = 0;
	depth++;
	ASSERT_M( depth < 20, "Circular NoteSkin fallback references detected." );

	RString sDir = GetNoteSkinDir(sNoteSkinName);

	// read global fallback the current NoteSkin (if any)
	RString sFallback;
	IniFile ini;
	ini.ReadFile( sDir+"metrics.ini" );

	if( ini.GetValue("Global","FallbackNoteSkin",sFallback) )
		LoadNoteSkinDataRecursive( sFallback, data_out );

	data_out.metrics.ReadFile( sDir+"metrics.ini" );
	data_out.vsDirSearchOrder.push_front( sDir );

	depth--;
}


void NoteSkinManager::GetNoteSkinNames( vector<RString> &AddTo )
{
	GetNoteSkinNames( GAMESTATE->m_pCurGame, AddTo );
}

void NoteSkinManager::GetNoteSkinNames( const Game* pGame, vector<RString> &AddTo, bool bFilterDefault )
{
	GetAllNoteSkinNamesForGame( pGame, AddTo );

	/* Move "default" to the front if it exists. */
	vector<RString>::iterator iter = find( AddTo.begin(), AddTo.end(), GAME_BASE_NOTESKIN_NAME.GetValue() );
	if( iter != AddTo.end() )
	{
		AddTo.erase( iter );
		if( !bFilterDefault || !PREFSMAN->m_bHideDefaultNoteSkin )
			AddTo.insert( AddTo.begin(), GAME_BASE_NOTESKIN_NAME );
	}
}


bool NoteSkinManager::DoesNoteSkinExist( const RString &sSkinName )
{
	vector<RString> asSkinNames;	
	GetAllNoteSkinNamesForGame( GAMESTATE->m_pCurGame, asSkinNames );
	for( unsigned i=0; i<asSkinNames.size(); i++ )
		if( 0==stricmp(sSkinName, asSkinNames[i]) )
			return true;
	return false;
}

bool NoteSkinManager::DoNoteSkinsExistForGame( const Game *pGame )
{
	vector<RString> asSkinNames;
	GetAllNoteSkinNamesForGame( pGame, asSkinNames );
	return !asSkinNames.empty();
}

RString NoteSkinManager::GetNoteSkinDir( const RString &sSkinName )
{
	RString sGame = m_pCurGame->m_szName;

	return NOTESKINS_DIR + sGame + "/" + sSkinName + "/";
}

void NoteSkinManager::GetAllNoteSkinNamesForGame( const Game *pGame, vector<RString> &AddTo )
{
	if( pGame == m_pCurGame )
	{
		/* Faster: */
		for( map<RString,NoteSkinData>::const_iterator iter = m_mapNameToData.begin();
		     iter != m_mapNameToData.end(); ++iter )
		{
			AddTo.push_back( iter->second.sName );
		}
	}
	else
	{
		RString sBaseSkinFolder = NOTESKINS_DIR + pGame->m_szName + "/";
		GetDirListing( sBaseSkinFolder + "*", AddTo, true );
		StripCvs( AddTo );
	}
}	

RString NoteSkinManager::GetMetric( const RString &sButtonName, const RString &sValue )
{
	ASSERT( !m_sCurrentNoteSkin.empty() );
	RString sNoteSkinName = m_sCurrentNoteSkin;
	sNoteSkinName.MakeLower();
	map<RString,NoteSkinData>::const_iterator it = m_mapNameToData.find(sNoteSkinName);
	ASSERT_M( it != m_mapNameToData.end(), sNoteSkinName );	// this NoteSkin doesn't exist!
	const NoteSkinData& data = it->second;

	RString sReturn;
	if( data.metrics.GetValue( sButtonName, sValue, sReturn ) )
		return sReturn;
	if( !data.metrics.GetValue( "NoteDisplay", sValue, sReturn ) )
		RageException::Throw( "Could not read metric \"%s::%s\" or \"NoteDisplay::%s\" in \"%s\".",
				      sButtonName.c_str(), sValue.c_str(), sValue.c_str(), sNoteSkinName.c_str() );
	return sReturn;
}

int NoteSkinManager::GetMetricI( const RString &sButtonName, const RString &sValueName )
{
	return atoi( GetMetric(sButtonName,sValueName) );
}

float NoteSkinManager::GetMetricF( const RString &sButtonName, const RString &sValueName )
{
	return StringToFloat( GetMetric(sButtonName,sValueName) );
}

bool NoteSkinManager::GetMetricB( const RString &sButtonName, const RString &sValueName )
{
	return atoi( GetMetric(sButtonName,sValueName) ) != 0;
}

apActorCommands NoteSkinManager::GetMetricA( const RString &sButtonName, const RString &sValueName )
{
	return ActorUtil::ParseActorCommands( GetMetric(sButtonName,sValueName) );
}

RString NoteSkinManager::GetPath( const RString &sButtonName, const RString &sElement )
{
try_again:
	const RString CacheString = m_sCurrentNoteSkin + "/" + sButtonName + "/" + sElement;
	map<RString,RString>::iterator it = g_PathCache.find( CacheString );
	if( it != g_PathCache.end() )
		return it->second;

	map<RString,NoteSkinData>::const_iterator iter = m_mapNameToData.find( m_sCurrentNoteSkin );
	ASSERT( iter != m_mapNameToData.end() );
	const NoteSkinData &data = iter->second;

	RString sPath;	// fill this in below
	FOREACHD_CONST( RString, data.vsDirSearchOrder, iter )
	{
		if( sButtonName.empty() )
			sPath = GetPathFromDirAndFile( *iter, sElement );
		else if( *iter == GLOBAL_BASE_NOTESKIN_DIR )
			sPath = GetPathFromDirAndFile( *iter, "Fallback "+sElement );
		else
			sPath = GetPathFromDirAndFile( *iter, sButtonName+" "+sElement );
		if( !sPath.empty() )
			break;	// done searching
	}

	if( sPath.empty() )
	{
		RString message = ssprintf(
			"The NoteSkin element \"%s %s\" could not be found in \"%s\", \"%s\", or \"%s\".", 
			sButtonName.c_str(), sElement.c_str(), 
			GetNoteSkinDir(m_sCurrentNoteSkin).c_str(),
			GetNoteSkinDir(GAME_BASE_NOTESKIN_NAME).c_str(),
			GLOBAL_BASE_NOTESKIN_DIR.c_str() );

		if( Dialog::AbortRetryIgnore(message) == Dialog::retry )
		{
			FILEMAN->FlushDirCache( GetNoteSkinDir(m_sCurrentNoteSkin) );
			FILEMAN->FlushDirCache( GAME_BASE_NOTESKIN_NAME );
			g_PathCache.clear();
			goto try_again;
		}
		
		RageException::Throw( "%s", message.c_str() ); 
	}

	int iLevel = 0;
	while( GetExtension(sPath) == "redir" )
	{
		iLevel++;
		ASSERT_M( iLevel < 100, ssprintf("Infinite recursion while looking up %s - %s", sButtonName.c_str(), sElement.c_str()) );
			
		RString sNewFileName;
		GetFileContents( sPath, sNewFileName, true );
		RString sRealPath;

		FOREACHD_CONST( RString, data.vsDirSearchOrder, iter )
		{
			 sRealPath = GetPathFromDirAndFile( *iter, sNewFileName );
			 if( !sRealPath.empty() )
				 break;	// done searching
		}

		if( sRealPath == "" )
		{
			RString message = ssprintf(
					"NoteSkinManager:  The redirect \"%s\" points to the file \"%s\", which does not exist. "
					"Verify that this redirect is correct.",
					sPath.c_str(), sNewFileName.c_str());

			if( Dialog::AbortRetryIgnore(message) == Dialog::retry )
			{
				FILEMAN->FlushDirCache( GetNoteSkinDir(m_sCurrentNoteSkin) );
				FILEMAN->FlushDirCache( GAME_BASE_NOTESKIN_NAME );
				g_PathCache.clear();
				goto try_again;
			}

			RageException::Throw( "%s", message.c_str() ); 
		}
		
		sPath = sRealPath;
	}

	g_PathCache[CacheString] = sPath;
	return sPath;
}

RString NoteSkinManager::GetPathFromDirAndFile( const RString &sDir, const RString &sFileName )
{
	vector<RString> matches;		// fill this with the possible files

	GetDirListing( sDir+sFileName+"*",		matches, false, true );

	if( matches.empty() )
		return RString();

	if( matches.size() > 1 )
	{
		RString sError = "Multiple files match '"+sDir+sFileName+"'.  Please remove all but one of these files.";
		Dialog::OK( sError );
	}
	
	return matches[0];
}

// lua start
#include "LuaBinding.h"

class LunaNoteSkinManager: public Luna<NoteSkinManager>
{
public:
	static int GetPath( T* p, lua_State *L )		{ lua_pushstring(L, p->GetPath(SArg(1),SArg(2)) ); return 1; }
	static int GetMetricA( T* p, lua_State *L )		{ p->GetMetricA(SArg(1),SArg(2))->PushSelf(L); return 1; }
	DEFINE_METHOD( GetGameBaseNoteSkinName, GAME_BASE_NOTESKIN_NAME.GetValue() )

	LunaNoteSkinManager()
	{
		LUA->Register( Register );

		ADD_METHOD( GetPath );
		ADD_METHOD( GetMetricA );
		ADD_METHOD( GetGameBaseNoteSkinName );
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
