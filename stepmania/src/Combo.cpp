#include "global.h"
#include "Combo.h"
#include "ThemeManager.h"
#include "StatsManager.h"
#include "GameState.h"
#include "song.h"
#include "PlayerState.h"

Combo::Combo()
{
	m_iLastSeenCombo = -1;
	m_pPlayerState = NULL;
	m_pPlayerStageStats = NULL;
}

void Combo::Load( const RString &sPath, const PlayerState *pPlayerState, const PlayerStageStats *pPlayerStageStats )
{
	ASSERT( m_SubActors.empty() );	// don't load twice

	m_pPlayerState = pPlayerState;
	m_pPlayerStageStats = pPlayerStageStats;

	m_sprComboLabel.Load( sPath );
	this->AddChild( m_sprComboLabel );
}

void Combo::SetCombo( int iCombo, int iMisses )
{
	if( m_iLastSeenCombo == -1 )	// first update, don't set bIsMilestone=true
		m_iLastSeenCombo = iCombo;

	bool b100Milestone = false;
	bool b1000Milestone = false;
	for( int i=m_iLastSeenCombo+1; i<=iCombo; i++ )
	{
		if( i < 600 )
			b100Milestone |= ((i % 100) == 0);
		else
			b1000Milestone |= ((i % 200) == 0);
	}
	m_iLastSeenCombo = iCombo;

	if( b100Milestone )
		this->PlayCommand( "100Milestone" );
	if( b1000Milestone )
		this->PlayCommand( "1000Milestone" );

	// don't show a colored combo until 1/4 of the way through the song
	bool bPastMidpoint = GAMESTATE->GetCourseSongIndex()>0 ||
		GAMESTATE->m_fMusicSeconds > GAMESTATE->m_pCurSong->m_fMusicLengthSeconds/4;

	Message msg("Combo");
	if( iCombo )
		msg.SetParam( "Combo", iCombo );
	if( iMisses )
		msg.SetParam( "Misses", iMisses );
	if( bPastMidpoint && m_pPlayerStageStats->FullComboOfScore(TNS_W1) )
		msg.SetParam( "FullComboW1", true );
	if( bPastMidpoint && m_pPlayerStageStats->FullComboOfScore(TNS_W2) )
		msg.SetParam( "FullComboW2", true );
	if( bPastMidpoint && m_pPlayerStageStats->FullComboOfScore(TNS_W3) )
		msg.SetParam( "FullComboW3", true );
	this->HandleMessage( msg );
}

/*
 * (c) 2001-2004 Chris Danford
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
