#include "stdafx.h"
/*
-----------------------------------------------------------------------------
 Class: GameDef

 Desc: See header.

 Copyright (c) 2001-2002 by the persons listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/

#include "GameDef.h"
#include "RageHelper.h"
#include "RageUtil.h"
#include "ErrorCatcher/ErrorCatcher.h"
#include "IniFile.h"
#include "StyleDef.h"


GameDef::GameDef( CString sGameDir )
{
	HELPER.Log( "GameDef::GameDef( '%s )", sGameDir );

	sGameDir.TrimRight( "/\\" );	// trim off the trailing slash if any
	m_sGameDir = sGameDir + "\\";

	// extract the game name
	CString sThrowAway;
	splitrelpath( sGameDir, sThrowAway, m_sName, sThrowAway );

	//
	// Parse the .game definition file
	//
	CString sGameFilePath = ssprintf("%s\\%s.game", sGameDir, m_sName);
	IniFile ini;
	ini.SetPath( sGameFilePath );
	if( !ini.ReadFile() )
		FatalError( "Error reading game definition file '%s'.", sGameFilePath );


	if( ini.GetValue( "Game", "Name" )  !=  m_sName )
		FatalError( "Game name in '%s' doesn't match the directory name.", sGameFilePath );

	m_sDescription = ini.GetValue( "Game", "Description" );

	m_iNumInstruments = ini.GetValueI( "Game", "NumInstruments" );
	if( m_iNumInstruments != 2 )
		FatalError( "Invalid value for NumInstruments in '%s'.", sGameFilePath );

	m_iButtonsPerInstrument = ini.GetValueI( "Game", "ButtonsPerInstrument" );
	if( m_iButtonsPerInstrument < 1  ||  m_iButtonsPerInstrument > MAX_INSTRUMENT_BUTTONS )
		FatalError( "Invalid value for ButtonsPerInstrument in '%s'.", sGameFilePath );

	CString sButtonsString = ini.GetValue( "Game", "ButtonNames" );
	CStringArray arrayButtonNames;
	split( sButtonsString, ",", arrayButtonNames );
	if( arrayButtonNames.GetSize() != m_iButtonsPerInstrument )
		FatalError( "Button names do not match number of buttons in '%s'.", sGameFilePath );
	for( int i=0; i<m_iButtonsPerInstrument; i++ )
		m_sButtonNames[i] = arrayButtonNames[i];

	CString sMenuButtonsString = ini.GetValue( "Game", "MenuButtons" );
	CStringArray arrayMenuButtonNames;
	split( sMenuButtonsString, ",", arrayMenuButtonNames );
	if( arrayMenuButtonNames.GetSize() != NUM_MENU_BUTTONS )
		FatalError( "Invalid number of menu buttons in '%s'.", sGameFilePath );
	for( i=0; i<NUM_MENU_BUTTONS; i++ )
		m_iMenuButtons[i] = ButtonNameToIndex( arrayMenuButtonNames[i] );
	


	//
	// Look for a StyleDefs in this game folder
	//
	m_iNumStyleDefs = 0;
	CStringArray arrayStyleFiles;
	GetDirListing( ssprintf("%s\\*.style", sGameDir), arrayStyleFiles );
	SortCStringArray( arrayStyleFiles );

	for( i=0; i<arrayStyleFiles.GetSize(); i++ )	// for each style
	{
		CString sStyleFileName = arrayStyleFiles[i];

		if( 0 == stricmp( sStyleFileName, "cvs" ) )	// the directory called "CVS"
			continue;		// ignore it

		CString sStyleFilePath = ssprintf("%s\\%s", sGameDir, sStyleFileName);

		// add the new style
		m_pStyleDefs[m_iNumStyleDefs++] = new StyleDef( this, sStyleFilePath );
	}



	//
	// Look for a note skins in this game folder
	//
	m_iNumSkinFolders = 0;
	CStringArray arraySkinFolders;
	GetDirListing( ssprintf("%s\\*.", sGameDir), arraySkinFolders, true );
	SortCStringArray( arraySkinFolders );

	if( arraySkinFolders.GetSize() == 0 )
		FatalError( "The Game directory '%s' must contain at least one skin.", sGameFilePath );

	for( i=0; i<arraySkinFolders.GetSize(); i++ )	// for each skin folder
	{
		CString sSkinFolder = arraySkinFolders[i];

		if( 0 == stricmp( sSkinFolder, "cvs" ) )	// the directory called "CVS"
			continue;		// ignore it

		m_sSkinFolders[m_iNumSkinFolders++] = sSkinFolder;
	}

}

GameDef::~GameDef() 
{
	for( int i=0; i<m_iNumStyleDefs; i++ )
		delete m_pStyleDefs[i];
}

StyleDef* GameDef::GetStyleDef( CString sStyle )
{
	for( int i=0; i<m_iNumStyleDefs; i++ )
	{
		if( m_pStyleDefs[i]->m_sName == sStyle )
			return m_pStyleDefs[i];
	}
	return NULL;
}

CString GameDef::ElementToGraphicSuffix( const GameButtonGraphic gbg ) 
{
	CString sAssetPath;		// fill this in below

	switch( gbg )
	{
		case GRAPHIC_NOTE_COLOR_PART:		sAssetPath = "note color part";			break;
		case GRAPHIC_NOTE_GRAY_PART:		sAssetPath = "note gray part";			break;
		case GRAPHIC_RECEPTOR:				sAssetPath = "receptor";				break;
		case GRAPHIC_HOLD_EXPLOSION:		sAssetPath = "hold explosion";			break;
		case GRAPHIC_TAP_EXPLOSION_BRIGHT:	sAssetPath = "tap explosion bright";	break;
		case GRAPHIC_TAP_EXPLOSION_DIM:		sAssetPath = "tap explosion dim";		break;

		default:
			FatalError( ssprintf("Unhandled StyleElement %d", gbg) );
	}
	
	return sAssetPath;
}

CString GameDef::GetPathToGraphic( const CString sSkinName, const int iInstrumentButton, const GameButtonGraphic gbg ) 
{
	const CString sSkinDir	= ssprintf("%s\\%s\\", m_sGameDir, sSkinName);
	const CString sButtonName = m_sButtonNames[ iInstrumentButton ];
	const CString sGraphicSuffix = ElementToGraphicSuffix( gbg );

	CStringArray arrayPossibleFileNames;		// fill this with the possible files

	GetDirListing( ssprintf("%s%s %s*.sprite", sSkinDir, sButtonName, sGraphicSuffix), arrayPossibleFileNames );
	GetDirListing( ssprintf("%s%s %s*.png",    sSkinDir, sButtonName, sGraphicSuffix), arrayPossibleFileNames );
	GetDirListing( ssprintf("%s%s %s*.jpg",    sSkinDir, sButtonName, sGraphicSuffix), arrayPossibleFileNames );
	GetDirListing( ssprintf("%s%s %s*.bmp",    sSkinDir, sButtonName, sGraphicSuffix), arrayPossibleFileNames );

	if( arrayPossibleFileNames.GetSize() > 0 )
		return sSkinDir + arrayPossibleFileNames[0];

	FatalError( "The game button graphic '%s%s %s' is missing.", sSkinDir, sButtonName, sGraphicSuffix );
	return "";
}
