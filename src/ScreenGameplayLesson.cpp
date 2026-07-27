#include "global.h"
#include "ScreenGameplayLesson.h"
#include "RageLog.h"
#include "GameState.h"
#include "GamePreferences.h"
#include "StatsManager.h"
#include "Song.h"

#include <vector>


REGISTER_SCREEN_CLASS( ScreenGameplayLesson );
ScreenGameplayLesson::ScreenGameplayLesson()
{
	m_iCurrentPageIndex = 0;
	m_Try = Try_1;
}

void ScreenGameplayLesson::Init()
{
	ASSERT( GAMESTATE->GetCurrentStyle(GAMESTATE->GetMasterPlayerNumber()) != nullptr );
	ASSERT( GAMESTATE->m_pCurSong != nullptr );

	/* Now that we've set up, init the base class. */
	ScreenGameplayNormal::Init();

	ClearMessageQueue();	// remove all of the messages set in ScreenGameplay that animate "ready", "here we go", etc.

	GAMESTATE->m_bGameplayLeadIn.Set( false );

	m_DancingState = STATE_DANCING;

	// Load pages
	Song *pSong = GAMESTATE->m_pCurSong;
	RString sDir = pSong->GetSongDir();
	std::vector<RString> vs;
	GetDirListing( sDir+"Page*", vs, true, true );
	m_vPages.resize( vs.size() );
	int i = 0;
	for (RString const &s : vs)
	{
		AutoActor &aa = m_vPages[i];

		LuaThreadVariable iIndex( "PageIndex", LuaReference::Create(i++) );
		LuaThreadVariable iPages( "NumPages", LuaReference::Create( (int)vs.size() ) );
		aa.Load( s );

		aa->SetDrawOrder( DRAW_ORDER_OVERLAY+1 );
		this->AddChild( aa );
	}

	i = 0;
	for (AutoActor &aa : m_vPages)
	{
		bool bIsFirst = (i++ == 0);
		aa->PlayCommand( bIsFirst ? "Show" : "Hide" );
		aa->PlayCommand( "On" );
	}

	// Reset stage number (not relevant in lessons)
	GAMESTATE->m_iCurrentStageIndex = 0;
	FOREACH_ENUM( PlayerNumber, p )
		GAMESTATE->m_iPlayerStageTokens[p] = 1;

	// Autoplay during demonstration
	FOREACH_EnabledPlayerInfo( m_vPlayerInfo, pi )
		pi->GetPlayerState()->m_PlayerController = PC_AUTOPLAY;
}

bool ScreenGameplayLesson::Input( const InputEventPlus &input )
{
	//LOG->Trace( "ScreenGameplayLesson::Input()" );

	if( m_iCurrentPageIndex != -1 )
	{
		// show a lesson page
		return Screen::Input( input );
	}
	else
	{
		// in the "your turn" section"
		return ScreenGameplay::Input( input );
	}
}

void ScreenGameplayLesson::HandleScreenMessage( const ScreenMessage SM )
{
	if( SM == SM_NotesEnded )
	{
		bool bShowingAPage = m_iCurrentPageIndex != -1;

		// While showing a page, loop the music.
		if( bShowingAPage )
		{
			ResetAndRestartCurrentSong();
		}
		else
		{
			PlayerStageStats &pss = STATSMAN->m_CurStageStats.m_player[PLAYER_1];
			int iActual = pss.GetLessonScoreActual();
			int iNeeded = pss.GetLessonScoreNeeded();
			bool bCleared = iActual >= iNeeded;
			bool bAnyTriesLeft = m_Try + 1 < NUM_Try;

			if( bCleared )
			{
				MESSAGEMAN->Broadcast( Message_LessonCleared );
				this->HandleScreenMessage( SM_LeaveGameplay );

				// Commit scores here since we don't go through an eval screen.
				// Only commit if we've cleared.  Don't commit if we've failed all 3 tries.
				STATSMAN->m_CurStageStats.FinalizeScores( false );
			}
			else if( bAnyTriesLeft )
			{
				ResetAndRestartCurrentSong();

				m_Try = (Try)(m_Try+1);
				MESSAGEMAN->Broadcast( (MessageID)(Message_LessonTry1+Enum::to_integral(m_Try)) );
			}
			else
			{
				this->HandleScreenMessage( SM_BeginFailed );
				MESSAGEMAN->Broadcast( Message_LessonFailed );
			}
		}
		return;	// handled
	}

	ScreenGameplay::HandleScreenMessage( SM );
}

bool ScreenGameplayLesson::MenuStart( const InputEventPlus &input )
{
	// XXX: Allow repeats?
	if( m_iCurrentPageIndex == -1 )
		return false;
	ChangeLessonPage( +1 );
	return true;
}

bool ScreenGameplayLesson::MenuBack( const InputEventPlus &input )
{
	// XXX: Allow repeats?
	if( m_iCurrentPageIndex == 0 )
	{
		BeginBackingOutFromGameplay();
		return true;
	}
	if( m_iCurrentPageIndex == -1 )
		return false;
	ChangeLessonPage( -1 );
	return true;
}

void ScreenGameplayLesson::ChangeLessonPage( int iDir )
{
	if( m_iCurrentPageIndex + iDir < 0 )
	{
		// don't change
		return;
	}
	else if( m_iCurrentPageIndex + iDir >= (int)m_vPages.size() )
	{
		m_vPages[m_iCurrentPageIndex]->PlayCommand( "Hide" );
		m_iCurrentPageIndex = -1;

		ResetAndRestartCurrentSong();

		MESSAGEMAN->Broadcast( (MessageID)(Message_LessonTry1+Enum::to_integral(m_Try)) );

		// Change back to the current autoplay setting (in most cases, human controlled).
		FOREACH_EnabledPlayerInfo( m_vPlayerInfo, pi )
		{
			/* Mike: I'm not sure if you can actually have the player options enabled in this mode,
			 * but I'm putting the check here anyways to play it safe. If it's unnecessary, then it
			 * should be removed. */
			if( pi->GetPlayerState()->m_PlayerOptions.GetCurrent().m_fPlayerAutoPlay != 0 )
				pi->GetPlayerState()->m_PlayerController = PC_AUTOPLAY;
			else
				pi->GetPlayerState()->m_PlayerController = GamePreferences::m_AutoPlay;
		}
	}
	else
	{
		m_vPages[m_iCurrentPageIndex]->PlayCommand( "Hide" );
		m_iCurrentPageIndex += iDir;
		m_vPages[m_iCurrentPageIndex]->PlayCommand( "Show" );
	}
}

void ScreenGameplayLesson::ResetAndRestartCurrentSong()
{
	STATSMAN->m_CurStageStats.m_player[PLAYER_1].ResetScoreForLesson();
	m_pSoundMusic->Stop();
	ReloadCurrentSong();
	StartPlayingSong( 2, 0 );
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
