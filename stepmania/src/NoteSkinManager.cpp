#include "global.h"
/*
-----------------------------------------------------------------------------
 Class: NoteSkinManager

 Desc: See header.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/

#include "NoteSkinManager.h"
#include "RageLog.h"
#include "RageException.h"
#include "GameState.h"
#include "GameDef.h"
#include "StyleInput.h"
#include "StyleDef.h"
#include "RageUtil.h"
#include "GameManager.h"
#include "arch/arch.h"


NoteSkinManager*	NOTESKIN = NULL;	// global object accessable from anywhere in the program


const CString NOTESKINS_DIR = BASE_PATH "NoteSkins" SLASH;
const CString GAME_BASE_NOTESKIN_NAME = "default";
const CString GLOBAL_BASE_NOTESKIN_DIR = NOTESKINS_DIR + "common" SLASH "default" SLASH;

NoteSkinManager::NoteSkinManager()
{
}

NoteSkinManager::~NoteSkinManager()
{
}

void NoteSkinManager::RefreshNoteSkinData( Game game )
{
	GameDef* pGameDef = GAMEMAN->GetGameDefForGame( game );

	CString sBaseSkinFolder = NOTESKINS_DIR + pGameDef->m_szName + SLASH;
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

	///* Read only the default keys from the default noteskin. */
	//IniFile defaults;
	//defaults.SetPath( GLOBAL_BASE_NOTESKIN_DIR+"metrics.ini" );
	//defaults.ReadFile();
	//defaults.SetPath( GetNoteSkinDir(BASE_NOTESKIN_NAME)+"metrics.ini" );
	//defaults.ReadFile();

	// Why only use NoteDisplay?  Now, a few different classes take 
	// metrics from the NoteSkin.  -Chris
	//const IniFile::key *def = defaults.GetKey("NoteDisplay");
	//if(def)
	//	data_out.metrics.SetValue("NoteDisplay", *def);


	/* Load global NoteSkin defaults */
	data_out.metrics.SetPath( GLOBAL_BASE_NOTESKIN_DIR+"metrics.ini" );
	data_out.metrics.ReadFile();

	/* Load game NoteSkin defaults */
	data_out.metrics.SetPath( GetNoteSkinDir(GAME_BASE_NOTESKIN_NAME)+"metrics.ini" );
	data_out.metrics.ReadFile();

	/* Read the active NoteSkin */
	data_out.metrics.SetPath( GetNoteSkinDir(sNoteSkinName)+"metrics.ini" );
	data_out.metrics.ReadFile();	
}


void NoteSkinManager::GetNoteSkinNames( CStringArray &AddTo )
{
	GetNoteSkinNames( GAMESTATE->m_CurGame, AddTo );
}

void NoteSkinManager::GetNoteSkinNames( Game game, CStringArray &AddTo )
{
	RefreshNoteSkinData( game );

	for( map<CString,NoteSkinData>::const_iterator iter = m_mapNameToData.begin(); 
		iter != m_mapNameToData.end();
		++iter )
	{
		AddTo.push_back( iter->second.sName );
	}

	/* Put the note skins back. */
	RefreshNoteSkinData( GAMESTATE->m_CurGame );
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
	CString sGame = GAMESTATE->GetCurrentGameDef()->m_szName;

	return NOTESKINS_DIR + sGame + SLASH + sSkinName + SLASH;
}

CString NoteSkinManager::GetMetric( PlayerNumber pn, CString sButtonName, CString sValue )	// looks in GAMESTATE for the current Style
{
	CString sReturn;
	CString sNoteSkinName = GAMESTATE->m_PlayerOptions[pn].m_sNoteSkin;
	sNoteSkinName.MakeLower();
	NoteSkinData& data = m_mapNameToData[sNoteSkinName];
	if( data.metrics.GetValue( sButtonName, sValue, sReturn ) )
		return sReturn;
	if( !data.metrics.GetValue( "NoteDisplay", sValue, sReturn ) )
		RageException::Throw( "Could not read metric '[%s] %s' or '[NoteDisplay] %s' in '%s'",
			sButtonName.c_str(), sValue.c_str(), sValue.c_str(), sNoteSkinName.c_str() );
	return sReturn;
}

int NoteSkinManager::GetMetricI( PlayerNumber pn, CString sButtonName, CString sValueName )
{
	return atoi( GetMetric(pn,sButtonName,sValueName) );
}

float NoteSkinManager::GetMetricF( PlayerNumber pn, CString sButtonName, CString sValueName )
{
	return (float)atof( GetMetric(pn,sButtonName,sValueName) );
}

bool NoteSkinManager::GetMetricB( PlayerNumber pn, CString sButtonName, CString sValueName )
{
	return atoi( GetMetric(pn,sButtonName,sValueName) ) != 0;
}

RageColor NoteSkinManager::GetMetricC( PlayerNumber pn, CString sButtonName, CString sValueName )
{
	float r=1,b=1,g=1,a=1;	// initialize in case sscanf fails
	CString sValue = GetMetric(pn,sButtonName,sValueName);
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
	const StyleDef* pStyleDef = GAMESTATE->GetCurrentStyleDef();
	const GameDef* pGameDef = GAMESTATE->GetCurrentGameDef();

	StyleInput SI( PLAYER_1, col );
	GameInput GI = pStyleDef->StyleInputToGameInput( SI );
	return pGameDef->m_szButtonNames[GI.button];
}

CString NoteSkinManager::GetPathTo( PlayerNumber pn, int col, CString sFileName )
{
	CString sButtonName = ColToButtonName(col);

	return GetPathTo( pn, sButtonName, sFileName );
}

CString NoteSkinManager::GetPathTo( PlayerNumber pn, CString sButtonName, CString sFileName )	// looks in GAMESTATE for the current Style
{
	// search active NoteSkin
	CString sNoteSkinName = GAMESTATE->m_PlayerOptions[pn].m_sNoteSkin;
	sNoteSkinName.MakeLower();

	CString ret = GetPathTo( sNoteSkinName, sButtonName, sFileName );
	if( !ret.empty() )	// we found something
		return ret;

	// Search game default NoteSkin
	ret = GetPathTo( GAME_BASE_NOTESKIN_NAME, sButtonName, sFileName);
	if( !ret.empty() )	// we found something
		return ret;

	// Search global default NoteSkin
	ret = GetPathTo( GAME_BASE_NOTESKIN_NAME, "Fallback", sFileName, true );
	if( !ret.empty() )	// we found something
		return ret;

	RageException::Throw( "The NoteSkin element '%s %s' could not be found in '%s', '%s', or '%s'.", 
		sButtonName.c_str(), sFileName.c_str(), 
		GetNoteSkinDir(sNoteSkinName).c_str(),
		GetNoteSkinDir(GAME_BASE_NOTESKIN_NAME).c_str(),
		GAME_BASE_NOTESKIN_NAME.c_str() );

	return ret;
}

CString NoteSkinManager::GetPathTo( CString sSkinName, CString sButtonName, CString sElementName, bool bGlobalDefault )	// looks in GAMESTATE for the current Style
{
	const CString sDir = bGlobalDefault ? GLOBAL_BASE_NOTESKIN_DIR : GetNoteSkinDir( sSkinName );

	CStringArray arrayPossibleFileNames;		// fill this with the possible files

	GetDirListing( ssprintf("%s%s %s*.txt",	   sDir.c_str(), sButtonName.c_str(), sElementName.c_str()), arrayPossibleFileNames, false, true );
	GetDirListing( ssprintf("%s%s %s*.sprite", sDir.c_str(), sButtonName.c_str(), sElementName.c_str()), arrayPossibleFileNames, false, true );
	GetDirListing( ssprintf("%s%s %s*.png",    sDir.c_str(), sButtonName.c_str(), sElementName.c_str()), arrayPossibleFileNames, false, true );
	GetDirListing( ssprintf("%s%s %s*.jpg",    sDir.c_str(), sButtonName.c_str(), sElementName.c_str()), arrayPossibleFileNames, false, true );
	GetDirListing( ssprintf("%s%s %s*.bmp",    sDir.c_str(), sButtonName.c_str(), sElementName.c_str()), arrayPossibleFileNames, false, true );
	GetDirListing( ssprintf("%s%s %s*.gif",    sDir.c_str(), sButtonName.c_str(), sElementName.c_str()), arrayPossibleFileNames, false, true );
	GetDirListing( ssprintf("%s%s %s*",        sDir.c_str(), sButtonName.c_str(), sElementName.c_str()), arrayPossibleFileNames, false, true );

	if( arrayPossibleFileNames.empty() )
		return "";
	else
		return DerefRedir(arrayPossibleFileNames[0]);
}
