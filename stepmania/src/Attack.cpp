#include "global.h"
#include "Attack.h"
#include "GameState.h"
#include "RageUtil.h"
#include "song.h"

void Attack::GetAttackBeats( const Song *song, PlayerNumber pn, float &fStartBeat, float &fEndBeat ) const
{
	if( fStartSecond >= 0 )
	{
		CHECKPOINT;
		fStartBeat = song->GetBeatFromElapsedTime( fStartSecond );
		fEndBeat = song->GetBeatFromElapsedTime( fStartSecond+fSecsRemaining );
	} else {
		CHECKPOINT;
		/* If fStartSecond < 0, then the attack starts right off the screen; this requires
		 * that a song actually be playing.  Pre-queued course attacks must always have 
		 * fStartSecond >= 0. */
		ASSERT( GAMESTATE->m_pCurSong );
		
		/* We're setting this effect on the fly.  If it's an arrow-changing effect
		 * (transform or note skin), apply it in the future, past what's currently on
		 * screen, so new arrows will scroll on screen with this effect. */
		GAMESTATE->GetUndisplayedBeats( pn, fSecsRemaining, fStartBeat, fEndBeat );
	}

	RAGE_ASSERT_M( fEndBeat >= fStartBeat, ssprintf("%f >= %f", fEndBeat, fStartBeat) );
}
