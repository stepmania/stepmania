#include "global.h"
#include "StepsWithScoring.h"
#include "NoteDataWithScoring.h" // For things the Steps don't need to do.
#include "NoteData.h"
#include "PlayerStageStats.h"
#include "GameConstantsAndTypes.h"
#include "ThemeMetric.h"
#include "RageLog.h"

// TODO: Combine this function and NoteDataWithScoring's MinTapNoteScore. The code is errily similar.
TapNoteScore StepsWithScoring::MinTapNoteScoreCouple( const NoteData &in, unsigned row, PlayerNumber pn )
{
	TapNoteScore score = TNS_W1;
	const int tracks = in.GetNumTracks();
	for( int t = pn * (tracks / 2); t < (pn + 1) * (tracks / 2); ++t )
	{
		// Ignore mines (and fake arrows), or the score will always be TNS_None.
		const TapNote &tn = in.GetTapNote( t, row );
		if (tn.type == TapNote::empty ||
			tn.type == TapNote::mine ||
			tn.type == TapNote::fake ||
			tn.type == TapNote::autoKeysound)
			continue;
		score = min( score, tn.result.tns );
	}

	return score;
}

TapNoteScore StepsWithScoring::MinTapNoteScoreRoutine( const NoteData &in, unsigned row, PlayerNumber pn )
{
	TapNoteScore score = TNS_W1;
	for (int t = 0; t < in.GetNumTracks(); ++t)
	{
		const TapNote &tn = in.GetTapNote(t, row);
		if (tn.pn != pn || 
			tn.type == TapNote::empty ||
			tn.type == TapNote::mine ||
			tn.type == TapNote::fake ||
			tn.type == TapNote::autoKeysound)
			continue;
		score = min(score, tn.result.tns);
	}
	return score;
}

bool StepsWithScoring::IsRowCompletelyJudged( const NoteData &in, unsigned row, StepsTypeCategory stc, PlayerNumber pn )
{
	TapNoteScore tns;
	switch (stc)
	{
		case StepsTypeCategory_Couple:
		{
			tns = StepsWithScoring::MinTapNoteScoreCouple(in, row, pn);
			break;
		}
		case StepsTypeCategory_Routine:
		{
			tns = StepsWithScoring::MinTapNoteScoreRoutine(in, row, pn);
			break;
		}
		default:
		{
			tns = NoteDataWithScoring::MinTapNoteScore(in, row);
		}
	}
	return tns >= TNS_Miss;
}
