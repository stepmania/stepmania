#include "global.h"
#include "ScreenGameplayLesson.h"
#include "RageLog.h"
#include "GameState.h"
#include "PrefsManager.h"

REGISTER_SCREEN_CLASS( ScreenGameplayLesson );
ScreenGameplayLesson::ScreenGameplayLesson( CString sName ) : ScreenGameplayNormal( sName )
{
	LOG->Trace( "ScreenGameplayLesson::ScreenGameplayLesson()" );

	m_iCurrentLessonPageIndex = 0;
}

void ScreenGameplayLesson::Init()
{
	ASSERT( GAMESTATE->m_pCurStyle );
	ASSERT( GAMESTATE->m_pCurSong );

	/* Now that we've set up, init the base class. */
	ScreenGameplay::Init();

	ClearMessageQueue();	// remove all of the messages set in ScreenGameplay that animate "ready", "here we go", etc.

	GAMESTATE->m_bPastHereWeGo = true;

	m_DancingState = STATE_DANCING;


	// Load lessons
	Song *pSong = GAMESTATE->m_pCurSong;
	CString sDir = pSong->GetSongDir();
	vector<CString> vs;
	GetDirListing( sDir+"Page*", vs, true, true );
	m_vLessonPages.resize( vs.size() );
	FOREACH( CString, vs, s )
	{
		int i = s - vs.begin();
		AutoActor &aa = m_vLessonPages[i];


		LUA->SetGlobal( "PageIndex", i );
		LUA->SetGlobal( "NumPages", (int)vs.size() );
		aa.Load( *s );
		LUA->UnsetGlobal( "PageIndex" );
		LUA->UnsetGlobal( "NumPages" );


		aa->SetDrawOrder( DRAW_ORDER_OVERLAY );
		this->AddChild( aa );
	}

	this->SortByDrawOrder();

	// Show the first lesson page
	if( !m_vLessonPages.empty() )
	{
		AutoActor &aa = m_vLessonPages[0];
		aa->PlayCommand( "Show" );
	}

	FOREACH( AutoActor, m_vLessonPages, aa )
		(*aa)->PlayCommand( "On" );
}

void ScreenGameplayLesson::Input( const InputEventPlus &input )
{
	//LOG->Trace( "ScreenGameplayLesson::Input()" );

	if( m_iCurrentLessonPageIndex != -1 )
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
		bool bShowingAPage = m_iCurrentLessonPageIndex != -1;

		if( bShowingAPage )
		{
			// reload and loop
		}
		else
		{
			// show finish message if passed, loop if failed
		}
	}

	ScreenGameplay::HandleScreenMessage( SM );
}

void ScreenGameplayLesson::MenuStart( PlayerNumber pn )
{
	if( m_iCurrentLessonPageIndex == -1 )
		return;
	ChangeLessonPage( +1 );
}

void ScreenGameplayLesson::MenuBack( PlayerNumber pn )
{
	if( m_iCurrentLessonPageIndex == -1 )
		return;
	ChangeLessonPage( -1 );
}

void ScreenGameplayLesson::ChangeLessonPage( int iDir )
{
	if( m_iCurrentLessonPageIndex + iDir < 0 )
	{
		// don't change
		return;
	}
	else if( m_iCurrentLessonPageIndex + iDir >= m_vLessonPages.size() )
	{
		// dismissed the last page.  Proceed to the "your turn" portion.
		FOREACH( AutoActor, m_vLessonPages, aa )
			(*aa)->PlayCommand( "Off" );

		m_iCurrentLessonPageIndex = -1;
	}
	else
	{
		m_vLessonPages[m_iCurrentLessonPageIndex]->PlayCommand( "Hide" );
		m_iCurrentLessonPageIndex += iDir;
		m_vLessonPages[m_iCurrentLessonPageIndex]->PlayCommand( "Show" );
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
