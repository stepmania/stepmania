#include "stdafx.h"
/*
-----------------------------------------------------------------------------
 File: StyleDef.cpp

 Desc: A data structure that holds the definition of a GameMode.

 Copyright (c) 2001 Chris Danford.  All rights reserved.
-----------------------------------------------------------------------------
*/

#include "StyleDef.h"
#include "RageHelper.h"
#include "RageUtil.h"


CString StyleDef::ElementToGraphicSuffix( const StyleElement se ) 
{
	CString sAssetPath;		// fill this in below

	switch( se )
	{
		case GRAPHIC_NOTE_COLOR_PART:		sAssetPath = "note color part";			break;
		case GRAPHIC_NOTE_GRAY_PART:		sAssetPath = "note gray part";			break;
		case GRAPHIC_RECEPTOR:				sAssetPath = "receptor";				break;
		case GRAPHIC_HOLD_EXPLOSION:		sAssetPath = "hold explosion";			break;
		case GRAPHIC_TAP_EXPLOSION_BRIGHT:	sAssetPath = "tap explosion bright";	break;
		case GRAPHIC_TAP_EXPLOSION_DIM:		sAssetPath = "tap explosion dim";		break;

		default:
			HELPER.FatalError( ssprintf("Unhandled StyleElement %d", se) );
	}
	
	return sAssetPath;
}

CString StyleDef::GetPathToGraphic( const int iColumnNumber, const StyleElement se ) 
{
	const CString sDir	= "Game\\Dance\\";
	const CString sNoteName = m_sColumnToNoteName[ iColumnNumber ];
	const CString sGraphicSuffix = ElementToGraphicSuffix( se );

	CStringArray arrayPossibleElementFileNames;		// fill this with the possible files

	GetDirListing( ssprintf("%s%s %s*.sprite", sDir, sNoteName, sGraphicSuffix), arrayPossibleElementFileNames );
	GetDirListing( ssprintf("%s%s %s*.png",    sDir, sNoteName, sGraphicSuffix), arrayPossibleElementFileNames );
	GetDirListing( ssprintf("%s%s %s*.jpg",    sDir, sNoteName, sGraphicSuffix), arrayPossibleElementFileNames );
	GetDirListing( ssprintf("%s%s %s*.bmp",    sDir, sNoteName, sGraphicSuffix), arrayPossibleElementFileNames );

	if( arrayPossibleElementFileNames.GetSize() > 0 )
		return sDir + arrayPossibleElementFileNames[0];

	HELPER.FatalError( "The style graphic '%s' if missing from '%s'.", sNoteName+" "+sGraphicSuffix, sDir );
	return "";
}