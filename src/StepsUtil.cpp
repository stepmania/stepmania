#include "global.h"
#include "StepsUtil.h"
#include "Steps.h"
#include "ProfileManager.h"
#include "Profile.h"
#include "Song.h"
#include "SongManager.h"
#include "NoteData.h"
#include "NoteDataUtil.h" // If we can skip the complicated radar stuff.
#include "GameManager.h"
#include "XmlFile.h"
#include "UnlockManager.h"
#include "SongUtil.h"


bool StepsCriteria::Matches( const Song *pSong, const Steps *pSteps ) const
{
	if( m_difficulty != Difficulty_Invalid  &&  pSteps->GetDifficulty() != m_difficulty )
		return false;
	if( m_iLowMeter != -1  &&  pSteps->GetMeter() < m_iLowMeter )
		return false;
	if( m_iHighMeter != -1  &&  pSteps->GetMeter() > m_iHighMeter )
		return false;
	if( m_st != StepsType_Invalid  &&  pSteps->m_StepsType != m_st )
		return false;
	switch( m_Locked )
	{
	DEFAULT_FAIL(m_Locked);
	case Locked_Locked:
		if( UNLOCKMAN  &&  !UNLOCKMAN->StepsIsLocked(pSong,pSteps) )
			return false;
		break;
	case Locked_Unlocked:
		if( UNLOCKMAN  &&  UNLOCKMAN->StepsIsLocked(pSong,pSteps) )
			return false;
		break;
	case Locked_DontCare:
		break;
	}

	return true;
}

void StepsUtil::GetAllMatching( const SongCriteria &soc, const StepsCriteria &stc, vector<SongAndSteps> &out )
{
	const RString &sGroupName = soc.m_sGroupName.empty()? GROUP_ALL:soc.m_sGroupName;
        const vector<Song*> &songs = SONGMAN->GetSongs( sGroupName );

	FOREACH_CONST( Song*, songs, so )
	{
		if( !soc.Matches(*so) )
			continue;
		GetAllMatching( *so, stc, out );
	}
}

void StepsUtil::GetAllMatching( Song *pSong, const StepsCriteria &stc, vector<SongAndSteps> &out )
{
	const vector<Steps*> &vSteps = ( stc.m_st == StepsType_Invalid ?  pSong->GetAllSteps() :
					 pSong->GetStepsByStepsType(stc.m_st) );
	
	FOREACH_CONST( Steps*, vSteps, st )
		if( stc.Matches(pSong, *st) )
			out.push_back( SongAndSteps(pSong, *st) );
}

bool StepsUtil::HasMatching( const SongCriteria &soc, const StepsCriteria &stc )
{
	const RString &sGroupName = soc.m_sGroupName.empty()? GROUP_ALL:soc.m_sGroupName;
        const vector<Song*> &songs = SONGMAN->GetSongs( sGroupName );

	FOREACH_CONST( Song*, songs, so )
	{
		if( soc.Matches(*so) && HasMatching(*so, stc) )
		        return true;
	}
	return false;
}

bool StepsUtil::HasMatching( const Song *pSong, const StepsCriteria &stc )
{
	const vector<Steps*> &vSteps = stc.m_st == StepsType_Invalid? pSong->GetAllSteps():pSong->GetStepsByStepsType( stc.m_st );
	
	FOREACH_CONST( Steps*, vSteps, st )
	{
		if( stc.Matches(pSong, *st) )
	  	        return true;
	}
	return false;
}

void CalculateTrueRadarValue(const Steps *in,
	const float songLength,
	RadarValues * all,
	const RadarCategory rc,
	vector<float> (*RadarCalc)(const Steps *, float)) // Not a member function, should be alright.
{
	vector<float> nums = (*RadarCalc)(in, songLength);
	FOREACH_ENUM(PlayerNumber, pn)
	{
		all[pn][rc] = nums[pn];
	}
}

void CalculateStatRadarValue(const Steps *in,
	RadarValues * all,
	const RadarCategory rc,
	vector<int> (Steps::*StatCalc)(int, int) const) // Need a member function here.
{
	vector<int> nums = (*in.*StatCalc)(0, MAX_NOTE_ROW); // Forced to specify these.
	all[PLAYER_1][rc] = nums[0];
	all[PLAYER_2][rc] = nums[(nums.size() < NUM_PLAYERS) ? 0 : 1];
}

void StepsUtil::CalculateRadarValues( Steps *in, float fSongSeconds )
{	
	bool isMulti = in->IsMultiPlayerStyle(); // needed for later
	if (!isMulti &&
		!in->m_Timing.HasFakes() &&
		!in->m_Timing.HasWarps())
	{
		RadarValues tmp;
		NoteDataUtil::CalculateRadarValues(in->GetNoteData(), fSongSeconds, tmp);
		RadarValues one[NUM_PLAYERS];
		FOREACH_ENUM(PlayerNumber, pn)
		{
			one[pn] = tmp;
		}
		in->SetCachedRadarValues(one);
		return;
	}

	RadarValues all[NUM_PLAYERS];

	// The for loop and the assert are used to ensure that all fields of 
	// RadarValue get set in here.
	
	/*
	 * Possible optimization opportunity here:
	 * The enums go in order from the "true" radar categories to the "stat"
	 * radar categories. The true ones require information that is later
	 * gotten from the stat categories. Perhaps instead of looping
	 * through the enum, we should hardcode values so as to reduce
	 * processing of the underlying notedata many times? --Wolfman2000
	 */
	FOREACH_ENUM( RadarCategory, rc )
	{
		switch( rc )
		{
			case RadarCategory_Stream:
			{
				CalculateTrueRadarValue(in, fSongSeconds, all, rc,
					&StepsUtil::GetStreamRadarValue);
				break;	
			}
			case RadarCategory_Voltage:
			{
				CalculateTrueRadarValue(in, fSongSeconds, all, rc,
					&StepsUtil::GetVoltageRadarValue);
				break;	
			}
			case RadarCategory_Air:
			{
				CalculateTrueRadarValue(in, fSongSeconds, all, rc,
					&StepsUtil::GetAirRadarValue);
				break;
			}
			case RadarCategory_Freeze:
			{
				CalculateTrueRadarValue(in, fSongSeconds, all, rc,
					&StepsUtil::GetFreezeRadarValue);
				break;
			}
			case RadarCategory_Chaos:
			{
				CalculateTrueRadarValue(in, fSongSeconds, all, rc,
					&StepsUtil::GetChaosRadarValue);
				break;
			}
			
			case RadarCategory_TapsAndHolds:	
			{
				CalculateStatRadarValue(in, all, rc,
					&Steps::GetNumTapNotes);
				break;
			}
			case RadarCategory_Jumps:
			{
				CalculateStatRadarValue(in, all, rc,
					&Steps::GetNumJumps);
				break;
			}
			case RadarCategory_Holds:
			{
				CalculateStatRadarValue(in, all, rc,
					&Steps::GetNumHoldNotes);
				break;
			}
			case RadarCategory_Mines:
			{
				CalculateStatRadarValue(in, all, rc,
					&Steps::GetNumMines);
				break;
			}
			case RadarCategory_Hands:
			{
				CalculateStatRadarValue(in, all, rc,
					&Steps::GetNumHands);
				break;
			}
			case RadarCategory_Rolls:
			{
				CalculateStatRadarValue(in, all, rc,
					&Steps::GetNumRolls);
				break;
			}
			case RadarCategory_Lifts:
			{
				CalculateStatRadarValue(in, all, rc,
					&Steps::GetNumLifts);
				break;
			}
			case RadarCategory_Fakes:
			{
				CalculateStatRadarValue(in, all, rc,
					&Steps::GetNumFakes);
				break;
			}
			default:	FAIL_M("Non-existant radar category attempted to be set!");
		}
	}
}

vector<float> StepsUtil::GetStreamRadarValue( const Steps *in, float fSongSeconds )
{
	vector<float> nums(NUM_PLAYERS, 0.0f);
	if( !fSongSeconds )
		return nums;
	// density of steps
	vector<float> notes(NUM_PLAYERS);
	vector<int> taps = in->GetNumTapNotes();
	vector<int> holds = in->GetNumHoldNotes();

	if (!in->IsMultiPlayerStyle())
	{
		taps.push_back(taps[0]);
		holds.push_back(holds[0]);
	}
	for (unsigned i = 0; i < notes.size(); ++i)
	{
		notes[i] = min((((taps[i] + holds[i]) / fSongSeconds) / 7), 1.0f);
	}
	return notes;
}

vector<float> StepsUtil::GetVoltageRadarValue( const Steps *in, float fSongSeconds )
{
	vector<float> nums(NUM_PLAYERS, 0.0f);
	if( !fSongSeconds )
		return nums;

	const NoteData &nd = in->GetNoteData();
	const float fLastBeat = nd.GetLastBeat();
	const float fAvgBPS = fLastBeat / fSongSeconds;

	// peak density of steps
	vector<float> maxDensity(NUM_PLAYERS, 0.0f);

	const float BEAT_WINDOW = 8;
	const int BEAT_WINDOW_ROWS = BeatToNoteRow( BEAT_WINDOW );

	// TODO: Find a way to work around warps and fakes better.
	for( int i=0; i<=BeatToNoteRow(fLastBeat); i+=BEAT_WINDOW_ROWS )
	{
		vector<int> taps = in->GetNumTapNotes(i, i + BEAT_WINDOW_ROWS);
		vector<int> holds = in->GetNumHoldNotes(i, i + BEAT_WINDOW_ROWS);

		if (!in->IsMultiPlayerStyle())
		{
			taps.push_back(taps[0]);
			holds.push_back(holds[0]);
		}

		for (unsigned pn = 0; pn < nums.size(); ++pn)
		{
			nums[pn] = max(nums[pn], (taps[pn] + holds[pn]) / BEAT_WINDOW);
		}
	}

	for (unsigned pn = 0; pn < nums.size(); ++pn)
	{
		nums[pn] = min((nums[pn] * fAvgBPS/10), 1.0f);
	}
	return nums;
}

vector<float> StepsUtil::GetAirRadarValue( const Steps *in, float fSongSeconds )
{
	vector<float> nums(NUM_PLAYERS, 0.0f);
	if( !fSongSeconds )
		return nums;
	// number of doubles
	vector<float> jumps(NUM_PLAYERS);

	vector<int> hops = in->GetNumJumps();
	if (!in->IsMultiPlayerStyle())
	{
		hops.push_back(hops[0]);
	}
	for (unsigned i = 0; i < jumps.size(); ++i)
	{
		jumps[i] = min(hops[i] / fSongSeconds, 1.0f);
	}
	return jumps;
}

vector<float> StepsUtil::GetFreezeRadarValue( const Steps *in, float fSongSeconds )
{
	vector<float> nums(NUM_PLAYERS, 0.0f);
	if( !fSongSeconds )
		return nums;
	// number of hold steps
	vector<float> holds(NUM_PLAYERS);

	vector<int> stats = in->GetNumHoldNotes();
	if (!in->IsMultiPlayerStyle())
	{
		stats.push_back(stats[0]);
	}
	for (unsigned i = 0; i < holds.size(); ++i)
	{
		holds[i] = min(stats[i] / fSongSeconds, 1.0f);
	}
	return holds;
}

vector<float> StepsUtil::GetChaosRadarValue( const Steps *in, float fSongSeconds )
{
	vector<float> nums(NUM_PLAYERS, 0.0f);
	if( !fSongSeconds )
		return nums;
	// count number of notes smaller than 8ths
	
	const NoteData &nd = in->GetNoteData();
	int perPlayerTracks = nd.GetNumTracks() / 2; // mainly for couple.
	FOREACH_NONEMPTY_ROW_ALL_TRACKS( nd, r )
	{
		if (!in->m_Timing.IsJudgableAtRow(r))
		{
			continue; // if players can't interact with the row, don't count it.
		}

		if( GetNoteType(r) >= NOTE_TYPE_12TH )
		{
			FOREACH_ENUM(PlayerNumber, pn)
			{
				if (!in->IsMultiPlayerStyle())
				{
					++nums[pn];
				}
				else if (in->GetStepsTypeCategory() == StepsTypeCategory_Couple)
				{
					// Remember: couple splits up the notes in the middle.
					for (int t = static_cast<int>(pn) * perPlayerTracks;
						t < (static_cast<int>(pn) + 1) * perPlayerTracks; ++t)
					{
						const TapNote &tn = nd.GetTapNote(t, r);
						if (tn.type != TapNote::empty && tn.type != TapNote::fake)
						{
							++nums[pn];
						}
					}
				}
				else // routine
				{
					for (int t = 0; t < nd.GetNumTracks(); ++t)
					{
						const TapNote &tn = nd.GetTapNote(t, r);
						if (tn.type != TapNote::empty &&
							tn.type != TapNote::fake &&
							tn.pn == pn)
						{
							++nums[pn];
							break;
						}
					}
				}
			}
		}
	}
	for (unsigned i = 0; i < nums.size(); ++i)
	{
		nums[i] = min(nums[i] / fSongSeconds * 0.5f, 1.0f);
	}
	return nums;
}

// Sorting stuff
map<const Steps*, RString> steps_sort_val;

static bool CompareStepsPointersBySortValueAscending(const Steps *pSteps1, const Steps *pSteps2)
{
	return steps_sort_val[pSteps1] < steps_sort_val[pSteps2];
}

static bool CompareStepsPointersBySortValueDescending(const Steps *pSteps1, const Steps *pSteps2)
{
	return steps_sort_val[pSteps1] > steps_sort_val[pSteps2];
}

void StepsUtil::SortStepsPointerArrayByNumPlays( vector<Steps*> &vStepsPointers, ProfileSlot slot, bool bDescending )
{
	if( !PROFILEMAN->IsPersistentProfile(slot) )
		return;	// nothing to do since we don't have data
	Profile* pProfile = PROFILEMAN->GetProfile(slot);
	SortStepsPointerArrayByNumPlays( vStepsPointers, pProfile, bDescending );
}

void StepsUtil::SortStepsPointerArrayByNumPlays( vector<Steps*> &vStepsPointers, const Profile* pProfile, bool bDecending )
{
	// ugly...
	vector<Song*> vpSongs = SONGMAN->GetAllSongs();
	vector<Steps*> vpAllSteps;
	map<Steps*,Song*> mapStepsToSong;
	{
		for( unsigned i=0; i<vpSongs.size(); i++ )
		{
			Song* pSong = vpSongs[i];
			vector<Steps*> vpSteps = pSong->GetAllSteps();
			for( unsigned j=0; j<vpSteps.size(); j++ )
			{
				Steps* pSteps = vpSteps[j];
				if( pSteps->IsAutogen() )
					continue;	// skip
				vpAllSteps.push_back( pSteps );
				mapStepsToSong[pSteps] = pSong;
			}
		}
	}

	ASSERT( pProfile );
	for(unsigned i = 0; i < vStepsPointers.size(); ++i)
	{
		Steps* pSteps = vStepsPointers[i];
		Song* pSong = mapStepsToSong[pSteps];
		steps_sort_val[vStepsPointers[i]] = ssprintf("%9i", pProfile->GetStepsNumTimesPlayed(pSong,pSteps));
	}
	stable_sort( vStepsPointers.begin(), vStepsPointers.end(), bDecending ? CompareStepsPointersBySortValueDescending : CompareStepsPointersBySortValueAscending );
	steps_sort_val.clear();
}

bool StepsUtil::CompareNotesPointersByRadarValues(const Steps* pSteps1, const Steps* pSteps2)
{
	float fScore1 = 0;
	float fScore2 = 0;

	fScore1 += pSteps1->GetRadarValues( PLAYER_1 )[RadarCategory_TapsAndHolds];
	fScore2 += pSteps2->GetRadarValues( PLAYER_1 )[RadarCategory_TapsAndHolds];

	return fScore1 < fScore2;
}

bool StepsUtil::CompareNotesPointersByMeter(const Steps *pSteps1, const Steps* pSteps2)
{
	return pSteps1->GetMeter() < pSteps2->GetMeter();
}

bool StepsUtil::CompareNotesPointersByDifficulty(const Steps *pSteps1, const Steps *pSteps2)
{
	return pSteps1->GetDifficulty() < pSteps2->GetDifficulty();
}

void StepsUtil::SortNotesArrayByDifficulty( vector<Steps*> &arraySteps )
{
	/* Sort in reverse order of priority. Sort by description first, to get
	 * a predictable order for songs with no radar values (edits). */
	stable_sort( arraySteps.begin(), arraySteps.end(), CompareStepsPointersByDescription );
	stable_sort( arraySteps.begin(), arraySteps.end(), CompareNotesPointersByRadarValues );
	stable_sort( arraySteps.begin(), arraySteps.end(), CompareNotesPointersByMeter );
	stable_sort( arraySteps.begin(), arraySteps.end(), CompareNotesPointersByDifficulty );
}

bool StepsUtil::CompareStepsPointersByTypeAndDifficulty(const Steps *pStep1, const Steps *pStep2)
{
	if( pStep1->m_StepsType < pStep2->m_StepsType )
		return true;
	if( pStep1->m_StepsType > pStep2->m_StepsType )
		return false;
	return pStep1->GetDifficulty() < pStep2->GetDifficulty();
}

void StepsUtil::SortStepsByTypeAndDifficulty( vector<Steps*> &arraySongPointers )
{
	sort( arraySongPointers.begin(), arraySongPointers.end(), CompareStepsPointersByTypeAndDifficulty );
}

bool StepsUtil::CompareStepsPointersByDescription(const Steps *pStep1, const Steps *pStep2)
{
	return pStep1->GetDescription().CompareNoCase( pStep2->GetDescription() ) < 0;
}

void StepsUtil::SortStepsByDescription( vector<Steps*> &arraySongPointers )
{
	sort( arraySongPointers.begin(), arraySongPointers.end(), CompareStepsPointersByDescription );
}

void StepsUtil::RemoveLockedSteps( const Song *pSong, vector<Steps*> &vpSteps )
{
	for( int i=vpSteps.size()-1; i>=0; i-- )
	{
		if( UNLOCKMAN->StepsIsLocked(pSong, vpSteps[i]) )
			vpSteps.erase( vpSteps.begin()+i );
	}
}


////////////////////////////////
// StepsID
////////////////////////////////

void StepsID::FromSteps( const Steps *p )
{
	if( p == NULL )
	{
		st = StepsType_Invalid;
		dc = Difficulty_Invalid;
		sDescription = "";
		uHash = 0;
	}
	else
	{
		st = p->m_StepsType;
		dc = p->GetDifficulty();
		if( dc == Difficulty_Edit )
		{
			sDescription = p->GetDescription();
			uHash = p->GetHash();
		}
		else
		{
			sDescription = "";
			uHash = 0;
		}
	}

	m_Cache.Unset();
}

/* XXX: Don't allow duplicate edit descriptions, and don't allow edit descriptions
 * to be difficulty names (eg. "Hard").  If we do that, this will be completely unambiguous.
 *
 * XXX: Unless two memcards are inserted and there's overlap in the names.  In that
 * case, maybe both edits should be renamed to "Pn: foo"; as long as we don't write
 * them back out (which we don't do except in the editor), it won't be permanent. 
 * We could do this during the actual Steps::GetID() call, instead, but then it'd have
 * to have access to Song::m_LoadedFromProfile. */

Steps *StepsID::ToSteps( const Song *p, bool bAllowNull ) const
{
	if( st == StepsType_Invalid || dc == Difficulty_Invalid )
		return NULL;

	SongID songID;
	songID.FromSong( p );

	Steps *pRet = NULL;
	if( dc == Difficulty_Edit )
	{
		pRet = SongUtil::GetOneSteps( p, st, dc, -1, -1, sDescription, "", uHash, true );
	}
	else
	{
		pRet = SongUtil::GetOneSteps( p, st, dc, -1, -1, "", "", 0, true );
	}
	
	if( !bAllowNull && pRet == NULL )
		FAIL_M( ssprintf("%i, %i, \"%s\"", st, dc, sDescription.c_str()) );

	m_Cache.Set( pRet );
	
	return pRet;
}

XNode* StepsID::CreateNode() const
{
	XNode* pNode = new XNode( "Steps" );

	pNode->AppendAttr( "StepsType", GAMEMAN->GetStepsTypeInfo(st).szName );
	pNode->AppendAttr( "Difficulty", DifficultyToString(dc) );
	if( dc == Difficulty_Edit )
	{
		pNode->AppendAttr( "Description", sDescription );
		pNode->AppendAttr( "Hash", uHash );
	}

	return pNode;
}

void StepsID::LoadFromNode( const XNode* pNode ) 
{
	ASSERT( pNode->GetName() == "Steps" );

	RString sTemp;

	pNode->GetAttrValue( "StepsType", sTemp );
	st = GAMEMAN->StringToStepsType( sTemp );

	pNode->GetAttrValue( "Difficulty", sTemp );
	dc = StringToDifficulty( sTemp );

	if( dc == Difficulty_Edit )
	{
		pNode->GetAttrValue( "Description", sDescription );
		pNode->GetAttrValue( "Hash", uHash );
	}
	else
	{
		sDescription = "";
		uHash = 0;
	}

	m_Cache.Unset();
}

RString StepsID::ToString() const
{
	RString s = GAMEMAN->GetStepsTypeInfo( st ).szName;
	s += " " + DifficultyToString( dc );
	if( dc == Difficulty_Edit )
	{
		s += " " + sDescription;
		s += ssprintf(" %u", uHash );
	}
	return s;
}

bool StepsID::IsValid() const
{
	return st != StepsType_Invalid && dc != Difficulty_Invalid;
}

bool StepsID::operator<( const StepsID &rhs ) const
{
#define COMP(a) if(a<rhs.a) return true; if(a>rhs.a) return false;
	COMP(st);
	COMP(dc);
	COMP(sDescription);
	COMP(uHash);
#undef COMP
	return false;
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
