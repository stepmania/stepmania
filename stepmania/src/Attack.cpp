#include "global.h"
#include "Attack.h"
#include "GameState.h"
#include "song.h"

void Attack::GetAttackBeats( const Song *song, PlayerNumber pn, float &fStartBeat, float &fEndBeat ) const
{
	if( fStartSecond >= 0 )
	{
		fStartBeat = song->GetBeatFromElapsedTime( fStartSecond );
		fEndBeat = song->GetBeatFromElapsedTime( fStartSecond+fSecsRemaining );
	} else {
		/* We're setting this effect on the fly.  If it's an arrow-changing effect
		 * (transform or note skin), apply it in the future, past what's currently on
		 * screen, so new arrows will scroll on screen with this effect. */
		GAMESTATE->GetUndisplayedBeats( pn, fSecsRemaining, fStartBeat, fEndBeat );
	}
}
