#pragma once
/*
-----------------------------------------------------------------------------
 Class: StyleDef

 Desc: A data structure that holds the definition for one of a Game's styles.
	Styles define a set of columns for each player, and information about those
	columns, like what Instruments are used play those columns and what track 
	to use to populate the column's notes.
	A "track" is the term used to descibe a particular note in NoteData space.
	A "column" is the term used to describe a particular note in Style space.

 Copyright (c) 2001-2002 by the persons listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/

#include "NoteMetadata.h"
#include "NoteData.h"
#include "StyleInput.h"
#include "GameInput.h"


typedef TrackNumber NoteColumn;
const int MAX_NOTE_COLUMNS = MAX_NOTE_TRACKS;


struct StyleDef
{
public:

	LPCTSTR m_szName;	// name of the style.  e.g. "single", "double"
	LPCTSTR m_szReads;	// name of the style that we can read.  For example, the style named "versus" reads the NoteData with the tag "single"

	int		m_iNumPlayers;	// either 1 or 2
	int		m_iNumTracks;	// number of total tracks this style expects (e.g. 4 for versus, but 8 for double)

	inline int GetColsPerPlayer() { return m_iNumTracks/m_iNumPlayers; };

	struct GameInputAndTrack { TrackNumber track; InstrumentNumber number; InstrumentButton button; };

	GameInputAndTrack m_StyleInputToGameInputAndTrack[NUM_PLAYERS][MAX_NOTE_COLUMNS];	// maps each players' column to a track in the NoteData


	inline GameInput StyleInputToGameInput( const StyleInput StyleI )
	{
		InstrumentNumber n = m_StyleInputToGameInputAndTrack[StyleI.player][StyleI.col].number;
		InstrumentButton b = m_StyleInputToGameInputAndTrack[StyleI.player][StyleI.col].button;
		return GameInput( n, b );
	};

	inline StyleInput GameInputToStyleInput( const GameInput GameI )
	{
		for( int p=0; p<NUM_PLAYERS; p++ )
		{
			for( int t=0; t<MAX_NOTE_TRACKS; t++ )
			{
				if( m_StyleInputToGameInputAndTrack[p][t].number == GameI.number  &&
					m_StyleInputToGameInputAndTrack[p][t].button == GameI.button )
				{
					return StyleInput( (PlayerNumber)p, t );
				}
			}
		}
		return StyleInput();	// didn't find a StyleInput
	};

	void GetTransformedNoteDataForStyle( PlayerNumber p, NoteData* pOriginal, NoteData &newNoteData )
	{
		TrackNumber iNewToOriginalTrack[MAX_NOTE_COLUMNS];
		for( int col=0; col<GetColsPerPlayer(); col++ )
		{
			GameInputAndTrack GIAT = m_StyleInputToGameInputAndTrack[p][col];
			TrackNumber originalTrack = GIAT.track;
			
			iNewToOriginalTrack[col] = originalTrack;
		}
		
		newNoteData.LoadTransformed( pOriginal, GetColsPerPlayer(), iNewToOriginalTrack );
	}

};

