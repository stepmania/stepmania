#ifndef NOTEDATAWITHSCORING_H
#define NOTEDATAWITHSCORING_H
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

class NoteDataWithScoring : public NoteData
{
	// maintain this extra data in addition to the NoteData
	vector<TapNoteScore> m_TapNoteScores[MAX_NOTE_TRACKS];
	vector<HoldNoteScore> m_HoldNoteScores;

	/* 1.0 means this HoldNote has full life.
	 * 0.0 means this HoldNote is dead
	 * When this value hits 0.0 for the first time, m_HoldScore becomes HSS_NG.
	 * If the life is > 0.0 when the HoldNote ends, then m_HoldScore becomes HSS_OK. */
	vector<float>	m_fHoldNoteLife;

public:
	NoteDataWithScoring();
	void Init();

	// statistics
	int GetNumTapNotesWithScore( TapNoteScore tns, const float fStartBeat = 0, const float fEndBeat = -1 ) const;
	int GetNumDoublesWithScore( TapNoteScore tns, const float fStartBeat = 0, const float fEndBeat = -1 ) const;
	int GetNumHoldNotesWithScore( HoldNoteScore hns, const float fStartBeat = 0, const float fEndBeat = -1 ) const;

	TapNoteScore GetTapNoteScore(unsigned track, unsigned row) const;
	void SetTapNoteScore(unsigned track, unsigned row, TapNoteScore tns);
	HoldNoteScore GetHoldNoteScore(unsigned h) const;
	void SetHoldNoteScore(unsigned h, HoldNoteScore hns);
	float GetHoldNoteLife(unsigned h) const;
	void SetHoldNoteLife(unsigned h, float f);

	bool IsRowComplete( int index, TapNoteScore minGrade = TNS_GREAT ) const;

	float GetActualRadarValue( RadarCategory rv, float fSongSeconds ) const;
	float GetActualStreamRadarValue( float fSongSeconds ) const;
	float GetActualVoltageRadarValue( float fSongSeconds ) const;
	float GetActualAirRadarValue( float fSongSeconds ) const;
	float GetActualFreezeRadarValue( float fSongSeconds ) const;
	float GetActualChaosRadarValue( float fSongSeconds ) const;
};

#endif
