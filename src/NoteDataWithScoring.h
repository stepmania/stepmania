#ifndef NOTE_DATA_WITH_SCORING_H
#define NOTE_DATA_WITH_SCORING_H

#include "PlayerNumber.h"
#include "GameConstantsAndTypes.h"

struct RadarValues;
class NoteData;
class PlayerStageStats;
struct TapNote;

/** @brief NoteData with scores for each TapNote and HoldNote. */
namespace NoteDataWithScoring
{
	/**
	 * @brief Has the current row of NoteData been judged completely?
	 * @param in the entire Notedata.
	 * @param iRow the row to check.
	 * @plnum If valid, only consider notes for that PlayerNumber
	 * @return true if it has been completley judged, or false otherwise. */
	bool IsRowCompletelyJudged( const NoteData &in, unsigned iRow, PlayerNumber plnum = PlayerNumber_Invalid );
	TapNoteScore MinTapNoteScore( const NoteData &in, unsigned iRow, PlayerNumber plnum = PlayerNumber_Invalid );
	const TapNote &LastTapNoteWithResult( const NoteData &in, unsigned iRow, PlayerNumber plnum = PlayerNumber_Invalid );

	void GetActualRadarValues(const NoteData &in, const PlayerStageStats &pss,
		float song_seconds, RadarValues& out);
};

#endif

/**
 * @file
 * @author Chris Danford, Glenn Maynard (c) 2001-2004
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
 * 
 * (c) 2016- Electromuis, Anton Grootes
 * This branch of https://github.com/stepmania/stepmania
 * will from here on out be released as GPL v3 (wich converts from the previous MIT license)
 */
