#include "global.h"
/*
-----------------------------------------------------------------------------
 Class: ScreenSelectMaxType2

 Desc: See header.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/

#include "ScreenSelectMaxType2.h"
#include "GameManager.h"
#include "ThemeManager.h"
#include "PrefsManager.h"
#include "ScreenManager.h"
#include "GameState.h"
#include "AnnouncerManager.h"
#include "ModeChoice.h"


#define NUM_CHOICES_ON_PAGE_1				THEME->GetMetricI("ScreenSelectMaxType2","NumChoicesOnPage1")
#define LOCK_INPUT_SECONDS					THEME->GetMetricF("ScreenSelectMaxType2","LockInputSeconds")
#define SLEEP_AFTER_CHOICE_SECONDS			THEME->GetMetricF("ScreenSelectMaxType2","SleepAfterChoiceSeconds")
#define SLEEP_AFTER_TWEEN_OFF_SECONDS		THEME->GetMetricF("ScreenSelectMaxType2","SleepAfterTweenOffSeconds")
#define MORE_ON_COMMAND( page )				THEME->GetMetric ("ScreenSelectMaxType2",ssprintf("MorePage%dOnCommand",page+1))
#define MORE_OFF_COMMAND( page )			THEME->GetMetric ("ScreenSelectMaxType2",ssprintf("MorePage%dOffCommand",page+1))
#define EXPLANATION_ON_COMMAND( page )		THEME->GetMetric ("ScreenSelectMaxType2",ssprintf("ExplanationPage%dOnCommand",page+1))
#define EXPLANATION_OFF_COMMAND( page )		THEME->GetMetric ("ScreenSelectMaxType2",ssprintf("ExplanationPage%dOffCommand",page+1))
#define HEADER_ON_COMMAND( page, choice )	THEME->GetMetric ("ScreenSelectMaxType2",ssprintf("HeaderPage%dChoice%dOnCommand",page+1,choice+1))
#define HEADER_OFF_COMMAND( page, choice )	THEME->GetMetric ("ScreenSelectMaxType2",ssprintf("HeaderPage%dChoice%dOffCommand",page+1,choice+1))
#define PICTURE_ON_COMMAND( page, choice )	THEME->GetMetric ("ScreenSelectMaxType2",ssprintf("PicturePage%dChoice%dOnCommand",page+1,choice+1))
#define PICTURE_OFF_COMMAND( page, choice )	THEME->GetMetric ("ScreenSelectMaxType2",ssprintf("PicturePage%dChoice%dOffCommand",page+1,choice+1))
#define CURSOR_ON_COMMAND					THEME->GetMetric ("ScreenSelectMaxType2","CursorOnCommand")
#define CURSOR_CHOOSE_COMMAND				THEME->GetMetric ("ScreenSelectMaxType2","CursorChooseCommand")
#define CURSOR_OFF_COMMAND					THEME->GetMetric ("ScreenSelectMaxType2","CursorOffCommand")
#define CURSOR_OFFSET_X_FROM_PICTURE( p )	THEME->GetMetricF("ScreenSelectMaxType2",ssprintf("CursorP%dOffsetXFromPicture",p+1))
#define CURSOR_OFFSET_Y_FROM_PICTURE( p )	THEME->GetMetricF("ScreenSelectMaxType2",ssprintf("CursorP%dOffsetYFromPicture",p+1))
#define SHADOW_ON_COMMAND					THEME->GetMetric ("ScreenSelectMaxType2","ShadowOnCommand")
#define SHADOW_CHOOSE_COMMAND				THEME->GetMetric ("ScreenSelectMaxType2","ShadowChooseCommand")
#define SHADOW_OFF_COMMAND					THEME->GetMetric ("ScreenSelectMaxType2","ShadowOffCommand")
#define SHADOW_LENGTH_X						THEME->GetMetricF("ScreenSelectMaxType2","ShadowLengthX")
#define SHADOW_LENGTH_Y						THEME->GetMetricF("ScreenSelectMaxType2","ShadowLengthY")
#define OK_ON_COMMAND						THEME->GetMetric ("ScreenSelectMaxType2","OKOnCommand")
#define OK_CHOOSE_COMMAND					THEME->GetMetric ("ScreenSelectMaxType2","OKChooseCommand")
#define OK_OFF_COMMAND						THEME->GetMetric ("ScreenSelectMaxType2","OKOffCommand")
#define DISABLED_COLOR						THEME->GetMetricC("ScreenSelectMaxType2","DisabledColor")



ScreenSelectMaxType2::ScreenSelectMaxType2() : ScreenSelect( "ScreenSelectMaxType2" )
{
	m_CurrentPage = PAGE_1;
	for( int p=0; p<NUM_PLAYERS; p++ )
	{
		m_iChoiceOnPage[p] = 0;
		m_bChosen[p] = false;
	}

	for( unsigned c=0; c<m_aModeChoices.size(); c++ )
	{
		if( (int)c < NUM_CHOICES_ON_PAGE_1 )
			m_ModeChoices[PAGE_1].push_back( m_aModeChoices[c] );
		else
			m_ModeChoices[PAGE_2].push_back( m_aModeChoices[c] );
	}

	for( int page=0; page<NUM_PAGES; page++ )
	{

		for( unsigned choice=0; choice<m_ModeChoices[page].size(); choice++ )
		{
			CString sHeaderFile = ssprintf( "ScreenSelectMaxType2 header %s", m_ModeChoices[page][choice].name );
			CString sPictureFile = ssprintf( "ScreenSelectMaxType2 picture %s", m_ModeChoices[page][choice].name );

			m_sprPicture[page][choice].Load( THEME->GetPathTo("Graphics",sPictureFile) );
			m_framePages.AddChild( &m_sprPicture[page][choice] );

			m_sprHeader[page][choice].Load( THEME->GetPathTo("Graphics",sHeaderFile) );
			m_framePages.AddChild( &m_sprHeader[page][choice] );
		}

		
		m_sprMore[page].Load( THEME->GetPathTo("Graphics", ssprintf("ScreenSelectMaxType2 more page%d",page+1) ) );
		m_framePages.AddChild( &m_sprMore[page] );

		m_sprExplanation[page].Load( THEME->GetPathTo("Graphics", "ScreenSelectMaxType2 explanation") );
		m_sprExplanation[page].StopAnimating();
		m_sprExplanation[page].SetState( page );
		m_framePages.AddChild( &m_sprExplanation[page] );
	}


	for( p=0; p<NUM_PLAYERS; p++ )
	{
		CLAMP( m_iChoiceOnPage[p], 0, (int)m_ModeChoices[0].size()-1 );
		m_bChosen[p] = false;

		if( !GAMESTATE->IsPlayerEnabled((PlayerNumber)p) )
			continue;

		m_sprShadow[p].Load( THEME->GetPathTo("Graphics", "ScreenSelectMaxType2 shadow 2x1") );
		m_sprShadow[p].StopAnimating();
		m_sprShadow[p].SetState( p );
		m_sprShadow[p].SetDiffuse( RageColor(0,0,0,0.6f) );
		m_framePages.AddChild( &m_sprShadow[p] );

		m_sprCursor[p].Load( THEME->GetPathTo("Graphics", "ScreenSelectMaxType2 cursor 2x1") );
		m_sprCursor[p].StopAnimating();
		m_sprCursor[p].SetState( p );
		m_framePages.AddChild( &m_sprCursor[p] );

		m_sprOK[p].Load( THEME->GetPathTo("Graphics", "ScreenSelectMaxType2 ok 2x1") );
		m_sprOK[p].SetState( p );
		m_sprOK[p].StopAnimating();
		m_sprOK[p].SetDiffuse( RageColor(1,1,1,0) );
		m_framePages.AddChild( &m_sprOK[p] );
	}

	this->AddChild( &m_framePages );
	
	m_soundChange.Load( THEME->GetPathTo("Sounds", "ScreenSelectMaxType2 change") );
	m_soundSelect.Load( THEME->GetPathTo("Sounds", "menu start") );
	m_soundDifficult.Load( ANNOUNCER->GetPathTo("ScreenSelectMaxType2 challenge") );

	m_fLockInputTime = LOCK_INPUT_SECONDS;

	TweenOnScreen();
}

void ScreenSelectMaxType2::Update( float fDelta )
{
	ScreenSelect::Update( fDelta );
	m_fLockInputTime = max( 0, m_fLockInputTime-fDelta );
}

void ScreenSelectMaxType2::HandleScreenMessage( const ScreenMessage SM )
{
	ScreenSelect::HandleScreenMessage( SM );

	switch( SM )
	{
	case SM_BeginFadingOut:
		TweenOffScreen();
		SCREENMAN->SendMessageToTopScreen( SM_AllDoneChoosing, SLEEP_AFTER_CHOICE_SECONDS );	// nofify parent that we're finished
		break;
	}
}

int ScreenSelectMaxType2::GetSelectionIndex( PlayerNumber pn )
{
	int index = 0;
	for( int page=0; page<m_CurrentPage; page++ )
		index += m_ModeChoices[page].size();
	index += m_iChoiceOnPage[pn];
	return index;
}

void ScreenSelectMaxType2::UpdateSelectableChoices()
{
	for( int page=0; page<NUM_PAGES; page++ )
	{
		/* XXX: If a player joins during the tween-in, this diffuse change
		 * will be undone by the tween.  Hmm. */
		for( unsigned i=0; i<m_ModeChoices[page].size(); i++ )
		{
			/* If the icon is text, use a dimmer diffuse, or we won't be
			 * able to see the glow. */
			if( GAMESTATE->IsPlayable(m_ModeChoices[page][i]) )
			{
				m_sprHeader[page][i].SetDiffuse( RageColor(1,1,1,1) );
				m_sprPicture[page][i].SetDiffuse( RageColor(1,1,1,1) );
			}
			else
			{
				m_sprHeader[page][i].SetDiffuse( DISABLED_COLOR );
				m_sprPicture[page][i].SetDiffuse( DISABLED_COLOR );
			}
		}
	}

	for( int p=0; p<NUM_PLAYERS; p++ )
		if( GAMESTATE->IsPlayerEnabled(p) )
		{
			MenuRight( (PlayerNumber)p );
			MenuLeft( (PlayerNumber)p );
		}
}

void ScreenSelectMaxType2::MenuLeft( PlayerNumber pn )
{
	if( m_fLockInputTime > 0 )
		return;
	if( m_bChosen[pn] )
		return;
	if( m_iChoiceOnPage[pn] == 0 )	// can't go left any more
	{
		if( m_CurrentPage > 0 )
			ChangePage( (Page)(m_CurrentPage-1) );
	}
	else
		ChangeWithinPage( pn, m_iChoiceOnPage[pn]-1, false );
}


void ScreenSelectMaxType2::MenuRight( PlayerNumber pn )
{
	if( m_fLockInputTime > 0 )
		return;
	if( m_bChosen[pn] )
		return;
	if( m_iChoiceOnPage[pn] == (int)m_ModeChoices[m_CurrentPage].size()-1 )	// can't go left any more
	{
		if( m_CurrentPage < NUM_PAGES-1 )
			ChangePage( (Page)(m_CurrentPage+1) );
	}
	else
		ChangeWithinPage( pn, m_iChoiceOnPage[pn]+1, false );
}

void ScreenSelectMaxType2::ChangePage( Page newPage )
{
	int p;

	// If anyone has already chosen, don't allow changing of pages
	for( p=0; p<NUM_PLAYERS; p++ )
		if( GAMESTATE->IsPlayerEnabled(p) && m_bChosen[p] )
			return;

	bool bPageIncreasing = newPage > m_CurrentPage;
	m_CurrentPage = newPage;

	if( newPage == PAGE_2 )
	{
		m_soundDifficult.Stop();
		m_soundDifficult.PlayRandom();
	}

	// change both players
	int iNewChoice = bPageIncreasing ? 0 : m_ModeChoices[m_CurrentPage].size()-1;
	for( p=0; p<NUM_PLAYERS; p++ )
		ChangeWithinPage( (PlayerNumber)p, iNewChoice, true );

	m_soundChange.Play();

	// move frame with choices
	m_framePages.StopTweening();
	m_framePages.BeginTweening( 0.2f );
	m_framePages.SetTweenX( (float)newPage*-SCREEN_WIDTH );
}

void ScreenSelectMaxType2::ChangeWithinPage( PlayerNumber pn, int iNewChoice, bool bChangingPages )
{
	for( int p=0; p<NUM_PLAYERS; p++ )
	{
		if( !GAMESTATE->IsPlayerEnabled(p) )
			continue;	// skip

		if( p!=pn && m_CurrentPage==PAGE_1 )
			continue;	// skip

		m_iChoiceOnPage[p] = iNewChoice;

		float fCursorX = GetCursorX( (PlayerNumber)p );
		float fCursorY = GetCursorY( (PlayerNumber)p );

		m_sprCursor[p].StopTweening();
		m_sprCursor[p].BeginTweening( 0.2f, bChangingPages ? TWEEN_LINEAR : TWEEN_DECELERATE );
		m_sprCursor[p].SetTweenX( fCursorX );
		m_sprCursor[p].SetTweenY( fCursorY );

		m_sprShadow[p].StopTweening();
		m_sprShadow[p].BeginTweening( 0.2f, bChangingPages ? TWEEN_LINEAR : TWEEN_DECELERATE );
		m_sprShadow[p].SetTweenX( fCursorX + SHADOW_LENGTH_X );
		m_sprShadow[p].SetTweenY( fCursorY + SHADOW_LENGTH_Y );
	}

	/* If we're changing pages, it's ChangePage's responsibility to play this
	 * (so we don't play it more than once). */
	if(!bChangingPages)
		m_soundChange.Play();
}

void ScreenSelectMaxType2::MenuStart( PlayerNumber pn )
{
	if( m_fLockInputTime > 0 )
		return;
	if( m_bChosen[pn] == true )
		return;
	m_bChosen[pn] = true;

	for( unsigned page=0; page<NUM_PAGES; page++ )
		m_sprMore[page].FadeOff( 0, "fade", 0.5f );

	const ModeChoice& mc = m_ModeChoices[m_CurrentPage][m_iChoiceOnPage[pn]];
	/* Don't play sound if we're on the second page and another player
	 * has already selected, since it just played. */
	bool AnotherPlayerSelected = false;
	int p;
	for( p=0; p<NUM_PLAYERS; p++ )
		if(p != pn && m_bChosen[p])
			AnotherPlayerSelected = true;

	if(m_CurrentPage != PAGE_2 || !AnotherPlayerSelected)
	{
		SOUNDMAN->PlayOnceFromDir( ANNOUNCER->GetPathTo(ssprintf("ScreenSelectMaxType2 comment %s",mc.name)) );
		m_soundSelect.Play();
	}

	if( m_CurrentPage == PAGE_2 )
	{
		// choose this for all the other players too
		for( p=0; p<NUM_PLAYERS; p++ )
		{
			if( m_bChosen[p] )
				continue;
		
			MenuStart( (PlayerNumber)p );
		}
	}


	m_sprCursor[pn].Command( CURSOR_CHOOSE_COMMAND );
	m_sprOK[pn].SetXY( m_sprShadow[pn].GetX(), m_sprShadow[pn].GetY() );
	m_sprOK[pn].Command( OK_CHOOSE_COMMAND );
	m_sprShadow[pn].Command( SHADOW_CHOOSE_COMMAND );


	// check to see if everyone has chosen
	for( int p=0; p<NUM_PLAYERS; p++ )
	{
		if( GAMESTATE->IsPlayerEnabled((PlayerNumber)p)  &&  m_bChosen[p] == false )
			return;
	}
	this->SendScreenMessage( SM_BeginFadingOut, SLEEP_AFTER_CHOICE_SECONDS );	// tell our owner it's time to move on
}


void ScreenSelectMaxType2::MenuBack( PlayerNumber pn )
{
}

void ScreenSelectMaxType2::TweenOnScreen() 
{
	unsigned p;

	for( int page=0; page<NUM_PAGES; page++ )
	{
//		m_sprExplanation[PAGE_1].Command( "x,-300;y,70;sleep,0.7;bounceend,0.5f;x,170" );
//		m_sprMore[PAGE_1].Command( "x,580;y,90;diffuse,1,1,1,0;linear,0.5;diffuse,1,1,1,1" );
		m_sprExplanation[page].Command( EXPLANATION_ON_COMMAND(page) );
		m_sprMore[page].Command( MORE_ON_COMMAND(page) );

		for( unsigned c=0; c<m_ModeChoices[page].size(); c++ )
		{
			// fly on
//			m_sprHeader[p][c].FadeOn( fPause, "left far accelerate", 0.4f );
			m_sprHeader[page][c].Command( HEADER_ON_COMMAND(page,c) );

			// roll down
//			m_sprPicture[p][c].FadeOn( fPause+0.4f, "foldy bounce", 0.3f );
			m_sprPicture[page][c].Command( PICTURE_ON_COMMAND(page,c) );
		}
	}

	for( p=0; p<NUM_PLAYERS; p++ )
	{
		if( !GAMESTATE->IsPlayerEnabled((PlayerNumber)p) )
			continue;

		float fCursorX = GetCursorX( (PlayerNumber)p );
		float fCursorY = GetCursorY( (PlayerNumber)p );

		m_sprCursor[p].SetXY( fCursorX, fCursorY );
		m_sprCursor[p].Command( CURSOR_ON_COMMAND );

		m_sprShadow[p].SetXY( fCursorX + SHADOW_LENGTH_X, fCursorY + SHADOW_LENGTH_Y );
		m_sprShadow[p].Command( SHADOW_ON_COMMAND );
	}
}

void ScreenSelectMaxType2::TweenOffScreen()
{	
	const int page = m_CurrentPage;

	m_sprExplanation[page].Command( "sleep,0.7;bouncebegin,0.5f;x,-300" );
	m_sprMore[page].FadeOff( 0.7f, "fade", 0.5f );


	for( int p=0; p<NUM_PLAYERS; p++ )
	{
		if( !GAMESTATE->IsPlayerEnabled((PlayerNumber)p) )
			continue;

		m_sprCursor[p].Command( CURSOR_OFF_COMMAND );
		m_sprOK[p].Command( OK_OFF_COMMAND );
		m_sprShadow[p].Command( SHADOW_OFF_COMMAND );
	}

	for( unsigned c=0; c<m_ModeChoices[page].size(); c++ )
	{
//		const float fPause = c*0.2f;

		// roll up
		//m_sprPicture[page][c].FadeOff( fPause, "foldy bounce", 0.3f );
		m_sprPicture[page][c].Command( PICTURE_OFF_COMMAND(page,c) );

		// fly off
		//m_sprHeader[page][c].FadeOff( fPause+0.3f, "left far accelerate", 0.4f );
		m_sprHeader[page][c].Command( HEADER_OFF_COMMAND(page,c) );
	}
}


float ScreenSelectMaxType2::GetCursorX( PlayerNumber pn )
{
	return m_sprPicture[m_CurrentPage][m_iChoiceOnPage[pn]].GetX() + CURSOR_OFFSET_X_FROM_PICTURE(pn);
}

float ScreenSelectMaxType2::GetCursorY( PlayerNumber pn )
{
	return m_sprPicture[m_CurrentPage][m_iChoiceOnPage[pn]].GetY() + CURSOR_OFFSET_Y_FROM_PICTURE(pn);
}
