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
#include "RageException.h"
#include "GameState.h"
#include "InputMapper.h"
#include "PrefsManager.h"


CString GameDef::ElementToGraphicSuffix( const SkinElement gbg ) 
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

CString GameDef::GetPathToGraphic( const CString sSkinName, const CString sButtonName, const SkinElement gbg ) 
{
	const CString sSkinDir	= ssprintf("Skins\\%s\\%s\\", m_szName, sSkinName);
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

void GameDef::GetTweenColors( const CString sSkinName, const CString sButtonName, CArray<D3DXCOLOR,D3DXCOLOR> &arrayTweenColors )
{
	const CString sSkinDir	= ssprintf("Skins\\%s\\%s\\", m_szName, sSkinName);

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
		AssertSkinIsComplete( asSkinNames[i] );
}

void GameDef::AssertSkinIsComplete( CString sSkin )
{
	CString sGameSkinFolder = "Skins\\" + sSkin + "\\";

	for( int j=0; j<NUM_GAME_BUTTON_GRAPHICS; j++ )
	{
		SkinElement gbg = (SkinElement)j;
		CString sButtonName = m_szButtonNames[j];
		CString sPathToGraphic = GetPathToGraphic( sSkin, sButtonName, gbg );
		if( !DoesFileExist(sPathToGraphic) )
			throw RageException( "Game button graphic at %s is missing.", sPathToGraphic );
	}		
}

MenuInput GameDef::GameInputToMenuInput( GameInput GameI )
{
	PlayerNumber pn;

	StyleDef::StyleType type = StyleDef::TWO_PLAYERS_USE_TWO_SIDES;
	if( GAMESTATE->m_CurStyle != STYLE_NONE )
		type = GAMESTATE->GetCurrentStyleDef()->m_StyleType;
	switch( type )
	{
	case StyleDef::ONE_PLAYER_USES_ONE_SIDE:
	case StyleDef::TWO_PLAYERS_USE_TWO_SIDES:
		pn = (PlayerNumber)GameI.controller;
		break;
	case StyleDef::ONE_PLAYER_USES_TWO_SIDES:
		pn = GAMESTATE->m_MasterPlayerNumber;
		break;
	default:
		ASSERT(0);	// invalid m_StyleType
	};

	for( int i=0; i<NUM_MENU_BUTTONS; i++ )
		if( m_DedicatedMenuButton[i] == GameI.button )
			return MenuInput( pn, (MenuButton)i );

	for( i=0; i<NUM_MENU_BUTTONS; i++ )
		if( m_SecondaryMenuButton[i] == GameI.button )
			return MenuInput( pn, (MenuButton)i );

	return MenuInput();	// invalid GameInput
}

void GameDef::MenuInputToGameInput( MenuInput MenuI, GameInput GameIout[4] )
{
	ASSERT( MenuI.IsValid() );

	GameIout[0].MakeInvalid();	// initialize
	GameIout[1].MakeInvalid();	
	GameIout[2].MakeInvalid();	
	GameIout[3].MakeInvalid();	

	GameController controller[2];
	int iNumSidesUsing = 1;
	switch( GAMESTATE->GetCurrentStyleDef()->m_StyleType )
	{
	case StyleDef::ONE_PLAYER_USES_ONE_SIDE:
	case StyleDef::TWO_PLAYERS_USE_TWO_SIDES:
		controller[0] = (GameController)MenuI.player;
		iNumSidesUsing = 1;
		break;
	case StyleDef::ONE_PLAYER_USES_TWO_SIDES:
		controller[0] = GAME_CONTROLLER_1;
		controller[1] = GAME_CONTROLLER_2;
		iNumSidesUsing = 2;
		break;
	default:
		ASSERT(0);	// invalid m_StyleType
	};

	GameButton button[2] = { m_DedicatedMenuButton[MenuI.button], m_SecondaryMenuButton[MenuI.button] };
	int iNumButtonsUsing = PREFSMAN->m_bOnlyDedicatedMenuButtons ? 1 : 2;

	for( int i=0; i<iNumSidesUsing; i++ )
	{
		for( int j=0; j<iNumButtonsUsing; j++ )
		GameIout[i*2+j].controller = controller[i];
		GameIout[i*2+j].button = button[j];
	}
}
