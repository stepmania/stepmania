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

CString GameDef::GetPathToGraphic( const int iInstrumentButton, const GameButtonGraphic gbg ) 
{
	const CString sDir	= "Game\\" + CString(m_szName) + "\\";
	const CString sButtonName = m_szButtonNames[ iInstrumentButton ];
	const CString sGraphicSuffix = ElementToGraphicSuffix( gbg );

	CStringArray arrayPossibleFileNames;		// fill this with the possible files

	GetDirListing( ssprintf("%s%s %s*.sprite", sDir, sButtonName, sGraphicSuffix), arrayPossibleFileNames );
	GetDirListing( ssprintf("%s%s %s*.png",    sDir, sButtonName, sGraphicSuffix), arrayPossibleFileNames );
	GetDirListing( ssprintf("%s%s %s*.jpg",    sDir, sButtonName, sGraphicSuffix), arrayPossibleFileNames );
	GetDirListing( ssprintf("%s%s %s*.bmp",    sDir, sButtonName, sGraphicSuffix), arrayPossibleFileNames );

	if( arrayPossibleFileNames.GetSize() > 0 )
		return sDir + arrayPossibleFileNames[0];

	FatalError( "The game button graphic '%s%s %s' is missing.", sDir, sButtonName, sGraphicSuffix );
	return "";
}
