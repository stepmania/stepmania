#include "global.h"
#include "Attack.h"
#include "GameState.h"
#include "RageUtil.h"
#include "Song.h"
#include "PlayerOptions.h"
#include "PlayerState.h"

#include <cmath>
#include <vector>


void Attack::GetAttackBeats( const Song *pSong, float &fStartBeat, float &fEndBeat ) const
{
	ASSERT( pSong != nullptr );
	ASSERT_M( fStartSecond >= 0, ssprintf("StartSecond: %f",fStartSecond) );

	const TimingData &timing = pSong->m_SongTiming;
	fStartBeat = timing.GetBeatFromElapsedTime( fStartSecond );
	fEndBeat = timing.GetBeatFromElapsedTime( fStartSecond+fSecsRemaining );
}

/* Get the range for an attack that's being applied in realtime, eg. during battle
 * mode.  We need a PlayerState for this, so we can push the region off-screen to
 * prevent popping when the attack has note modifers. */
void Attack::GetRealtimeAttackBeats( const Song *pSong, const PlayerState* pPlayerState, float &fStartBeat, float &fEndBeat ) const
{
	ASSERT( pSong != nullptr );

	if( fStartSecond >= 0 )
	{
		GetAttackBeats( pSong, fStartBeat, fEndBeat );
		return;
	}

	ASSERT( pPlayerState != nullptr );

	/* If reasonable, push the attack forward 8 beats so that notes on screen don't change suddenly. */
	fStartBeat = std::min( GAMESTATE->m_Position.m_fSongBeat+8, pPlayerState->m_fLastDrawnBeat );
	fStartBeat = std::trunc(fStartBeat) + 1;

	const TimingData &timing = pSong->m_SongTiming;
	const float lStartSecond = timing.GetElapsedTimeFromBeat( fStartBeat );
	const float fEndSecond = lStartSecond + fSecsRemaining;
	fEndBeat = timing.GetBeatFromElapsedTime( fEndSecond );
	fEndBeat = std::trunc(fEndBeat) + 1;

	// loading the course should have caught this.
	ASSERT_M( fEndBeat >= fStartBeat, ssprintf("EndBeat %f >= StartBeat %f", fEndBeat, fStartBeat) );
}

bool Attack::operator== ( const Attack &rhs ) const
{
#define EQUAL(a) (a==rhs.a)
	return
		EQUAL(level) &&
		EQUAL(fStartSecond) &&
		EQUAL(fSecsRemaining) &&
		EQUAL(sModifiers) &&
		EQUAL(bOn) &&
		EQUAL(bGlobal);
}

bool Attack::ContainsTransformOrTurn() const
{
	PlayerOptions po;
	po.FromString( sModifiers );
	return po.ContainsTransformOrTurn();
}

Attack Attack::FromGlobalCourseModifier( const RString &sModifiers )
{
	Attack a;
	a.fStartSecond = 0;
	a.fSecsRemaining = 10000; /* whole song */
	a.level = ATTACK_LEVEL_1;
	a.sModifiers = sModifiers;
	a.bGlobal = true;
	return a;
}

RString Attack::GetTextDescription() const
{
	RString s = sModifiers + " " + ssprintf("(%.2f seconds)", fSecsRemaining);
	return s;
}

int Attack::GetNumAttacks() const
{
	std::vector<RString> tmp;
	split(this->sModifiers, ",", tmp);
	return tmp.size();
}

bool AttackArray::ContainsTransformOrTurn() const
{
	return std::any_of((*this).begin(), (*this).end(), [](Attack const &a) { return a.ContainsTransformOrTurn(); });
}

std::vector<RString> AttackArray::ToVectorString() const
{
	std::vector<RString> ret;
	for (Attack const &a : *this)
	{
		ret.push_back(ssprintf("TIME=%f:LEN=%f:MODS=%s",
				       a.fStartSecond,
				       a.fSecsRemaining,
				       a.sModifiers.c_str()));
	}
	return ret;
}

void AttackArray::UpdateStartTimes(float delta)
{
	for (Attack &a : *this)
	{
		a.fStartSecond += delta;
	}
}

/*
 * (c) 2003-2004 Chris Danford
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
