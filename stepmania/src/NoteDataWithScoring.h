#pragma once
/*
-----------------------------------------------------------------------------
 Class: NoteDataWithScoring

 Desc: NoteData with scores for each TapNote and HoldNote

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/

#include "GameConstantsAndTypes.h"
#include "NoteData.h"

struct NoteDataWithScoring : public NoteData
{
	NoteDataWithScoring();
	void Init();

	// maintain this extra data in addition to the NoteData
	TapNoteScore	m_TapNoteScores[MAX_NOTE_TRACKS][MAX_TAP_NOTE_ROWS];
	HoldNoteScore	m_HoldNoteScores[MAX_HOLD_NOTE_ELEMENTS];


	// statistics
	int GetNumSuccessfulTapNotes( const float fStartBeat = 0, const float fEndBeat = MAX_BEATS );
	int GetNumSuccessfulDoubles( const float fStartBeat = 0, const float fEndBeat = MAX_BEATS );
	int GetNumSuccessfulHoldNotes( const float fStartBeat = 0, const float fEndBeat = MAX_BEATS );

	inline bool IsRowComplete( int index )
	{
		for( int t=0; t<m_iNumTracks; t++ )
			if( m_TapNotes[t][index] != '0'  &&  m_TapNoteScores[t][index] < TNS_GREAT )
				return false;
		return true;
	}

	float GetActualRadarValue( RadarCategory rv, float fSongSeconds )
	{
		switch( rv )
		{
		case RADAR_STREAM:	return GetActualStreamRadarValue( fSongSeconds );	break;
		case RADAR_VOLTAGE:	return GetActualVoltageRadarValue( fSongSeconds );	break;
		case RADAR_AIR:		return GetActualAirRadarValue( fSongSeconds );		break;
		case RADAR_FREEZE:	return GetActualFreezeRadarValue( fSongSeconds );	break;
		case RADAR_CHAOS:	return GetActualChaosRadarValue( fSongSeconds );	break;
		default:	ASSERT(0);  return 0;
		}
	};
	float GetActualStreamRadarValue( float fSongSeconds );
	float GetActualVoltageRadarValue( float fSongSeconds );
	float GetActualAirRadarValue( float fSongSeconds );
	float GetActualFreezeRadarValue( float fSongSeconds );
	float GetActualChaosRadarValue( float fSongSeconds );
};

