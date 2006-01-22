#include "global.h"
#include "Attack.h"
#include "GameState.h"
#include "RageUtil.h"
#include "song.h"
#include "Foreach.h"
#include "PlayerOptions.h"

void Attack::GetAttackBeats( const Song *song, const PlayerState* pPlayerState, float &fStartBeat, float &fEndBeat ) const
{
	ASSERT( song );

	if( fStartSecond >= 0 )
	{
		CHECKPOINT;
		fStartBeat = song->GetBeatFromElapsedTime( fStartSecond );
		fEndBeat = song->GetBeatFromElapsedTime( fStartSecond+fSecsRemaining );
	}
	else
	{
		CHECKPOINT;
		/* If fStartSecond < 0, then the attack starts right off the screen; this requires
		 * that a song actually be playing.  Pre-queued course attacks must always have 
		 * fStartSecond >= 0. */
		ASSERT( GAMESTATE->m_pCurSong );
		
		ASSERT( pPlayerState );

		/* We're setting this effect on the fly.  If it's an arrow-changing effect
		 * (transform or note skin), apply it in the future, after what's currently on
		 * screen, so new arrows will scroll on screen with this effect. */
		GAMESTATE->GetUndisplayedBeats( pPlayerState, fSecsRemaining, fStartBeat, fEndBeat );
	}

	// loading the course should have caught this.
	ASSERT_M( fEndBeat >= fStartBeat, ssprintf("%f >= %f", fEndBeat, fStartBeat) );
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

bool AttackArray::ContainsTransformOrTurn() const
{
	FOREACH_CONST( Attack, *this, a )
	{
		if( a->ContainsTransformOrTurn() )
			return true;
	}
	return false;
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
