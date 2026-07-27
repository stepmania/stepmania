#include "global.h"
#include "ScreenGameplayShared.h"
#include "GameState.h"
#include "Player.h"
#include "NoteDataUtil.h"
#include "NoteDataWithScoring.h"
#include "Song.h"
#include "StatsManager.h"

#include <vector>


REGISTER_SCREEN_CLASS( ScreenGameplayShared );

void ScreenGameplayShared::FillPlayerInfo( std::vector<PlayerInfo> &vPlayerInfoOut )
{
	const PlayerNumber master = GAMESTATE->GetMasterPlayerNumber();
	const PlayerNumber other = (master == PLAYER_1? PLAYER_2:PLAYER_1);

	/* The master player is where all of the real work takes place.  The other player exists
	 * only so we have a place to split stats out into at the end. */
	vPlayerInfoOut.resize( 2 );
	vPlayerInfoOut[master].Load( master, MultiPlayer_Invalid, true, Difficulty_Invalid );
	vPlayerInfoOut[other].Load( other, MultiPlayer_Invalid, false, Difficulty_Invalid );
}

PlayerInfo &ScreenGameplayShared::GetPlayerInfoForInput( const InputEventPlus& iep )
{
	return m_vPlayerInfo[GAMESTATE->GetMasterPlayerNumber()];
}

/*
 * (c) 2006 Steve Checkoway
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

