#include "stdafx.h"
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
#include "IniFile.h"
#include "StyleInput.h"
#include "StyleDef.h"
#include "RageUtil.h"
#include "GameManager.h"


NoteSkinManager*	NOTESKIN = NULL;	// global object accessable from anywhere in the program


const CString BASE_NOTESKIN_NAME = "default";
const CString NOTESKINS_DIR  = "NoteSkins/";

NoteSkinManager::NoteSkinManager()
{
	m_pIniMetrics = new IniFile;

	m_sCurNoteSkinName = BASE_NOTESKIN_NAME;	// Use the base theme for now.  It's up to PrefsManager to change this.
}

NoteSkinManager::~NoteSkinManager()
{
	delete m_pIniMetrics;
}

void NoteSkinManager::GetNoteSkinNames( Game game, CStringArray &AddTo ) const
{
	GameDef* pGameDef = GAMEMAN->GetGameDefForGame( game );

	CString sBaseSkinFolder = NOTESKINS_DIR + pGameDef->m_szName + "\\";
	GetDirListing( sBaseSkinFolder + "*", AddTo, true );

	// strip out "CVS"
	for( int i=AddTo.size()-1; i>=0; i-- )
		if( 0 == stricmp("cvs", AddTo[i]) )
			AddTo.erase( AddTo.begin()+i, AddTo.begin()+i+1 );
}

void NoteSkinManager::GetNoteSkinNames( CStringArray &AddTo ) const
{
	GetNoteSkinNames( GAMESTATE->m_CurGame, AddTo );
}

bool NoteSkinManager::DoesNoteSkinExist( CString sSkinName ) const
{
	CStringArray asSkinNames;	
	GetNoteSkinNames( asSkinNames );
	for( unsigned i=0; i<asSkinNames.size(); i++ )
		if( 0==stricmp(sSkinName, asSkinNames[i]) )
			return true;
	return false;
}

void NoteSkinManager::SwitchNoteSkin( CString sNewNoteSkin )
{
	if( sNewNoteSkin == ""  ||  !DoesNoteSkinExist(sNewNoteSkin) )
	{
		CStringArray as;
		GetNoteSkinNames( as );
		ASSERT( !as.empty() );

		/* Prefer "default" if it exists. */
		sNewNoteSkin = as[0];
		for(unsigned i = 0; i < as.size(); ++i)
			if(!as[i].CompareNoCase("default")) sNewNoteSkin = "default";
	}

	m_sCurNoteSkinName = sNewNoteSkin;

	m_pIniMetrics->Reset();
	m_pIniMetrics->SetPath( GetNoteSkinDir(BASE_NOTESKIN_NAME)+"metrics.ini" );
	m_pIniMetrics->ReadFile();
	m_pIniMetrics->SetPath( GetNoteSkinDir(m_sCurNoteSkinName)+"metrics.ini" );
	m_pIniMetrics->ReadFile();
}

CString NoteSkinManager::GetNoteSkinDir( CString sSkinName )
{
	const GameDef* pGameDef = GAMESTATE->GetCurrentGameDef();

	return NOTESKINS_DIR + ssprintf("%s/%s/", pGameDef->m_szName, sSkinName.GetString());
}

CString NoteSkinManager::GetMetric( CString sButtonName, CString sValue )	// looks in GAMESTATE for the current Style
{
	CString sReturn;
	if( m_pIniMetrics->GetValue( sButtonName, sValue, sReturn ) )
		return sReturn;
	if( !m_pIniMetrics->GetValue( "NoteDisplay", sValue, sReturn ) )
		RageException::Throw( "Could not read metric '%s - %s' or 'NoteDisplay - %s'",
			sButtonName.GetString(), sValue.GetString(), sValue.GetString() );
	return sReturn;
}

int NoteSkinManager::GetMetricI( CString sButtonName, CString sValueName )
{
	return atoi( GetMetric(sButtonName,sValueName) );
}

float NoteSkinManager::GetMetricF( CString sButtonName, CString sValueName )
{
	return (float)atof( GetMetric(sButtonName,sValueName) );
}

bool NoteSkinManager::GetMetricB( CString sButtonName, CString sValueName )
{
	return atoi( GetMetric(sButtonName,sValueName) ) != 0;
}

RageColor NoteSkinManager::GetMetricC( CString sButtonName, CString sValueName )
{
	float r=1,b=1,g=1,a=1;	// initialize in case sscanf fails
	CString sValue = GetMetric(sButtonName,sValueName);
	char szValue[40];
	strncpy( szValue, sValue, 39 );
	int result = sscanf( szValue, "%f,%f,%f,%f", &r, &g, &b, &a );
	if( result != 4 )
	{
		LOG->Warn( "The color value '%s' for theme metric '%s : %s' is invalid.", szValue, sButtonName.GetString(), sValueName.GetString() );
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

CString NoteSkinManager::GetPathTo( int col, CString sFileName )	// looks in GAMESTATE for the current Style
{
	CString sButtonName = ColToButtonName(col);

	CString sCurNoteSkinName = m_sCurNoteSkinName;
	if( GAMESTATE->m_bEditing )
		sCurNoteSkinName = "note";

	CString ret = GetPathTo( m_sCurNoteSkinName, sButtonName, sFileName );
	if( !ret.empty() )	// we found something
		return ret;
	ret = GetPathTo( BASE_NOTESKIN_NAME, sButtonName, sFileName);

	if( ret.empty() )
		RageException::Throw( "The NoteSkin element '%s %s' could not be found in '%s' or '%s'.", 
		sButtonName.GetString(), sFileName.GetString(), 
		GetNoteSkinDir(m_sCurNoteSkinName).GetString(),
		GetNoteSkinDir(BASE_NOTESKIN_NAME).GetString() );

	return ret;
}

CString NoteSkinManager::GetPathTo( CString sSkinName, CString sButtonName, CString sElementName )	// looks in GAMESTATE for the current Style
{
	const CString sDir = GetNoteSkinDir( sSkinName );

	CStringArray arrayPossibleFileNames;		// fill this with the possible files

	GetDirListing( ssprintf("%s%s %s*.sprite", sDir.GetString(), sButtonName.GetString(), sElementName.GetString()), arrayPossibleFileNames, false, true );
	GetDirListing( ssprintf("%s%s %s*.png",    sDir.GetString(), sButtonName.GetString(), sElementName.GetString()), arrayPossibleFileNames, false, true );
	GetDirListing( ssprintf("%s%s %s*.jpg",    sDir.GetString(), sButtonName.GetString(), sElementName.GetString()), arrayPossibleFileNames, false, true );
	GetDirListing( ssprintf("%s%s %s*.bmp",    sDir.GetString(), sButtonName.GetString(), sElementName.GetString()), arrayPossibleFileNames, false, true );
	GetDirListing( ssprintf("%s%s %s*.gif",    sDir.GetString(), sButtonName.GetString(), sElementName.GetString()), arrayPossibleFileNames, false, true );
	GetDirListing( ssprintf("%s%s %s*",        sDir.GetString(), sButtonName.GetString(), sElementName.GetString()), arrayPossibleFileNames, false, true );

	if( arrayPossibleFileNames.empty() )
		return "";
	else
		return DerefRedir(arrayPossibleFileNames[0]);
}
