#pragma once
/*
-----------------------------------------------------------------------------
 Class: StyleDef

 Desc: A data structure that holds the definition for one of a Game's styles.
	Styles define a set of columns for each player, and information about those
	columns, like what Instruments are used play those columns and what track 
	to use to populate the column's notes.
	A "track" is the term used to descibe a particular vertical sting of note 
	in NoteData.
	A "column" is the term used to describe the vertical string of notes that 
	a player sees on the screen while they're playing.  Column notes are 
	picked from a track, but columns and tracks don't have a 1-to-1 
	correspondance.  For example, dance-versus has 8 columns but only 4 tracks
	because two players place from the same set of 4 tracks.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/

#include "Notes.h"
#include "NoteData.h"
#include "StyleInput.h"
#include "GameInput.h"


const int MAX_COLS_PER_PLAYER = MAX_NOTE_TRACKS;


class GameDef;

class StyleDef
{
public:
	StyleDef( GameDef* pGameDef, CString sStyleFilePath );

	struct ColumnInfo 
	{ 
		TrackNumber track;	// take note data from this track
		InstrumentNumber number;	// use this instrument to hit a note on this track
		InstrumentButton button;	// use this button to hit a note on this track
		int	iX;				// x position of the column
	};

	CString		m_sName;	// name of the style.  e.g. "single", "double"
	CString		m_sDescription;	
	CString		m_sReadsTag;	// name of the style that we can read.  For example, the style named "versus" reads the NoteData with the tag "dance-single"
	int			m_iNumPlayers;	// either 1 or 2
	int			m_iColsPerPlayer;	// number of total tracks this style expects (e.g. 4 for versus, but 8 for double)
	ColumnInfo	m_ColumnInfo[NUM_PLAYERS][MAX_COLS_PER_PLAYER];	// maps each players' column to a track in the NoteData
	int			m_iColumnDrawOrder[MAX_COLS_PER_PLAYER];

	inline GameInput StyleInputToGameInput( const StyleInput StyleI )
	{
		InstrumentNumber n = m_ColumnInfo[StyleI.player][StyleI.col].number;
		InstrumentButton b = m_ColumnInfo[StyleI.player][StyleI.col].button;
		return GameInput( n, b );
	};

	inline StyleInput StyleDef::GameInputToStyleInput( const GameInput &GameI )
	{
		for( int p=0; p<NUM_PLAYERS; p++ )
		{
			for( int t=0; t<MAX_NOTE_TRACKS; t++ )
			{
				if( m_ColumnInfo[p][t].number == GameI.number  &&
					m_ColumnInfo[p][t].button == GameI.button )
				{
					return StyleInput( (PlayerNumber)p, t );
				}
			}
		}
		return StyleInput();	// Didn't find a match.  Return blank.
	}

	void GetTransformedNoteDataForStyle( PlayerNumber p, NoteData* pOriginal, NoteData &newNoteData );

};

