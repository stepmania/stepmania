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
	StyleDef sd;


	/*
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
	*/

	switch( mode )
	{
	case STYLE_SINGLE:
	case STYLE_DOUBLE:
	case STYLE_SOLO:
		sd.m_iNumPlayers = 1;
		break;
	case STYLE_VERSUS:
	case STYLE_COUPLE:
		sd.m_iNumPlayers = 2;
		break;
	default:
		ASSERT( false );	// invalid GameMode
	}


	switch( mode )
	{
	case STYLE_SINGLE:
	case STYLE_VERSUS:
	case STYLE_COUPLE:
		sd.m_iNumColumns = 4;
		sd.m_ColumnToTapNote[0]		= NOTE_PAD1_LEFT;
		sd.m_ColumnToTapNote[1]		= NOTE_PAD1_DOWN;
		sd.m_ColumnToTapNote[2]		= NOTE_PAD1_UP;
		sd.m_ColumnToTapNote[3]		= NOTE_PAD1_RIGHT;
		sd.m_sColumnToNoteName[0] = "left";
		sd.m_sColumnToNoteName[1] = "down";
		sd.m_sColumnToNoteName[2] = "up";
		sd.m_sColumnToNoteName[3] = "right";
/*		sd.m_ColumnToRotation[0] = mapTapNoteToRotation[NOTE_PAD1_LEFT];
		sd.m_ColumnToRotation[1] = mapTapNoteToRotation[NOTE_PAD1_DOWN];
		sd.m_ColumnToRotation[2] = mapTapNoteToRotation[NOTE_PAD1_UP];
		sd.m_ColumnToRotation[3] = mapTapNoteToRotation[NOTE_PAD1_RIGHT];
*/		break;
	case STYLE_SOLO:
		sd.m_iNumColumns = 6;		// LEFT, UP+LEFT, DOWN, UP, UP+RIGHT, RIGHT
		sd.m_ColumnToTapNote[0]		= NOTE_PAD1_LEFT;
		sd.m_ColumnToTapNote[1]		= NOTE_PAD1_UPLEFT;
		sd.m_ColumnToTapNote[2]		= NOTE_PAD1_DOWN;
		sd.m_ColumnToTapNote[3]		= NOTE_PAD1_UP;
		sd.m_ColumnToTapNote[4]		= NOTE_PAD1_UPRIGHT;
		sd.m_ColumnToTapNote[5]		= NOTE_PAD1_RIGHT;
		sd.m_sColumnToNoteName[0] = "left";
		sd.m_sColumnToNoteName[1] = "upleft";
		sd.m_sColumnToNoteName[2] = "down";
		sd.m_sColumnToNoteName[3] = "up";
		sd.m_sColumnToNoteName[4] = "upright";
		sd.m_sColumnToNoteName[5] = "right";
/*		sd.m_ColumnToRotation[0] = mapTapNoteToRotation[NOTE_PAD1_LEFT];
		sd.m_ColumnToRotation[1] = mapTapNoteToRotation[NOTE_PAD1_UPLEFT];
		sd.m_ColumnToRotation[2] = mapTapNoteToRotation[NOTE_PAD1_DOWN];
		sd.m_ColumnToRotation[3] = mapTapNoteToRotation[NOTE_PAD1_UP];
		sd.m_ColumnToRotation[4] = mapTapNoteToRotation[NOTE_PAD1_UPRIGHT];
		sd.m_ColumnToRotation[5] = mapTapNoteToRotation[NOTE_PAD1_RIGHT];
*/		break;
	case STYLE_DOUBLE:
		sd.m_iNumColumns = 8;		// 1_LEFT, 1_DOWN, 1_UP, 1_RIGHT, 2_LEFT, 2_DOWN, 2_UP, 2_RIGHT
		sd.m_ColumnToTapNote[0]		= NOTE_PAD1_LEFT;
		sd.m_ColumnToTapNote[1]		= NOTE_PAD1_DOWN;
		sd.m_ColumnToTapNote[2]		= NOTE_PAD1_UP;
		sd.m_ColumnToTapNote[3]		= NOTE_PAD1_RIGHT;
		sd.m_ColumnToTapNote[4]		= NOTE_PAD2_LEFT;
		sd.m_ColumnToTapNote[5]		= NOTE_PAD2_DOWN;
		sd.m_ColumnToTapNote[6]		= NOTE_PAD2_UP;
		sd.m_ColumnToTapNote[7]		= NOTE_PAD2_RIGHT;
		sd.m_ColumnToTapNote[8]		= NOTE_PAD1_UPLEFT;// Even though this isn't used, Need them here so
		sd.m_ColumnToTapNote[9]		= NOTE_PAD1_UPRIGHT;// TapNoteToColumnNumber doesn't get confused by them
		sd.m_ColumnToTapNote[10]		= NOTE_PAD2_UPLEFT;
		sd.m_ColumnToTapNote[11]		= NOTE_PAD2_UPRIGHT;
		sd.m_sColumnToNoteName[0] = "left";
		sd.m_sColumnToNoteName[1] = "upleft";
		sd.m_sColumnToNoteName[2] = "down";
		sd.m_sColumnToNoteName[3] = "up";
		sd.m_sColumnToNoteName[4] = "upright";
		sd.m_sColumnToNoteName[5] = "right";
		sd.m_sColumnToNoteName[6] = "left";
		sd.m_sColumnToNoteName[7] = "upleft";
		sd.m_sColumnToNoteName[8] = "down";
		sd.m_sColumnToNoteName[9] = "up";
		sd.m_sColumnToNoteName[10] = "upright";
		sd.m_sColumnToNoteName[11] = "right";
		/*
		sd.m_ColumnToRotation[0] = mapTapNoteToRotation[NOTE_PAD1_LEFT];
		sd.m_ColumnToRotation[1] = mapTapNoteToRotation[NOTE_PAD1_DOWN];
		sd.m_ColumnToRotation[2] = mapTapNoteToRotation[NOTE_PAD1_UP];
		sd.m_ColumnToRotation[3] = mapTapNoteToRotation[NOTE_PAD1_RIGHT];
		sd.m_ColumnToRotation[4] = mapTapNoteToRotation[NOTE_PAD2_LEFT];
		sd.m_ColumnToRotation[5] = mapTapNoteToRotation[NOTE_PAD2_DOWN];
		sd.m_ColumnToRotation[6] = mapTapNoteToRotation[NOTE_PAD2_UP];
		sd.m_ColumnToRotation[7] = mapTapNoteToRotation[NOTE_PAD2_RIGHT];
		sd.m_ColumnToRotation[8] = mapTapNoteToRotation[NOTE_PAD1_UPLEFT];
		sd.m_ColumnToRotation[9] = mapTapNoteToRotation[NOTE_PAD1_UPRIGHT];
		sd.m_ColumnToRotation[10] = mapTapNoteToRotation[NOTE_PAD2_UPLEFT];
		sd.m_ColumnToRotation[11] = mapTapNoteToRotation[NOTE_PAD2_UPRIGHT];
		*/
		break;
	default:
		ASSERT( false );	// invalid GameMode
	}

	return sd;
}