#include "global.h"
#include "StepsWithScoring.h"
#include "NoteDataWithScoring.h" // For things the Steps don't need to do.
#include "NoteData.h"
#include "PlayerStageStats.h"
#include "GameConstantsAndTypes.h"
#include "ThemeMetric.h"
#include "Steps.h"
#include "RageLog.h"

static ThemeMetric<TapNoteScore> MIN_SCORE_TO_MAINTAIN_COMBO( "Gameplay", "MinScoreToMaintainCombo" );

const TapNote &StepsWithScoring::LastTapNoteWithResult( const NoteData &in, unsigned row, StepsTypeCategory stc, PlayerNumber pn )
{
	int track;
	switch (stc)
	{
		case StepsTypeCategory_Couple:
		{
			const int tracksPerPlayer = in.GetNumTracks() / 2;
			int start = pn * tracksPerPlayer;
			track = NoteDataWithScoring::LastTapNoteScoreTrack(in, row, start, start + tracksPerPlayer, PLAYER_INVALID);
			break;
		}
		case StepsTypeCategory_Routine:
		{
			track = NoteDataWithScoring::LastTapNoteScoreTrack(in, row, 0, in.GetNumTracks(), pn);
			break;
		}
		default:
		{
			track = NoteDataWithScoring::LastTapNoteScoreTrack(in, row, 0, in.GetNumTracks(), PLAYER_INVALID);
		}
	}
	if( track == -1 )
		return TAP_EMPTY;

	//LOG->Trace( ssprintf("returning in.GetTapNote(iTrack=%i, iRow=%i)", iTrack, iRow) );
	return in.GetTapNote( track, row );
}

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

int GetNumHoldNotesWithScore(const NoteData &in,
	TapNote::SubType subType,
	HoldNoteScore hns,
	int start,
	int last,
	PlayerNumber pn = PLAYER_INVALID)
{
	ASSERT( subType != TapNote::SubType_Invalid );

	int iNumSuccessfulHolds = 0;
	for (int t = start; t < last; ++t )
	{
		NoteData::TrackMap::const_iterator begin, end;
		in.GetTapNoteRange( t, 0, MAX_NOTE_ROW, begin, end );

		for( ; begin != end; ++begin )
		{
			const TapNote &tn = begin->second;
			if( tn.type != TapNote::hold_head )
				continue;
			if( tn.subType != subType )
				continue;
			// Routine player mode check.
			if( tn.pn != PLAYER_INVALID && tn.pn != pn && pn != PLAYER_INVALID )
				continue;
			if( tn.HoldResult.hns == hns )
				++iNumSuccessfulHolds;
		}
	}
	return iNumSuccessfulHolds;
}

int GetNumTapNotesWithScore(const NoteData &in,
	TapNoteScore tns,
	int first,
	int last,
	PlayerNumber pn = PLAYER_INVALID)
{ 
	int iNumSuccessfulTapNotes = 0;
	for (int t = first; t < last; ++t )
	{
		FOREACH_NONEMPTY_ROW_IN_TRACK_RANGE( in, t, r, 0, MAX_NOTE_ROW )
		{
			const TapNote &tn = in.GetTapNote(t, r);
			// Routine player mode check.
			if( tn.pn != PLAYER_INVALID && tn.pn != pn && pn != PLAYER_INVALID )
				continue;
			if( tn.result.tns >= tns )
				iNumSuccessfulTapNotes++;
		}
	}
	return iNumSuccessfulTapNotes;
}

int GetNumNWithScore(const Steps *in,
	TapNoteScore tns,
	int MinTaps,
	PlayerNumber pn = PLAYER_INVALID)
{
	int iNumSuccessfulDoubles = 0;
	const NoteData &nd = in->GetNoteData();
	FOREACH_NONEMPTY_ROW_ALL_TRACKS_RANGE( nd, r, 0, MAX_NOTE_ROW )
	{
		vector<int> notesInRow = in->GetNumTracksWithTapOrHoldHead(r);
		int playerNotes = notesInRow[0];
		if (pn != PLAYER_INVALID && in->IsMultiPlayerStyle())
		{
			playerNotes = notesInRow[1];
		}
		TapNoteScore tnsRow = StepsWithScoring::LastTapNoteWithResult(nd, r, in->GetStepsTypeCategory(), pn).result.tns;

		if( playerNotes >= MinTaps && tnsRow >= tns )
			iNumSuccessfulDoubles++;
	}

	return iNumSuccessfulDoubles;
}

namespace // radar calculation namespace
{

// Return the ratio of actual to possible Bs.
float GetActualStreamRadarValue( const Steps *in, PlayerNumber pn )
{
	vector<int> allSteps = in->GetNumTapNotes();
	if (allSteps.size() == 1)
	{
		allSteps.push_back(allSteps[0]);
	}
	int possibleSteps = allSteps[pn];
	if (possibleSteps == 0)
	{
		return 1.0f;
	}

	int w2s = 0;
	const NoteData &nd = in->GetNoteData();
	switch (in->GetStepsTypeCategory())
	{
		case StepsTypeCategory_Couple:
		{
			int perPlayer = nd.GetNumTracks() / 2;
			int start = pn * perPlayer;
			int end = (pn + 1) * perPlayer;
			w2s = GetNumTapNotesWithScore(nd, TNS_W2, start, end);
			break;
		}
		case StepsTypeCategory_Routine:
		{
			w2s = GetNumTapNotesWithScore(nd, TNS_W2, 0, nd.GetNumTracks(), pn);
			break;
		}
		default:
		{
			w2s = GetNumTapNotesWithScore(nd, TNS_W2, 0, nd.GetNumTracks());
		}
	}
	return clamp( static_cast<float>(w2s)/possibleSteps, 0.0f, 1.0f );
}

// Return the ratio of actual combo to max combo.
float GetActualVoltageRadarValue(const PlayerStageStats &pss)
{
	/* STATSMAN->m_CurStageStats.iMaxCombo is unrelated to GetNumTapNotes:
	 * m_bComboContinuesBetweenSongs might be on, and the way combo is counted
	 * varies depending on the mode and score keeper. Instead, let's use the
	 * length of the longest recorded combo. This is only subtly different:
	 * it's the percent of the song the longest combo took to get. */
	const PlayerStageStats::Combo_t MaxCombo = pss.GetMaxCombo();
	float fComboPercent = SCALE( MaxCombo.m_fSizeSeconds, 0, pss.m_fLastSecond-pss.m_fFirstSecond, 0.0f, 1.0f );
	return clamp( fComboPercent, 0.0f, 1.0f );
}

// Return the ratio of actual to possible W2s on jumps.
float GetActualAirRadarValue( const Steps *in, PlayerNumber pn )
{
	vector<int> allJumps = in->GetNumJumps();
	if (allJumps.size() == 1)
	{
		allJumps.push_back(allJumps[0]);
	}
	const int possibleJumps = allJumps[pn];
	if (possibleJumps == 0)
	{
		return 1.0f; // no jumps for the player.
	}

	// number of doubles
	int w2s = GetNumNWithScore(in, TNS_W2, 2, pn);
	return clamp( static_cast<float>(w2s) / possibleJumps, 0.0f, 1.0f );
}

// Return the ratio of actual to possible successful holds.
float GetActualFreezeRadarValue( const Steps *in, PlayerNumber pn )
{
	// number of hold steps
	vector<int> totalHolds = in->GetNumHoldNotes();
	if (totalHolds.size() == 1)
	{
		totalHolds.push_back(totalHolds[0]);
	}
	int possibleHolds = totalHolds[pn];
	if (possibleHolds == 0)
	{
		return 1.0f;
	}

	/*
	 * XXX: The Freeze value requires getting the total number of holds, but added
	 * the results of rolls, which are not a part of this. This made no sense.
	 */
	const NoteData &nd = in->GetNoteData();
	int start = 0;
	int end = nd.GetNumTracks();
	if (in->GetStepsTypeCategory() == StepsTypeCategory_Couple)
	{
		int perPlayer = end / 2;
		start = pn * perPlayer;
		end = (pn + 1) * perPlayer;
		pn = PLAYER_INVALID;
	}
	else if (in->GetStepsTypeCategory() != StepsTypeCategory_Routine)
	{
		pn = PLAYER_INVALID;
	}

	int actualHolds = GetNumHoldNotesWithScore(nd, TapNote::hold_head_hold, HNS_Held, start, end, pn);
	return clamp( float(actualHolds) / possibleHolds, 0.0f, 1.0f );
}

// Return the ratio of actual to possible dance points.
float GetActualChaosRadarValue( const PlayerStageStats &pss )
{
	const int iPossibleDP = pss.m_iPossibleDancePoints;
	if ( iPossibleDP == 0 )
		return 1;

	const int ActualDP = pss.m_iActualDancePoints;
	return clamp( float(ActualDP)/iPossibleDP, 0.0f, 1.0f );
}

}

int GetSuccessfulLifts(const NoteData &in,
	TapNoteScore tns,
	int firstTrack,
	int lastTrack,
	PlayerNumber pn = PLAYER_INVALID,
	int firstRow = 0,
	int lastRow = MAX_NOTE_ROW)
{
	int successfulLifts = 0;
	for (int t = firstTrack; t < lastTrack; ++t)
	{
		FOREACH_NONEMPTY_ROW_IN_TRACK_RANGE(in, t, r, firstRow, lastRow)
		{
			const TapNote &tn = in.GetTapNote(t, r);
			if (tn.type != TapNote::lift)
				continue;
			if (tn.pn != PLAYER_INVALID && tn.pn != pn && pn != PLAYER_INVALID)
				continue;
			if (tn.result.tns >= tns)
				++successfulLifts;
		}
	}
	return successfulLifts;
}

int GetSuccessfulMines(const NoteData &in,
	int firstTrack,
	int lastTrack,
	PlayerNumber pn = PLAYER_INVALID,
	int firstRow = 0,
	int lastRow = MAX_NOTE_ROW)
{
	int successes = 0;
	for (int t = firstTrack; t < lastTrack; ++t)
	{
		FOREACH_NONEMPTY_ROW_IN_TRACK_RANGE(in, t, r, firstRow, lastRow)
		{
			const TapNote &tn = in.GetTapNote(t, r);
			if (tn.type != TapNote::mine)
				continue;
			if (tn.pn != PLAYER_INVALID && tn.pn != pn && pn != PLAYER_INVALID)
				continue;
			if (tn.result.tns == TNS_AvoidMine)
				++successes;
		}
	}
	return successes;
}

int GetSuccessfulHands(const Steps *in,
	int firstTrack,
	int lastTrack,
	PlayerNumber pn = PLAYER_INVALID,
	int firstRow = 0,
	int lastRow = MAX_NOTE_ROW)
{
	int iNum = 0;
	const NoteData &nd = in->GetNoteData();
	// TODO: Better way of handling routine check within this function.
	PlayerNumber routinePN = (in->GetStepsTypeCategory() == StepsTypeCategory_Routine) ? pn : PLAYER_INVALID;
	FOREACH_NONEMPTY_ROW_ALL_TRACKS_RANGE(nd, r, firstRow, lastRow)
	{
		vector<bool> needsHands = in->RowNeedsAtLeastSimultaneousPresses(3, r);
		bool doesPlayerNeedHand = needsHands[0];
		// TODO: Better way of phrasing this.
		if (needsHands.size() == NUM_PLAYERS && pn == PLAYER_2)
		{
			doesPlayerNeedHand = needsHands[1];
		}

		if(!doesPlayerNeedHand)
			continue;

		bool Missed = false;
		for (int t = firstTrack; t < lastTrack; ++t)
		{
			const TapNote &tn = nd.GetTapNote(t, r);
			if( tn.type == TapNote::empty )
				continue;
			if( tn.type == TapNote::mine ) // mines don't count
				continue;
			if (tn.type == TapNote::fake ) // fake arrows don't count
				continue;
			// routine mode check.
			if (tn.pn != PLAYER_INVALID && tn.pn != routinePN && routinePN != PLAYER_INVALID)
				continue;
			if( tn.result.tns <= TNS_W5 )
				Missed = true;
		}

		if( Missed )
			continue;

		// Check hold scores.
		for (int t = firstTrack; t < lastTrack; ++t)
		{
			int iHeadRow;
			if( !nd.IsHoldNoteAtRow( t, r, &iHeadRow ) )
				continue;
			const TapNote &tn = nd.GetTapNote( t, iHeadRow );
			// routine mode check.
			if (tn.pn != PLAYER_INVALID && tn.pn != routinePN && routinePN != PLAYER_INVALID)
				continue;

			/* If a hold is released *after* a hand containing it, the hand is
			 * still good. Ignore the judgement and only examine iLastHeldRow
			 * to be sure that the hold was still held at the point of this row.
			 * (Note that if the hold head tap was missed, then iLastHeldRow == i
			 * and this won't fail--but the tap check above will have already failed.) */
			if( tn.HoldResult.iLastHeldRow < r )
				Missed = true;
		}

		if( !Missed )
			iNum++;
	}

	return iNum;
}

RadarValues StepsWithScoring::GetActualRadarValues(const Steps *in,
	const PlayerStageStats &pss,
	float fSongSeconds,
	PlayerNumber pn)
{
	RadarValues rv;
	// The for loop and the assert are used to ensure that all fields of 
	// RadarValue get set in here.
	vector<int> statResults;
	vector<float> radarResults;
	const StepsTypeCategory stc = in->GetStepsTypeCategory();
	const NoteData &nd = in->GetNoteData();

	/*
	 * TODO: Optimization opportunity. Some of these calls are repeated in
	 * both stat and radar based categories. For loops may not be needed. */
	FOREACH_ENUM( RadarCategory, rc )
	{
		switch( rc )
		{
			case RadarCategory_Stream:
			{
				rv[rc] = GetActualStreamRadarValue( in, pn );
				break;
			}
			case RadarCategory_Voltage:
			{
				rv[rc] = GetActualVoltageRadarValue(pss);
				break;
			}
			case RadarCategory_Air:
			{
				rv[rc] = GetActualAirRadarValue( in, pn );
				break;
			}
			case RadarCategory_Freeze:
			{
				rv[rc] = GetActualFreezeRadarValue( in, pn );
				break;
			}
			case RadarCategory_Chaos:
			{
				rv[rc] = GetActualChaosRadarValue(pss);
				break;
			}
			case RadarCategory_TapsAndHolds:
			case RadarCategory_Jumps:
			{
				int num = (rc == RadarCategory_TapsAndHolds) ? 1 : 2;
				rv[rc] = GetNumNWithScore(in, TNS_W4, num, pn);
				break;
			}
			case RadarCategory_Holds:
			case RadarCategory_Rolls:
			{
				TapNote::SubType sub = (rc == RadarCategory_Holds) ? TapNote::hold_head_hold : TapNote::hold_head_roll;
				switch (stc)
				{
					case StepsTypeCategory_Couple:
					{
						int perPlayer = nd.GetNumTracks() / 2;
						int start = pn * perPlayer;
						int end = (pn + 1) * perPlayer;
						rv[rc] = GetNumHoldNotesWithScore(nd, sub, HNS_Held, start, end);
						break;
					}
					case StepsTypeCategory_Routine:
					{
						rv[rc] = GetNumHoldNotesWithScore(nd, sub, HNS_Held, 0, nd.GetNumTracks(), pn);
						break;
					}
					default:
					{
						rv[rc] = GetNumHoldNotesWithScore(nd, sub, HNS_Held, 0, nd.GetNumTracks());
					}
				}
				break;
			}
			case RadarCategory_Mines:
			{
				switch (stc)
				{
					case StepsTypeCategory_Couple:
					{
						int perPlayer = nd.GetNumTracks() / 2;
						int start = pn * perPlayer;
						int end = (pn + 1) * perPlayer;
						rv[rc] = GetSuccessfulMines(nd, start, end);
						break;
					}
					case StepsTypeCategory_Routine:
					{
						rv[rc] = GetSuccessfulMines(nd, 0, nd.GetNumTracks(), pn);
						break;
					}
					default:
					{
						rv[rc] = GetSuccessfulMines(nd, 0, nd.GetNumTracks());
					}
				}
				break;
			}
			case RadarCategory_Hands:
				{
					switch (stc)
					{
						case StepsTypeCategory_Couple:
						{
							int perPlayer = nd.GetNumTracks() / 2;
							int start = pn * perPlayer;
							int end = (pn + 1) * perPlayer;
							rv[rc] = GetSuccessfulHands(in, start, end, pn); // we need the PN this time.
							break;
						}
						case StepsTypeCategory_Routine:
						{
							rv[rc] = GetSuccessfulHands(in, 0, nd.GetNumTracks(), pn);
							break;
						}
						default:
						{
							rv[rc] = GetSuccessfulHands(in, 0, nd.GetNumTracks());
						}
					}
					break;
				}
			case RadarCategory_Lifts:
			{
				switch (stc)
				{
					case StepsTypeCategory_Couple:
					{
						int perPlayer = nd.GetNumTracks() / 2;
						int start = pn * perPlayer;
						int end = (pn + 1) * perPlayer;
						rv[rc] = GetSuccessfulLifts(nd, MIN_SCORE_TO_MAINTAIN_COMBO, start, end);
						break;
					}
					case StepsTypeCategory_Routine:
					{
						rv[rc] = GetSuccessfulLifts(nd, MIN_SCORE_TO_MAINTAIN_COMBO, 0, nd.GetNumTracks(), pn);
						break;
					}
					default:
					{
						rv[rc] = GetSuccessfulLifts(nd, MIN_SCORE_TO_MAINTAIN_COMBO, 0, nd.GetNumTracks());
					}
				}
				break;
			}
			case RadarCategory_Fakes:
			{
				statResults = in->GetNumFakes();
				if (statResults.size() == 1)
				{
					statResults.push_back(statResults[0]);
				}
				rv[rc] = statResults[pn];
				break;
			}
		DEFAULT_FAIL( rc );
		}
	}
	return rv;
}
