#ifndef STEPS_WITH_SCORING_H
#define STEPS_WITH_SCORING_H

#include "PlayerNumber.h"
#include "GameConstantsAndTypes.h"

struct RadarValues;
class Steps;
class NoteData;
class PlayerStageStats;
struct TapNote;

/** @brief Steps with scores for each TapNote and HoldNote. */
namespace StepsWithScoring
{
	/**
	 * @brief Has the current row of NoteData been judged completely?
	 * @param in the NoteData in question.
	 * @param iRow the row to check.
	 * @param stc the StepsTypeCategory of this NoteData/Steps.
	 * @param pn the PlayerNumber (should it be relevant).
	 * @return true if it has been completley judged, or false otherwise. */
	bool IsRowCompletelyJudged( const NoteData &in, unsigned iRow, StepsTypeCategory stc, PlayerNumber pn = PLAYER_INVALID );

	/**
	 * @brief Retrieve the last tap note that had a result associated with it.
	 * @param in the NoteData in question.
	 * @param iRow the row to check.
	 * @param stc the StepsTypeCategory of this NoteData/Steps.
	 * @param pn the PlayerNumber (should it be relevant).
	 * @return the TapNote last judged. */
	const TapNote &LastTapNoteWithResult( const NoteData &in, unsigned iRow, StepsTypeCategory stc, PlayerNumber pn = PLAYER_INVALID );

	void GetActualRadarValues( const NoteData &in, const PlayerStageStats &pss, 
				  float fSongSeconds, RadarValues& out );
};

#endif

/**
 * @file
 * @author Chris Danford, Glenn Maynard, Jason Felds (c) 2001-2012
 * @section LICENSE
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