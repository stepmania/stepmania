#include "stdafx.h"
/*
-----------------------------------------------------------------------------
 Class: GameDef

 Desc: See header.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/

#include "GameDef.h"
#include "RageLog.h"
#include "RageUtil.h"

#include "IniFile.h"
#include "StyleDef.h"


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
			throw RageException( ssprintf("Unhandled StyleElement %d", gbg) );
	}
	
	return sAssetPath;
}

CString GameDef::GetPathToGraphic( const CString sSkinName, const int iInstrumentButton, const GameButtonGraphic gbg ) 
{
	const CString sSkinDir	= ssprintf("Skins\\%s\\%s\\", m_szName, sSkinName);
	const CString sButtonName = m_szButtonNames[ iInstrumentButton ];
	const CString sGraphicSuffix = ElementToGraphicSuffix( gbg );

	CStringArray arrayPossibleFileNames;		// fill this with the possible files

	GetDirListing( ssprintf("%s%s %s*.sprite", sSkinDir, sButtonName, sGraphicSuffix), arrayPossibleFileNames );
	GetDirListing( ssprintf("%s%s %s*.png",    sSkinDir, sButtonName, sGraphicSuffix), arrayPossibleFileNames );
	GetDirListing( ssprintf("%s%s %s*.jpg",    sSkinDir, sButtonName, sGraphicSuffix), arrayPossibleFileNames );
	GetDirListing( ssprintf("%s%s %s*.bmp",    sSkinDir, sButtonName, sGraphicSuffix), arrayPossibleFileNames );

	if( arrayPossibleFileNames.GetSize() > 0 )
		return sSkinDir + arrayPossibleFileNames[0];

	throw RageException( "The game button graphic '%s%s %s' is missing.", sSkinDir, sButtonName, sGraphicSuffix );
	return "";
}

void GameDef::GetTweenColors( const CString sSkinName, const int iInstrumentButton, CArray<D3DXCOLOR,D3DXCOLOR> &arrayTweenColors )
{
	const CString sSkinDir	= ssprintf("Skins\\%s\\%s\\", m_szName, sSkinName);
	const CString sButtonName = m_szButtonNames[ iInstrumentButton ];

	const CString sColorsFilePath = sSkinDir + sButtonName + ".colors";

	FILE* file = fopen( sColorsFilePath, "r" );
	ASSERT( file != NULL );
	if( file == NULL )
		return;

	bool bSuccess;
	do
	{
		D3DXCOLOR color;
		int retval = fscanf( file, "%f,%f,%f,%f\n", &color.r, &color.g, &color.b, &color.a );
		bSuccess = retval == 4;
		if( bSuccess )
			arrayTweenColors.Add( color );
	} while( bSuccess );

	return;
}

void GameDef::GetSkinNames( CStringArray &AddTo )
{
	CString sBaseSkinFolder = "Skins\\" + CString(m_szName) + "\\";
	GetDirListing( sBaseSkinFolder + "*.*", AddTo, true );

	// strip out "CVS"
	for( int i=AddTo.GetSize()-1; i>=0; i-- )
		if( 0 == stricmp("cvs", AddTo[i]) )
			AddTo.RemoveAt( i );

	if( AddTo.GetSize() == 0 )
		throw RageException( "The folder '%s' must contain at least one skin.", sBaseSkinFolder );
}

bool GameDef::HasASkinNamed( CString sSkin )
{
	CStringArray asSkinNames;
	GetSkinNames( asSkinNames );

	for( int i=0; i<asSkinNames.GetSize(); i++ )
		if( asSkinNames[i] == sSkin )
			return true;

	return false;
}

void GameDef::AssertSkinsAreComplete()
{
	CStringArray asSkinNames;
	GetSkinNames( asSkinNames );

	for( int i=0; i<asSkinNames.GetSize(); i++ )
	{
		CString sSkin = asSkinNames[i];
		CString sGameSkinFolder = "Skins\\" + sSkin + "\\";

		for( int i=0; i<NUM_GAME_BUTTON_GRAPHICS; i++ )
		{
			GameButtonGraphic gbg = (GameButtonGraphic)i;
			CString sPathToGraphic = GetPathToGraphic( sSkin, INSTRUMENT_1, gbg );
			if( !DoesFileExist(sPathToGraphic) )
				throw RageException( "Game button graphic at %s is missing.", sPathToGraphic );
		}		
	}
}
