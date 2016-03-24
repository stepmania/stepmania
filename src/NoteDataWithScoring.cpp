#include "global.h"
#include "NoteDataWithScoring.h"
#include "NoteData.h"
#include "PlayerStageStats.h"
#include "Game.h"
#include "GameConstantsAndTypes.h"
#include "GameState.h"
#include "ThemeMetric.h"
#include "RageLog.h"
#include "TimingData.h"

namespace
{

//ThemeMetric<TapNoteScoreJudgeType> LAST_OR_MINIMUM_TNS	("Gameplay","LastOrMinimumTapNoteScore");
	static ThemeMetric<TapNoteScore> MIN_SCORE_TO_MAINTAIN_COMBO( "Gameplay", "MinScoreToMaintainCombo" );

/* Return the last tap score of a row: the grade of the tap that completed
 * the row.  If the row has no tap notes, return -1.  If any tap notes aren't
 * graded (any tap is TNS_None) or are missed (TNS_Miss), return it. */
int LastTapNoteScoreTrack( const NoteData &in, unsigned iRow, PlayerNumber pn )
{
	float scoretime = -9999;
	int best_track = -1;
	for( int t=0; t<in.GetNumTracks(); t++ )
	{
		/* Skip empty tracks and mines */
		const TapNote &tn = in.GetTapNote( t, iRow );
		if (tn.type == TapNoteType_Empty ||
			tn.type == TapNoteType_Mine ||
			tn.type == TapNoteType_Fake ||
			tn.type == TapNoteType_AutoKeysound) 
			continue;
		if( tn.pn != PLAYER_INVALID && tn.pn != pn && pn != PLAYER_INVALID )
			continue;

		TapNoteScore tns = tn.result.tns;
		
		if( tns == TNS_Miss || tns == TNS_None )
			return t;

		float tm = tn.result.fTapNoteOffset;
		if(tm < scoretime) continue;

		scoretime = tm;
		best_track = t;
	}

	return best_track;
}

/* Return the minimum tap score of a row: the lowest grade of the tap in the row.
 * If the row isn't complete (not all taps have been hit),
 * return TNS_NONE or TNS_MISS. */
#if 0
int MinTapNoteScoreTrack( const NoteData &in, unsigned iRow, PlayerNumber pn )
{
	// work in progress
	float scoretime = -9999;
	int worst_track = -1;
	TapNoteScore lowestTNS = TapNoteScore_Invalid;
	for( int t=0; t<in.GetNumTracks(); t++ )
	{
		// Skip empty tracks and mines
		const TapNote &tn = in.GetTapNote( t, iRow );
		if (tn.type == TapNoteType_Empty ||
			tn.type == TapNoteType_Mine ||
			tn.type == TapNoteType_Fake ||
			tn.type == TapNoteType_AutoKeysound) 
			continue;
		if( tn.pn != PLAYER_INVALID && tn.pn != pn && pn != PLAYER_INVALID )
			continue;

		TapNoteScore tns = tn.result.tns;

		if( tns == TNS_Miss || tns == TNS_None )
			return t;

		float tm = tn.result.fTapNoteOffset;
		if(tm > scoretime) continue; // huh -aj

		// enum compare against lowestTNS here
		//if( tns < lowestTNS ) continue;

		scoretime = tm;
		worst_track = t;
	}

	return worst_track;
}
#endif

}

const TapNote &NoteDataWithScoring::LastTapNoteWithResult( const NoteData &in, unsigned iRow, PlayerNumber plnum )
{
	// Allow this to be configurable between LastTapNoteScoreTrack and
	// MinTapNoteScore; this change inspired by PumpMania (Zmey, et al) -aj
	/*
	LOG->Trace( ssprintf("hi i'm NoteDataWithScoring::LastTapNoteWithResult(NoteData in, iRow=%i, PlayerNumber pn)", iRow) );
	int iTrack = 0;
	switch(LAST_OR_MINIMUM_TNS)
	{
		case TapNoteScoreJudgeType_MinimumScore:
			iTrack = MinTapNoteScoreTrack( in, iRow, pn );
			LOG->Trace( ssprintf("TapNoteScoreJudgeType_MinimumScore omg iTrack is %i and iRow is %i",iTrack,iRow) );
			break;
		case TapNoteScoreJudgeType_LastScore:
		default:
			iTrack = LastTapNoteScoreTrack( in, iRow, pn );
			break;
	}
	*/
	int iTrack = LastTapNoteScoreTrack( in, iRow, plnum );
	if( iTrack == -1 )
		return TAP_EMPTY;

	//LOG->Trace( ssprintf("returning in.GetTapNote(iTrack=%i, iRow=%i)", iTrack, iRow) );
	return in.GetTapNote( iTrack, iRow );
}

/* Return the minimum tap score of a row.  If the row isn't complete (not all
 * taps have been hit), return TNS_None or TNS_Miss. */
TapNoteScore NoteDataWithScoring::MinTapNoteScore( const NoteData &in, unsigned row, PlayerNumber plnum )
{
	//LOG->Trace("Hey I'm NoteDataWithScoring::MinTapNoteScore");
	TapNoteScore score = TNS_W1;
	for( int t=0; t<in.GetNumTracks(); t++ )
	{
		// Ignore mines (and fake arrows), or the score will always be TNS_None.
		const TapNote &tn = in.GetTapNote( t, row );
		if (tn.type == TapNoteType_Empty ||
			tn.type == TapNoteType_Mine ||
			tn.type == TapNoteType_Fake ||
			tn.type == TapNoteType_AutoKeysound ||
			( plnum != PlayerNumber_Invalid && tn.pn != plnum ) )
			continue;
		score = min( score, tn.result.tns );
	}

	//LOG->Trace( ssprintf("OMG score is?? %s",TapNoteScoreToString(score).c_str()) );
	return score;
}

bool NoteDataWithScoring::IsRowCompletelyJudged( const NoteData &in, unsigned row, PlayerNumber plnum )
{
	return MinTapNoteScore( in, row, plnum ) >= TNS_Miss;
}

namespace
{
// Return the ratio of actual combo to max combo.
float GetActualVoltageRadarValue( const NoteData &in, float fSongSeconds, const PlayerStageStats &pss )
{
	/* STATSMAN->m_CurStageStats.iMaxCombo is unrelated to GetNumTapNotes:
	 * m_bComboContinuesBetweenSongs might be on, and the way combo is counted
	 * varies depending on the mode and score keeper. Instead, let's use the
	 * length of the longest recorded combo. This is only subtly different:
	 * it's the percent of the song the longest combo took to get. */
	const PlayerStageStats::Combo_t MaxCombo = pss.GetMaxCombo();
	float fComboPercent = SCALE(MaxCombo.m_fSizeSeconds, 0, fSongSeconds, 0.0f, 1.0f);
	return clamp( fComboPercent, 0.0f, 1.0f );
}

// Return the ratio of actual to possible dance points.
float GetActualChaosRadarValue( const NoteData &in, float fSongSeconds, const PlayerStageStats &pss )
{
	const int iPossibleDP = pss.m_iPossibleDancePoints;
	if ( iPossibleDP == 0 )
		return 1;

	const int ActualDP = pss.m_iActualDancePoints;
	return clamp( float(ActualDP)/iPossibleDP, 0.0f, 1.0f );
}
}

struct hold_status
{
	int end_row;
	int last_held_row;
	hold_status(int e, int l)
		:end_row(e), last_held_row(l)
	{}
};

struct garv_state
{
	int curr_row;
	int notes_hit_for_stream;
	int jumps_hit_for_air;
	int holds_held;
	int rolls_held;
	int notes_hit;
	int taps_hit;
	int jumps_hit;
	int hands_hit;
	int mines_avoided;
	int lifts_hit;
	// hold_ends tracks where currently active holds will end, which is used
	// to count the number of hands. -Kyz
	vector<hold_status> hold_ends;
	int num_notes_on_curr_row;
	// num_holds_on_curr_row saves us the work of tracking where holds started
	// just to keep a jump of two holds from counting as a hand.
	int num_holds_on_curr_row;
	int num_notes_hit_on_curr_row;
	// last_tns_on_row and last_time_on_row are used for deciding whether a jump
	// or hand was successfully hit.
	TapNoteScore last_tns_on_row;
	float last_time_on_row;
	// A hand is considered missed if any of the notes is missed.
	TapNoteScore worst_tns_on_row;
	// TODO?  Make these configurable in some way?
	TapNoteScore stream_tns;
	TapNoteScore air_tns;
	TapNoteScore taps_tns;
	TapNoteScore jumps_tns;
	TapNoteScore hands_tns;
	TapNoteScore lifts_tns;
	bool judgable;
	garv_state()
		:curr_row(-1), notes_hit_for_stream(0), jumps_hit_for_air(0),
		 holds_held(0), rolls_held(0), notes_hit(0), taps_hit(0), jumps_hit(0),
		 hands_hit(0), mines_avoided(0), lifts_hit(0), num_notes_on_curr_row(0),
		 num_holds_on_curr_row(0), num_notes_hit_on_curr_row(0),
		 last_tns_on_row(TapNoteScore_Invalid), last_time_on_row(-9999),
		 worst_tns_on_row(TapNoteScore_Invalid), stream_tns(TNS_W2),
		 air_tns(TNS_W2), taps_tns(TNS_W4), jumps_tns(TNS_W4), hands_tns(TNS_W4),
		 lifts_tns(MIN_SCORE_TO_MAINTAIN_COMBO),
		 judgable(false)
	{}
};

static void DoRowEndRadarActualCalc(garv_state& state, RadarValues& out)
{
	if(state.judgable && state.last_tns_on_row != TapNoteScore_Invalid)
	{
		if(state.num_notes_on_curr_row >= 1)
		{
			state.taps_hit+= (state.last_tns_on_row >= state.taps_tns);
		}
		if(state.num_notes_on_curr_row >= 2)
		{
			state.jumps_hit_for_air+= (state.last_tns_on_row >= state.air_tns);
			state.jumps_hit+= (state.last_tns_on_row >= state.jumps_tns);
		}
		if(state.num_notes_on_curr_row + (state.hold_ends.size() -
				state.num_holds_on_curr_row) >= 3)
		{
			if(state.worst_tns_on_row >= state.hands_tns)
			{
				size_t holds_down= 0;
				for(size_t n= 0; n < state.hold_ends.size(); ++n)
				{
					holds_down+= (state.curr_row <= state.hold_ends[n].last_held_row);
				}
				state.hands_hit+= (holds_down == state.hold_ends.size());
			}
		}
	}
}

static void UpdateHittable(int curr_row, int& first, int& last)
{
	if(first == -1)
	{
		first= curr_row;
	}
	last= curr_row;
}

void NoteDataWithScoring::GetActualRadarValues(const NoteData &in,
	const PlayerStageStats &pss, float song_seconds, RadarValues& out)
{
	// Anybody editing this function should also examine
	// NoteDataUtil::CalculateRadarValues to make sure it handles things the
	// same way.
	// Some of this logic is similar or identical to
	// NoteDataUtil::CalculateRadarValues because I couldn't figure out a good
	// way to combine them into one. -Kyz
	PlayerNumber pn= pss.m_player_number;
	garv_state state;

	NoteData::all_tracks_const_iterator curr_note=
		in.GetTapNoteRangeAllTracks(0, MAX_NOTE_ROW);
	TimingData* timing= GAMESTATE->GetProcessedTimingData();
	// first_hittable_row and last_hittable_row exist so that
	// GetActualVoltageRadarValue can be passed the correct song length.
	// GetActualVoltageRadarValue scores based on the max combo, a full combo
	// is a full voltage score.  The song length is used instead of trying to
	// figure out the max combo for the song because rolls mean there isn't a
	// limit to the max combo. -Kyz
	int first_hittable_row= -1;
	int last_hittable_row= -1;
	bool tick_holds= GAMESTATE->GetCurrentGame()->m_bTickHolds;

	while(!curr_note.IsAtEnd())
	{
		if(curr_note.Row() != state.curr_row)
		{
			DoRowEndRadarActualCalc(state, out);
			state.curr_row= curr_note.Row();
			state.num_notes_on_curr_row= 0;
			state.num_holds_on_curr_row= 0;
			state.judgable= timing->IsJudgableAtRow(state.curr_row);
			for(size_t n= 0; n < state.hold_ends.size(); ++n)
			{
				if(state.hold_ends[n].end_row < state.curr_row)
				{
					state.hold_ends.erase(state.hold_ends.begin() + n);
					--n;
				}
			}
			state.last_tns_on_row= TapNoteScore_Invalid;
			state.last_time_on_row= -9999;
			state.worst_tns_on_row= TapNoteScore_Invalid;
		}
		bool for_this_player= curr_note->pn == pn || pn == PLAYER_INVALID ||
			curr_note->pn == PLAYER_INVALID;
		if(state.judgable && for_this_player)
		{
			switch(curr_note->type)
			{
				case TapNoteType_HoldTail:
					// If there are tick holds, then the hold tail needs to be counted
					// in last_hittable_row because that's where the combo will end.
					// -Kyz
					if(tick_holds)
					{
						UpdateHittable(state.curr_row, first_hittable_row, last_hittable_row);
					}
					break;
				case TapNoteType_Tap:
				case TapNoteType_HoldHead:
					// HoldTails and Attacks are counted by IsTap.  But it doesn't
					// make sense to count HoldTails as hittable notes. -Kyz
				case TapNoteType_Attack:
				case TapNoteType_Lift:
					UpdateHittable(state.curr_row, first_hittable_row, last_hittable_row);
					++state.num_notes_on_curr_row;
					state.notes_hit_for_stream+= (curr_note->result.tns >= state.stream_tns);
					state.notes_hit+= (curr_note->result.tns >= state.taps_tns);
					if(curr_note->result.tns < state.worst_tns_on_row)
					{
						state.worst_tns_on_row= curr_note->result.tns;
					}
					if(curr_note->result.fTapNoteOffset > state.last_time_on_row)
					{
						state.last_time_on_row= curr_note->result.fTapNoteOffset;
						state.last_tns_on_row= curr_note->result.tns;
					}
					if(curr_note->type == TapNoteType_HoldHead)
					{
						if(curr_note->subType == TapNoteSubType_Hold)
						{
							state.holds_held+= (curr_note->HoldResult.hns == HNS_Held);
						}
						else if(curr_note->subType == TapNoteSubType_Roll)
						{
							state.rolls_held+= (curr_note->HoldResult.hns == HNS_Held);
						}
						state.hold_ends.push_back(
							hold_status(state.curr_row + curr_note->iDuration,
								curr_note->HoldResult.iLastHeldRow));
						++state.num_holds_on_curr_row;
					}
					else if(curr_note->type == TapNoteType_Lift)
					{
						state.lifts_hit+= (curr_note->result.tns >= state.lifts_tns);
					}
					break;
				case TapNoteType_Mine:
					state.mines_avoided+= (curr_note->result.tns == TNS_AvoidMine);
					break;
				case TapNoteType_Fake:
				default:
					break;
			}
		}
		++curr_note;
	}
	DoRowEndRadarActualCalc(state, out);

	// ScreenGameplay passes in the RadarValues that were calculated by
	// NoteDataUtil::CalculateRadarValues, so those are reused here. -Kyz
	int note_count= out[RadarCategory_Notes];
	int jump_count= out[RadarCategory_Jumps];
	int hold_count= out[RadarCategory_Holds];
	int tap_count= out[RadarCategory_TapsAndHolds];
	float hittable_steps_length= max(0, 
		timing->GetElapsedTimeFromBeat(NoteRowToBeat(last_hittable_row)) -
		timing->GetElapsedTimeFromBeat(NoteRowToBeat(first_hittable_row)));
	// The for loop and the assert are used to ensure that all fields of 
	// RadarValue get set in here.
	FOREACH_ENUM(RadarCategory, rc)
	{
		switch(rc)
		{
			case RadarCategory_Stream:
				out[rc]= clamp(float(state.notes_hit_for_stream) / note_count, 0.0f, 1.0f);
				break;
			case RadarCategory_Voltage:
				out[rc]= GetActualVoltageRadarValue(in, hittable_steps_length, pss);
				break;
			case RadarCategory_Air:
				out[rc]= clamp(float(state.jumps_hit_for_air) / jump_count, 0.0f, 1.0f);
				break;
			case RadarCategory_Freeze:
				out[rc]= clamp(float(state.holds_held) / hold_count, 0.0f, 1.0f);
				break;
			case RadarCategory_Chaos:
				out[rc]= GetActualChaosRadarValue(in, song_seconds, pss);
				break;
			case RadarCategory_TapsAndHolds:
				out[rc]= state.taps_hit;
				break;
			case RadarCategory_Jumps:
				out[rc]= state.jumps_hit;
				break;
			case RadarCategory_Holds:
				out[rc]= state.holds_held;
				break;
			case RadarCategory_Mines:
				out[rc]= state.mines_avoided;
				break;
			case RadarCategory_Hands:
				out[rc]= state.hands_hit;
				break;
			case RadarCategory_Rolls:
				out[rc]= state.rolls_held;
				break;
			case RadarCategory_Lifts:
				out[rc]= state.lifts_hit;
				break;
			case RadarCategory_Fakes:
				out[rc]= out[rc];
				break;
			case RadarCategory_Notes:
				out[rc]= state.notes_hit;
				break;
			DEFAULT_FAIL(rc);
		}
	}
}

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
