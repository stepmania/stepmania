#include "global.h"
#include "NoteSkinManager.h"
#include "RageLog.h"
#include "RageException.h"
#include "GameState.h"
#include "Game.h"
#include "StyleInput.h"
#include "Style.h"
#include "RageUtil.h"
#include "GameManager.h"
#include "arch/arch.h"
#include "RageDisplay.h"
#include "arch/Dialog/Dialog.h"
#include "PrefsManager.h"


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

void NoteSkinManager::RefreshNoteSkinData( const Game* game )
{
	/* Reload even if we don't need to, so exiting out of the menus refreshes the note
	 * skin list (so you don't have to restart to see new noteskins). */
	m_pCurGame = GAMESTATE->m_pCurGame;

	const Game* pGame = game;

	// clear path cache
	g_PathCache.clear();

	CString sBaseSkinFolder = NOTESKINS_DIR + pGame->m_szName + "/";
	CStringArray asNoteSkinNames;
	GetDirListing( sBaseSkinFolder + "*", asNoteSkinNames, true );

	// strip out "CVS"
	for( int i=asNoteSkinNames.size()-1; i>=0; i-- )
		if( 0 == stricmp("cvs", asNoteSkinNames[i]) )
			asNoteSkinNames.erase( asNoteSkinNames.begin()+i, asNoteSkinNames.begin()+i+1 );

	m_mapNameToData.clear();
	for( unsigned j=0; j<asNoteSkinNames.size(); j++ )
	{
		CString sName = asNoteSkinNames[j];
		sName.MakeLower();
		m_mapNameToData[sName] = NoteSkinData();
		LoadNoteSkinData( sName, m_mapNameToData[sName] );
	}
}

void NoteSkinManager::LoadNoteSkinData( CString sNoteSkinName, NoteSkinData& data_out )
{
	data_out.sName = sNoteSkinName;
	data_out.metrics.Reset();

	/* Load global NoteSkin defaults */
	data_out.metrics.ReadFile( GLOBAL_BASE_NOTESKIN_DIR+"metrics.ini" );

	/* Load game NoteSkin defaults */
	data_out.metrics.ReadFile( GetNoteSkinDir(GAME_BASE_NOTESKIN_NAME)+"metrics.ini" );

	/* Read the active NoteSkin */
	data_out.metrics.ReadFile( GetNoteSkinDir(sNoteSkinName)+"metrics.ini" );	
}


void NoteSkinManager::GetNoteSkinNames( CStringArray &AddTo )
{
	/* If the skin data for the current game isn't already load it, load it now. */
	if( m_pCurGame != GAMESTATE->m_pCurGame )
		RefreshNoteSkinData( GAMESTATE->m_pCurGame );

	/* Don't call GetNoteSkinNames below, since we don't want to call RefreshNoteSkinData; it's
	 * slow. */
	for( map<CString,NoteSkinData>::const_iterator iter = m_mapNameToData.begin(); 
		iter != m_mapNameToData.end(); ++iter )
	{
		AddTo.push_back( iter->second.sName );
	}

	/* Move "default" to the front if it exists. */
	{
		CStringArray::iterator iter = find( AddTo.begin(), AddTo.end(), "default" );
		if( iter != AddTo.end() )
		{
			AddTo.erase( iter );
			if( !PREFSMAN->m_bHideDefaultNoteSkin )
				AddTo.insert( AddTo.begin(), "default" );
		}
	}
}

void NoteSkinManager::GetNoteSkinNames( const Game* game, CStringArray &AddTo )
{
	RefreshNoteSkinData( game );

	for( map<CString,NoteSkinData>::const_iterator iter = m_mapNameToData.begin(); 
		iter != m_mapNameToData.end(); ++iter )
	{
		AddTo.push_back( iter->second.sName );
	}

	/* Put the note skins back. */
	RefreshNoteSkinData( GAMESTATE->m_pCurGame );
}


bool NoteSkinManager::DoesNoteSkinExist( CString sSkinName )
{
	CStringArray asSkinNames;	
	GetNoteSkinNames( asSkinNames );
	for( unsigned i=0; i<asSkinNames.size(); i++ )
		if( 0==stricmp(sSkinName, asSkinNames[i]) )
			return true;
	return false;
}

CString NoteSkinManager::GetNoteSkinDir( CString sSkinName )
{
	CString sGame = GAMESTATE->GetCurrentGame()->m_szName;

	return NOTESKINS_DIR + sGame + "/" + sSkinName + "/";
}

CString NoteSkinManager::GetMetric( CString sNoteSkinName, CString sButtonName, CString sValue )
{
	sNoteSkinName.MakeLower();
	map<CString,NoteSkinData>::const_iterator it = m_mapNameToData.find(sNoteSkinName);
	ASSERT_M( it != m_mapNameToData.end(), sNoteSkinName );	// this NoteSkin doesn't exist!
	const NoteSkinData& data = it->second;

	CString sReturn;
	if( data.metrics.GetValue( sButtonName, sValue, sReturn ) )
		return sReturn;
	if( !data.metrics.GetValue( "NoteDisplay", sValue, sReturn ) )
		RageException::Throw( "Could not read metric '[%s] %s' or '[NoteDisplay] %s' in '%s'",
			sButtonName.c_str(), sValue.c_str(), sValue.c_str(), sNoteSkinName.c_str() );
	return sReturn;
}

int NoteSkinManager::GetMetricI( CString sNoteSkinName, CString sButtonName, CString sValueName )
{
	return atoi( GetMetric(sNoteSkinName,sButtonName,sValueName) );
}

float NoteSkinManager::GetMetricF( CString sNoteSkinName, CString sButtonName, CString sValueName )
{
	return (float)atof( GetMetric(sNoteSkinName,sButtonName,sValueName) );
}

bool NoteSkinManager::GetMetricB( CString sNoteSkinName, CString sButtonName, CString sValueName )
{
	return atoi( GetMetric(sNoteSkinName,sButtonName,sValueName) ) != 0;
}

RageColor NoteSkinManager::GetMetricC( CString sNoteSkinName, CString sButtonName, CString sValueName )
{
	float r=1,b=1,g=1,a=1;	// initialize in case sscanf fails
	CString sValue = GetMetric(sNoteSkinName,sButtonName,sValueName);
	char szValue[40];
	strncpy( szValue, sValue, 39 );
	int result = sscanf( szValue, "%f,%f,%f,%f", &r, &g, &b, &a );
	if( result != 4 )
	{
		LOG->Warn( "The color value '%s' for theme metric '%s : %s' is invalid.", szValue, sButtonName.c_str(), sValueName.c_str() );
		ASSERT(0);
	}

	return RageColor(r,g,b,a);
}


CString NoteSkinManager::ColToButtonName(int col)
{
	const Style* pStyle = GAMESTATE->GetCurrentStyle();
	const Game* pGame = GAMESTATE->GetCurrentGame();

	StyleInput SI( PLAYER_1, col );
	GameInput GI = pStyle->StyleInputToGameInput( SI );
	return pGame->m_szButtonNames[GI.button];
}

CString NoteSkinManager::GetPathToFromPlayerAndCol( PlayerNumber pn, int col, CString sFileName, bool bOptional )
{
	CString sButtonName = ColToButtonName(col);

	return GetPathToFromPlayerAndButton( pn, sButtonName, sFileName, bOptional );
}


CString NoteSkinManager::GetPathToFromPlayerAndButton( PlayerNumber pn, CString sButtonName, CString sElement, bool bOptional )	// looks in GAMESTATE for the current Style
{
	// search active NoteSkin
	CString sNoteSkinName = GAMESTATE->m_PlayerOptions[pn].m_sNoteSkin;
	sNoteSkinName.MakeLower();
	return GetPathToFromNoteSkinAndButton( sNoteSkinName, sButtonName, sElement, bOptional );
}


CString NoteSkinManager::GetPathToFromNoteSkinAndButton( CString NoteSkin, CString sButtonName, CString sElement, bool bOptional )
{
	const CString CacheString = NoteSkin + "/" + sButtonName + "/" + sElement;
	map<CString,CString>::iterator it = g_PathCache.find( CacheString );
	if( it != g_PathCache.end() )
		return it->second;

	CString sPath;
	if( sPath.empty() )
		sPath = GetPathToFromDir( GetNoteSkinDir(NoteSkin), sButtonName+" "+sElement );
	if( sPath.empty() ) // Search game default NoteSkin
		sPath = GetPathToFromDir( GetNoteSkinDir(GAME_BASE_NOTESKIN_NAME), sButtonName+" "+sElement );
	if( sPath.empty() ) // Search global default NoteSkin
		sPath = GetPathToFromDir( GLOBAL_BASE_NOTESKIN_DIR, "Fallback "+sElement );

	if( sPath.empty() ) // Search global default NoteSkin
	{
		if( bOptional )
		{
			g_PathCache[CacheString] = sPath;
			return sPath;
		}

		RageException::Throw( "The NoteSkin element '%s %s' could not be found in '%s', '%s', or '%s'.", 
			sButtonName.c_str(), sElement.c_str(), 
			GetNoteSkinDir(NoteSkin).c_str(),
			GetNoteSkinDir(GAME_BASE_NOTESKIN_NAME).c_str(),
			GLOBAL_BASE_NOTESKIN_DIR.c_str() );
	}

	while( GetExtension(sPath) == "redir" )
	{
		CString sNewFileName = GetRedirContents(sPath);
		CString sRealPath;
		if( sRealPath == "" )
			sRealPath = GetPathToFromDir( GetNoteSkinDir(NoteSkin), sNewFileName );
		if( sRealPath == "" )
			sRealPath = GetPathToFromDir( GetNoteSkinDir(GAME_BASE_NOTESKIN_NAME), sNewFileName );
		if( sRealPath == "" )
			sRealPath = GetPathToFromDir( GLOBAL_BASE_NOTESKIN_DIR, sNewFileName );

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

				continue;
			}

			RageException::Throw( "%s", message.c_str() ); 
		}
		
		sPath = sRealPath;
	}

	g_PathCache[CacheString] = sPath;
	return sPath;
}

CString NoteSkinManager::GetPathToFromDir( CString sDir, CString sFileName )
{
	CStringArray matches;		// fill this with the possible files

	GetDirListing( sDir+sFileName+"*.redir",	matches, false, true );
	GetDirListing( sDir+sFileName+"*.actor",	matches, false, true );
	GetDirListing( sDir+sFileName+"*.model",	matches, false, true );
	GetDirListing( sDir+sFileName+"*.txt",		matches, false, true );
	GetDirListing( sDir+sFileName+"*.sprite",	matches, false, true );
	GetDirListing( sDir+sFileName+"*.png",		matches, false, true );
	GetDirListing( sDir+sFileName+"*.jpg",		matches, false, true );
	GetDirListing( sDir+sFileName+"*.bmp",		matches, false, true );
	GetDirListing( sDir+sFileName+"*.gif",		matches, false, true );
	GetDirListing( sDir+sFileName+"*",			matches, false, true );

	if( matches.empty() )
		return "";

	if( matches.size() > 1 )
	{
		CString sError = "Multiple files match '"+sDir+sFileName+"'.  Please remove all but one of these files.";
		Dialog::OK( sError );
	}
	
	return matches[0];
}

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
