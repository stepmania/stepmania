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
	void Init(int taps=0, int holds=0);

	// maintain this extra data in addition to the NoteData
	TapNoteScore	m_TapNoteScores[MAX_NOTE_TRACKS][MAX_TAP_NOTE_ROWS];
	vector<HoldNoteScore> m_HoldNoteScores;
	/* 1.0 means this HoldNote has full life.
	 * 0.0 means this HoldNote is dead
	 * When this value hits 0.0 for the first time, m_HoldScore becomes HSS_NG.
	 * If the life is > 0.0 when the HoldNote ends, then m_HoldScore becomes HSS_OK. */
	vector<float>	m_fHoldNoteLife;

	// statistics
	int GetNumTapNotesWithScore( TapNoteScore tns, const float fStartBeat = 0, const float fEndBeat = MAX_BEATS );
	int GetNumDoublesWithScore( TapNoteScore tns, const float fStartBeat = 0, const float fEndBeat = MAX_BEATS );
	int GetNumHoldNotesWithScore( HoldNoteScore hns, const float fStartBeat = 0, const float fEndBeat = MAX_BEATS );

	inline bool IsRowComplete( int index )
	{
		for( int t=0; t<m_iNumTracks; t++ )
			if( GetTapNote(t, index) != TAP_EMPTY && m_TapNoteScores[t][index] < TNS_GREAT )
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

