/* NoteDataWithScoring - NoteData with scores for each TapNote and HoldNote. */

#ifndef NOTEDATAWITHSCORING_H
#define NOTEDATAWITHSCORING_H

#include "GameConstantsAndTypes.h"
#include "NoteData.h"
#include "PlayerNumber.h"
#include <map>

struct RadarValues;

struct RowTrack: public pair<int,int>
{
	RowTrack( const HoldNote &hn ): pair<int,int>( hn.iEndRow, hn.iTrack ) { }
};

struct HoldNoteResult
{
	HoldNoteScore hns;

	/* 1.0 means this HoldNote has full life.
	 * 0.0 means this HoldNote is dead
	 * When this value hits 0.0 for the first time, m_HoldScore becomes HSS_NG.
	 * If the life is > 0.0 when the HoldNote ends, then m_HoldScore becomes HSS_OK. */
	float fLife;

	/* Last index where fLife was greater than 0.  If the tap was missed, this will
	 * be the first index of the hold. */
	int iLastHeldRow;

	HoldNoteResult()
	{
		hns = HNS_NONE;
		fLife = 1.0f;
		iLastHeldRow = 0;
	}
	HoldNoteResult( const HoldNote &hn );

	float GetLastHeldBeat() const { return NoteRowToBeat(iLastHeldRow); }
};

class NoteDataWithScoring : public NoteData
{
	// maintain this extra data in addition to the NoteData
	vector<TapNoteScore> m_TapNoteScores[MAX_NOTE_TRACKS];
	map<RowTrack, HoldNoteResult> m_HoldNoteScores;

	/* Offset, in seconds, for each tap grade.  Negative numbers mean the note
	 * was hit early; positive numbers mean it was hit late.  These values are
	 * only meaningful for graded taps (m_TapNoteScores >= TNS_BOO). */
	vector<float> m_TapNoteOffset[MAX_NOTE_TRACKS];

	map<RowTrack, float> m_fHoldNoteLife;

public:
	NoteDataWithScoring();
	void Init();

	// statistics
	int GetNumTapNotesWithScore( TapNoteScore tns, const float fStartBeat = 0, const float fEndBeat = -1 ) const;
	int GetNumNWithScore( TapNoteScore tns, int MinTaps, const float fStartBeat = 0, const float fEndBeat = -1 ) const;
	int GetNumHoldNotesWithScore( HoldNoteScore hns, const float fStartBeat = 0, const float fEndBeat = -1 ) const;
	int GetSuccessfulMines( const float fStartBeat = 0, const float fEndBeat = -1 ) const;
	int GetSuccessfulHands( const float fStartBeat = 0, const float fEndBeat = -1 ) const;
	TapNoteScore GetTapNoteScore(unsigned track, unsigned row) const;
	void SetTapNoteScore(unsigned track, unsigned row, TapNoteScore tns);
	float GetTapNoteOffset(unsigned track, unsigned row) const;
	void SetTapNoteOffset(unsigned track, unsigned row, float offset);

	HoldNoteScore GetHoldNoteScore( const HoldNote &hn ) const;
	void SetHoldNoteScore( const HoldNote &hn, HoldNoteScore hns );
	float GetHoldNoteLife( const HoldNote &hn ) const;
	void SetHoldNoteLife( const HoldNote &hn, float f );
	const HoldNoteResult GetHoldNoteResult( const HoldNote &hn ) const;
	HoldNoteResult *CreateHoldNoteResult( const HoldNote &hn );

	bool IsRowCompletelyJudged(unsigned row) const;
	TapNoteScore MinTapNoteScore(unsigned row) const;
	int LastTapNoteScoreTrack(unsigned row) const;
	TapNoteScore LastTapNoteScore(unsigned row) const;

	void GetActualRadarValues( PlayerNumber pn, float fSongSeconds, RadarValues& out ) const;

private:
	float GetActualStreamRadarValue( float fSongSeconds, PlayerNumber pn ) const;
	float GetActualVoltageRadarValue( float fSongSeconds, PlayerNumber pn ) const;
	float GetActualAirRadarValue( float fSongSeconds, PlayerNumber pn ) const;
	float GetActualFreezeRadarValue( float fSongSeconds, PlayerNumber pn ) const;
	float GetActualChaosRadarValue( float fSongSeconds, PlayerNumber pn ) const;
};

#endif

/*
 * (c) 2001-2004 Chris Danford, Glenn Maynard
 * All rights reserved.
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, and/or sell copies of the Software, and to permit persons to
 * whom the Software is furnished to do so, provided that the above
 * copyright notice(s) and this permission notice appear in all copies of
 * the Software and that both the above copyright notice(s) and this
 * permission notice appear in supporting documentation.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT OF
 * THIRD PARTY RIGHTS. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR HOLDERS
 * INCLUDED IN THIS NOTICE BE LIABLE FOR ANY CLAIM, OR ANY SPECIAL INDIRECT
 * OR CONSEQUENTIAL DAMAGES, OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS
 * OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR
 * OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 */
