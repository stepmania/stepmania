#include "global.h"
#include "TimingAssist.h"
#include "AnnouncerManager.h"
#include "GameState.h"
#include "NoteData.h"
#include "NoteTypes.h"
#include "PrefsManager.h"

#include "RageLog.h"
TimingAssist::TimingAssist()
{
	for(int i = 0; i < 2; ++i)
	{
		const CString direction = i? "early":"late";
		for(TapNoteScore j = TNS_BOO; j < TNS_MARVELOUS; j = TapNoteScore(j+1))
		{
			CString name;
			switch(j)
			{
			case TNS_MISS: name = "miss"; break;
			case TNS_BOO: name = "boo"; break;
			case TNS_GOOD: name = "good"; break;
			case TNS_GREAT: name = "great"; break;
			case TNS_PERFECT: name = "perfect"; break;
			}
			CString path = ssprintf("gameplay assist %s %s", direction.GetString(), name.GetString());
			Timing[j][i].Load(ANNOUNCER->GetPathTo(path));
		}
	}

	Miss.Load(ANNOUNCER->GetPathTo("gameplay assist miss"));
	memset(data, 0, sizeof(data));
}

void TimingAssist::Load(PlayerNumber pn, NoteDataWithScoring *nd)
{
	data[pn] = nd;
}

void TimingAssist::Reset()
{
	LastRow = 0;
}

void TimingAssist::Update(float fDeltaTime)
{
	Actor::Update(fDeltaTime);

	for( int pn=0; pn<NUM_PLAYERS; pn++ )
	{
		if( !GAMESTATE->IsPlayerEnabled(PlayerNumber(pn)) )
			continue;

		DoTimingAssist((PlayerNumber)pn);
	}
}

void TimingAssist::DoTimingAssist(PlayerNumber pn)
{
	ASSERT(data[pn]);

	if(!GAMESTATE->m_PlayerOptions[pn].m_bTimingAssist)
		return;

	const int maxrow = BeatToNoteRow(GAMESTATE->m_fSongBeat);
	for( ; LastRow <= maxrow; LastRow++)
	{
		TapNoteScore mintns = data[pn]->MinTapNoteScore(LastRow);

		/* If the minimum score is TNS_NONE, then not all notes on this row have
		 * been graded, so stop. */
		if(mintns == TNS_NONE) return;

		/* Don't announce if the score is the maximum. */
		if(mintns == TNS_MARVELOUS) continue;
		if(mintns == TNS_PERFECT && !PREFSMAN->m_bMarvelousTiming) continue;

		if(mintns == TNS_MISS)
		{
			Miss.PlayRandom();
			continue;
		}

		const int last_tns_track = data[pn]->LastTapNoteScoreTrack(LastRow);

		/* We shouldn't get here if there're no notes on this row, since MinTNS
		 * above should have returned TNS_NONE. */
		ASSERT(last_tns_track != -1);

		const TapNoteScore last_tns = data[pn]->GetTapNoteScore(last_tns_track, LastRow);
		const float offset = data[pn]->GetTapNoteOffset(last_tns_track, LastRow);
		const bool early = offset < 0;

		Timing[last_tns][early].PlayRandom();
	}
}
/*
-----------------------------------------------------------------------------
 Copyright (c) 2003 by the person(s) listed below.  All rights reserved.
	Glenn Maynard
-----------------------------------------------------------------------------
*/
