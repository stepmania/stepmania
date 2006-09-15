#include "global.h"
#include "ScreenGameplayLesson.h"
#include "RageLog.h"
#include "GameState.h"
#include "PrefsManager.h"
#include "StatsManager.h"
#include "song.h"

REGISTER_SCREEN_CLASS( ScreenGameplayLesson );
ScreenGameplayLesson::ScreenGameplayLesson()
{
	m_iCurrentPageIndex = 0;
	m_Try = Try_1;
}

void ScreenGameplayLesson::Init()
{
	ASSERT( GAMESTATE->m_pCurStyle );
	ASSERT( GAMESTATE->m_pCurSong );

	/* Now that we've set up, init the base class. */
	ScreenGameplayNormal::Init();

	ClearMessageQueue();	// remove all of the messages set in ScreenGameplay that animate "ready", "here we go", etc.

	GAMESTATE->m_bGameplayLeadIn.Set( false );

	m_DancingState = STATE_DANCING;


	// Load pages
	Song *pSong = GAMESTATE->m_pCurSong;
	RString sDir = pSong->GetSongDir();
	vector<RString> vs;
	GetDirListing( sDir+"Page*", vs, true, true );
	m_vPages.resize( vs.size() );
	FOREACH( RString, vs, s )
	{
		int i = s - vs.begin();
		AutoActor &aa = m_vPages[i];


		LUA->SetGlobal( "PageIndex", i );
		LUA->SetGlobal( "NumPages", (int)vs.size() );
		aa.Load( *s );
		LUA->UnsetGlobal( "PageIndex" );
		LUA->UnsetGlobal( "NumPages" );


		aa->SetDrawOrder( DRAW_ORDER_OVERLAY+1 );
		this->AddChild( aa );
	}

	this->SortByDrawOrder();

	FOREACH( AutoActor, m_vPages, aa )
	{
		bool bIsFirst = aa == m_vPages.begin();
		(*aa)->PlayCommand( bIsFirst ? "Show" : "Hide" );
		(*aa)->PlayCommand( "On" );
	}

	this->SortByDrawOrder();

	// Reset stage number (not relevant in lessons)
	GAMESTATE->m_iCurrentStageIndex = 0;

	// Autoplay during demonstration
	FOREACH_EnabledPlayerInfo( m_vPlayerInfo, pi )
		pi->GetPlayerState()->m_PlayerController = PC_AUTOPLAY;
}

void ScreenGameplayLesson::Input( const InputEventPlus &input )
{
	//LOG->Trace( "ScreenGameplayLesson::Input()" );

	if( m_iCurrentPageIndex != -1 )
	{
		// show a lesson page
		Screen::Input( input );
	}
	else
	{
		// in the "your turn" section"
		ScreenGameplay::Input( input );
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
				STATSMAN->m_CurStageStats.CommitScores( false );
			}
			else if( bAnyTriesLeft )
			{
				ResetAndRestartCurrentSong();

				m_Try = (Try)(m_Try+1);
				MESSAGEMAN->Broadcast( (Message)(Message_LessonTry1+m_Try) );
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

void ScreenGameplayLesson::MenuStart( const InputEventPlus &input )
{
	// XXX: Allow repeats?
	if( m_iCurrentPageIndex == -1 )
		return;
	ChangeLessonPage( +1 );
}

void ScreenGameplayLesson::MenuBack( const InputEventPlus &input )
{
	// XXX: Allow repeats?
	if( m_iCurrentPageIndex == 0 )
	{
		BeginBackingOutFromGameplay();
		return;
	}
	if( m_iCurrentPageIndex == -1 )
		return;
	ChangeLessonPage( -1 );
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

		MESSAGEMAN->Broadcast( (Message)(Message_LessonTry1+m_Try) );

		// Change back to the current autoplay setting (in most cases, human controlled).
		FOREACH_EnabledPlayerInfo( m_vPlayerInfo, pi )
			pi->GetPlayerState()->m_PlayerController = PREFSMAN->m_AutoPlay;
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
