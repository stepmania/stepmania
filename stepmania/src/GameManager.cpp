#include "stdafx.h"
/*
-----------------------------------------------------------------------------
 Class: GameManager

 Desc: See Header.

 Copyright (c) 2001-2002 by the names listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/

#include "GameManager.h"
#include "D3DXmath.h"


GameManager*	GAME = NULL;	// global and accessable from anywhere in our program



GameManager::GameManager()
{
	m_DanceStyle = STYLE_SINGLE;
}

GameManager::~GameManager()
{

}

bool GameManager::IsPlayerEnabled( PlayerNumber PlayerNo )
{
	switch( m_DanceStyle )
	{
	case STYLE_VERSUS:
	case STYLE_COUPLE:
		if( PlayerNo == PLAYER_1 )
			return true;
		if( PlayerNo == PLAYER_2 )
			return true;
		break;
	case STYLE_SINGLE:
	case STYLE_SOLO:
	case STYLE_DOUBLE:
		if( PlayerNo == PLAYER_1 )
			return true;
		if( PlayerNo == PLAYER_2 )
			return false;
		break;
	default:
		ASSERT( false );	// invalid StyleDef
	}
	return false;
}


StyleDef GameManager::GetStyleDef( DanceStyle mode )
{
	StyleDef StyleDef;


	CMap<TapNote, TapNote, float, float> mapTapNoteToRotation;	// arrow facing left is rotation 0
	mapTapNoteToRotation[NOTE_PAD1_LEFT]	= 0;
	mapTapNoteToRotation[NOTE_PAD1_UPLEFT]	= D3DX_PI/4.0f;
	mapTapNoteToRotation[NOTE_PAD1_DOWN]	= -D3DX_PI/2.0f;
	mapTapNoteToRotation[NOTE_PAD1_UP]		= D3DX_PI/2.0f;
	mapTapNoteToRotation[NOTE_PAD1_UPRIGHT]	= D3DX_PI*3.0f/4.0f;
	mapTapNoteToRotation[NOTE_PAD1_RIGHT]	= D3DX_PI;
	mapTapNoteToRotation[NOTE_PAD2_LEFT]	= mapTapNoteToRotation[NOTE_PAD1_LEFT];
	mapTapNoteToRotation[NOTE_PAD2_UPLEFT]	= mapTapNoteToRotation[NOTE_PAD1_UPLEFT];
	mapTapNoteToRotation[NOTE_PAD2_DOWN]	= mapTapNoteToRotation[NOTE_PAD1_DOWN];
	mapTapNoteToRotation[NOTE_PAD2_UP]		= mapTapNoteToRotation[NOTE_PAD1_UP];
	mapTapNoteToRotation[NOTE_PAD2_UPRIGHT]	= mapTapNoteToRotation[NOTE_PAD1_UPRIGHT];
	mapTapNoteToRotation[NOTE_PAD2_RIGHT]	= mapTapNoteToRotation[NOTE_PAD1_RIGHT];


	switch( mode )
	{
	case STYLE_SINGLE:
	case STYLE_DOUBLE:
	case STYLE_SOLO:
		StyleDef.m_iNumPlayers = 1;
		break;
	case STYLE_VERSUS:
	case STYLE_COUPLE:
		StyleDef.m_iNumPlayers = 2;
		break;
	default:
		ASSERT( false );	// invalid GameMode
	}


	switch( mode )
	{
	case STYLE_SINGLE:
	case STYLE_VERSUS:
	case STYLE_COUPLE:
		StyleDef.m_iNumColumns = 4;		// LEFT, DOWN, UP, RIGHT
		StyleDef.m_ColumnToTapNote[0]		= NOTE_PAD1_LEFT;
		StyleDef.m_ColumnToTapNote[1]		= NOTE_PAD1_DOWN;
		StyleDef.m_ColumnToTapNote[2]		= NOTE_PAD1_UP;
		StyleDef.m_ColumnToTapNote[3]		= NOTE_PAD1_RIGHT;
		StyleDef.m_ColumnToTapNote[4]		= NOTE_PAD1_UPLEFT; // Even though this isn't used, Need them here so
		StyleDef.m_ColumnToTapNote[5]		= NOTE_PAD1_UPRIGHT; // TapNoteToColumnNumber doesn't get confused by them
		StyleDef.m_ColumnToRotation[0] = mapTapNoteToRotation[NOTE_PAD1_LEFT];
		StyleDef.m_ColumnToRotation[1] = mapTapNoteToRotation[NOTE_PAD1_DOWN];
		StyleDef.m_ColumnToRotation[2] = mapTapNoteToRotation[NOTE_PAD1_UP];
		StyleDef.m_ColumnToRotation[3] = mapTapNoteToRotation[NOTE_PAD1_RIGHT];
		StyleDef.m_ColumnToRotation[4] = mapTapNoteToRotation[NOTE_PAD1_UPLEFT];
		StyleDef.m_ColumnToRotation[5] = mapTapNoteToRotation[NOTE_PAD1_UPRIGHT];
		break;
	case STYLE_SOLO:
		StyleDef.m_iNumColumns = 6;		// LEFT, UP+LEFT, DOWN, UP, UP+RIGHT, RIGHT
		StyleDef.m_ColumnToTapNote[0]		= NOTE_PAD1_LEFT;
		StyleDef.m_ColumnToTapNote[1]		= NOTE_PAD1_UPLEFT;
		StyleDef.m_ColumnToTapNote[2]		= NOTE_PAD1_DOWN;
		StyleDef.m_ColumnToTapNote[3]		= NOTE_PAD1_UP;
		StyleDef.m_ColumnToTapNote[4]		= NOTE_PAD1_UPRIGHT;
		StyleDef.m_ColumnToTapNote[5]		= NOTE_PAD1_RIGHT;
		StyleDef.m_ColumnToRotation[0] = mapTapNoteToRotation[NOTE_PAD1_LEFT];
		StyleDef.m_ColumnToRotation[1] = mapTapNoteToRotation[NOTE_PAD1_UPLEFT];
		StyleDef.m_ColumnToRotation[2] = mapTapNoteToRotation[NOTE_PAD1_DOWN];
		StyleDef.m_ColumnToRotation[3] = mapTapNoteToRotation[NOTE_PAD1_UP];
		StyleDef.m_ColumnToRotation[4] = mapTapNoteToRotation[NOTE_PAD1_UPRIGHT];
		StyleDef.m_ColumnToRotation[5] = mapTapNoteToRotation[NOTE_PAD1_RIGHT];
		break;
	case STYLE_DOUBLE:
		StyleDef.m_iNumColumns = 8;		// 1_LEFT, 1_DOWN, 1_UP, 1_RIGHT, 2_LEFT, 2_DOWN, 2_UP, 2_RIGHT
		StyleDef.m_ColumnToTapNote[0]		= NOTE_PAD1_LEFT;
		StyleDef.m_ColumnToTapNote[1]		= NOTE_PAD1_DOWN;
		StyleDef.m_ColumnToTapNote[2]		= NOTE_PAD1_UP;
		StyleDef.m_ColumnToTapNote[3]		= NOTE_PAD1_RIGHT;
		StyleDef.m_ColumnToTapNote[4]		= NOTE_PAD2_LEFT;
		StyleDef.m_ColumnToTapNote[5]		= NOTE_PAD2_DOWN;
		StyleDef.m_ColumnToTapNote[6]		= NOTE_PAD2_UP;
		StyleDef.m_ColumnToTapNote[7]		= NOTE_PAD2_RIGHT;
		StyleDef.m_ColumnToTapNote[8]		= NOTE_PAD1_UPLEFT;// Even though this isn't used, Need them here so
		StyleDef.m_ColumnToTapNote[9]		= NOTE_PAD1_UPRIGHT;// TapNoteToColumnNumber doesn't get confused by them
		StyleDef.m_ColumnToTapNote[10]		= NOTE_PAD2_UPLEFT;
		StyleDef.m_ColumnToTapNote[11]		= NOTE_PAD2_UPRIGHT;
		StyleDef.m_ColumnToRotation[0] = mapTapNoteToRotation[NOTE_PAD1_LEFT];
		StyleDef.m_ColumnToRotation[1] = mapTapNoteToRotation[NOTE_PAD1_DOWN];
		StyleDef.m_ColumnToRotation[2] = mapTapNoteToRotation[NOTE_PAD1_UP];
		StyleDef.m_ColumnToRotation[3] = mapTapNoteToRotation[NOTE_PAD1_RIGHT];
		StyleDef.m_ColumnToRotation[4] = mapTapNoteToRotation[NOTE_PAD2_LEFT];
		StyleDef.m_ColumnToRotation[5] = mapTapNoteToRotation[NOTE_PAD2_DOWN];
		StyleDef.m_ColumnToRotation[6] = mapTapNoteToRotation[NOTE_PAD2_UP];
		StyleDef.m_ColumnToRotation[7] = mapTapNoteToRotation[NOTE_PAD2_RIGHT];
		StyleDef.m_ColumnToRotation[8] = mapTapNoteToRotation[NOTE_PAD1_UPLEFT];
		StyleDef.m_ColumnToRotation[9] = mapTapNoteToRotation[NOTE_PAD1_UPRIGHT];
		StyleDef.m_ColumnToRotation[10] = mapTapNoteToRotation[NOTE_PAD2_UPLEFT];
		StyleDef.m_ColumnToRotation[11] = mapTapNoteToRotation[NOTE_PAD2_UPRIGHT];
		break;
	default:
		ASSERT( false );	// invalid GameMode
	}

	return StyleDef;
}