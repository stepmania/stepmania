/* ScoreKeeperMAX2 - MAX2-style scorekeeping. */

#ifndef SCOREKEEPER_MAX2_H
#define SCOREKEEPER_MAX2_H

#include "ScoreKeeper.h"
#include "NoteDataWithScoring.h"
class Steps;

class ScoreKeeperMAX2: public ScoreKeeper
{
	int				m_iScoreRemainder;
	int				m_iMaxPossiblePoints;
	int				m_iTapNotesHit;	// number of notes judged so far, needed by scoring

	int				m_iNumTapsAndHolds;
	int			    m_iMaxScoreSoFar; // for nonstop scoring
	int				m_iPointBonus; // the difference to award at the end
 	int				m_iCurToastyCombo;
	bool			m_bIsLastSongInCourse;

	const vector<Steps*>& apSteps;

	void AddScore( TapNoteScore score );

	/* Configuration: */
	/* Score after each tap will be rounded to the nearest m_iRoundTo; 1 to do nothing. */
	int				m_iRoundTo;
	int				m_ComboBonusFactor[NUM_TAP_NOTE_SCORES];

public:
	ScoreKeeperMAX2( const vector<Song*>& apSongs, const vector<Steps*>& apSteps, const vector<AttackArray> &asModifiers, PlayerNumber pn);

	// before a song plays (called multiple times if course)
	void OnNextSong( int iSongInCourseIndex, const Steps* pSteps, const NoteData* pNoteData );

	void HandleTapScore( TapNoteScore score );
	void HandleTapRowScore( TapNoteScore scoreOfLastTap, int iNumTapsInRow );
	void HandleHoldScore( HoldNoteScore holdScore, TapNoteScore tapScore );

	// This must be calculated using only cached radar values so that we can 
	// do it quickly.
	static int	GetPossibleDancePoints( const RadarValues& fRadars );
	static int	GetPossibleDancePoints( const RadarValues& fOriginalRadars, const RadarValues& fPostRadars );

private:
	static int TapNoteScoreToDancePoints( TapNoteScore tns );
	static int HoldNoteScoreToDancePoints( HoldNoteScore hns );

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
