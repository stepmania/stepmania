#include "global.h"
/*
-----------------------------------------------------------------------------
 Class: StyleDef

 Desc: A data structure that holds the definition of a GameMode.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/

#include "StyleDef.h"
#include "RageLog.h"
#include "RageUtil.h"
#include "GameDef.h"
#include "IniFile.h"
#include "GameState.h"
#include "NoteData.h"


void StyleDef::GetTransformedNoteDataForStyle( PlayerNumber pn, NoteData* pOriginal, NoteData* pNoteDataOut ) const
{
	int iNewToOriginalTrack[MAX_COLS_PER_PLAYER];
	for( int col=0; col<m_iColsPerPlayer; col++ )
	{
		ColumnInfo colInfo = m_ColumnInfo[pn][col];
		int originalTrack = colInfo.track;
		
		iNewToOriginalTrack[col] = originalTrack;
	}
	
	pNoteDataOut->LoadTransformed( pOriginal, m_iColsPerPlayer, iNewToOriginalTrack );
}


GameInput StyleDef::StyleInputToGameInput( const StyleInput StyleI ) const
{
	GameController c = m_ColumnInfo[StyleI.player][StyleI.col].controller;
	GameButton b = m_ColumnInfo[StyleI.player][StyleI.col].button;
	return GameInput( c, b );
};

StyleInput StyleDef::GameInputToStyleInput( const GameInput &GameI ) const
{
	StyleInput SI;

	for( int p=0; p<NUM_PLAYERS; p++ )
	{
		for( int t=0; t<MAX_NOTE_TRACKS; t++ )
		{
			if( m_ColumnInfo[p][t].controller == GameI.controller  &&
				m_ColumnInfo[p][t].button == GameI.button )
			{
				SI = StyleInput( (PlayerNumber)p, t );

				// HACK:  Looking up the player number using m_ColumnInfo 
				// returns the wrong answer for ONE_PLAYER_TWO_CREDITS styles
				if( m_StyleType == ONE_PLAYER_TWO_CREDITS )
					SI.player = GAMESTATE->m_MasterPlayerNumber;
				
				return SI;
			}
		}
	}
	return SI;	// Didn't find a match.  Return invalid.
}


PlayerNumber StyleDef::ControllerToPlayerNumber( GameController controller ) const
{
	switch( m_StyleType )
	{
	case ONE_PLAYER_ONE_CREDIT:
	case TWO_PLAYERS_TWO_CREDITS:
		return (PlayerNumber)controller;
	case ONE_PLAYER_TWO_CREDITS:
		return GAMESTATE->m_MasterPlayerNumber;
	default:
		ASSERT(0);
		return PLAYER_INVALID;
	}
}

bool StyleDef::MatchesNotesType( StepsType type ) const
{
	if(type == m_StepsType) return true;

	return false;
}

void StyleDef::GetMinAndMaxColX( PlayerNumber pn, float& fMixXOut, float& fMaxXOut ) const
{
	fMixXOut = +100000;
	fMaxXOut = -100000;
	for( int i=0; i<m_iColsPerPlayer; i++ )
	{
		fMixXOut = min( fMixXOut, m_ColumnInfo[pn][i].fXOffset );
		fMaxXOut = max( fMaxXOut, m_ColumnInfo[pn][i].fXOffset );
	}
}
