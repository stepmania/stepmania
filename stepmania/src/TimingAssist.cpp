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
			CString path = ssprintf("gameplay assist %s %s", direction.c_str(), name.c_str());
			Timing[j][i].Load(ANNOUNCER->GetPathTo(path));
		}
	}

	Miss.Load(ANNOUNCER->GetPathTo("gameplay assist miss"));
	Exact.Load(ANNOUNCER->GetPathTo("gameplay assist exact"));
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

void TimingAssist::Announce(TapNoteScore tns, bool early)
{
	/* Only one miss and perfect/marv sound.  We could have early/late
	 * sounds for perfect and marvelous, but let's just have one "exact"
	 * sound.  We should have something else for advanced players who
	 * actually do want to know if they're late or early for a marvelous. */
	if(tns == TNS_MISS)
		Miss.PlayRandom();
	else if(tns == TNS_PERFECT || tns == TNS_MARVELOUS)
		Exact.PlayRandom();
	else
		Timing[tns][early].PlayRandom();
}

void TimingAssist::DoTimingAssist(PlayerNumber pn)
{
	ASSERT(data[pn]);

	if(!GAMESTATE->m_PlayerOptions[pn].m_bTimingAssist)
		return;

	const int maxrow = BeatToNoteRow(GAMESTATE->m_fSongBeat);
	for( ; LastRow <= maxrow; LastRow++)
	{
		const int last_tns_track = data[pn]->LastTapNoteScoreTrack(LastRow);

		/* -1 means the row has no tap notes; it's irrelevant. */
		if(last_tns_track == -1) continue;

		const TapNoteScore last_tns = data[pn]->GetTapNoteScore(last_tns_track, LastRow);
		/* TNS_NONE means the row is incomplete--it hasn't been graded yet. */
		if(last_tns == TNS_NONE) return;

		const float offset = data[pn]->GetTapNoteOffset(last_tns_track, LastRow);
		const bool early = offset < 0;

		Announce(last_tns, early);
	}
}
/*
-----------------------------------------------------------------------------
 Copyright (c) 2003 by the person(s) listed below.  All rights reserved.
	Glenn Maynard
-----------------------------------------------------------------------------
*/
