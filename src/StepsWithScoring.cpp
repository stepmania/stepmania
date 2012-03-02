#include "global.h"
#include "StepsWithScoring.h"
#include "NoteDataWithScoring.h" // For things the Steps don't need to do.
#include "NoteData.h"
#include "PlayerStageStats.h"
#include "GameConstantsAndTypes.h"
#include "ThemeMetric.h"
#include "RageLog.h"

bool StepsWithScoring::IsRowCompletelyJudged( const NoteData &in, unsigned row, StepsTypeCategory stc, PlayerNumber pn )
{
	TapNoteScore tns;
	switch (stc)
	{
		case StepsTypeCategory_Couple:
		{
			const int tracksPerPlayer = in.GetNumTracks() / 2;
			int start = pn * tracksPerPlayer;
			tns = NoteDataWithScoring::MinTapNoteScore(in, row, start, start + tracksPerPlayer, PLAYER_INVALID);
			break;
		}
		case StepsTypeCategory_Routine:
		{
			tns = NoteDataWithScoring::MinTapNoteScore(in, row, 0, in.GetNumTracks(), pn);
			break;
		}
		default:
		{
			tns = NoteDataWithScoring::MinTapNoteScore(in, row);
		}
	}
	return tns >= TNS_Miss;
}
