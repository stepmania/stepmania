#include "global.h"
/*
-----------------------------------------------------------------------------
 Class: ScreenSelectDifficulty

 Desc: See header.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/

#include "ScreenSelectDifficulty.h"
#include "ScreenManager.h"
#include "PrefsManager.h"
#include "RageSounds.h"
#include "GameManager.h"
#include "ThemeManager.h"
#include "GameState.h"
#include "AnnouncerManager.h"
#include "ModeChoice.h"
#include "ActorUtil.h"

#define NUM_CHOICES_ON_PAGE_1				THEME->GetMetricI("ScreenSelectDifficulty","NumChoicesOnPage1")
#define LOCK_INPUT_SECONDS					THEME->GetMetricF("ScreenSelectDifficulty","LockInputSeconds")
#define SLEEP_AFTER_CHOICE_SECONDS			THEME->GetMetricF("ScreenSelectDifficulty","SleepAfterChoiceSeconds")
#define SLEEP_AFTER_TWEEN_OFF_SECONDS		THEME->GetMetricF("ScreenSelectDifficulty","SleepAfterTweenOffSeconds")
#define CURSOR_CHOOSE_COMMAND				THEME->GetMetric ("ScreenSelectDifficulty","CursorChooseCommand")
#define CURSOR_OFFSET_X_FROM_PICTURE( p )	THEME->GetMetricF("ScreenSelectDifficulty",ssprintf("CursorP%dOffsetXFromPicture",p+1))
#define CURSOR_OFFSET_Y_FROM_PICTURE( p )	THEME->GetMetricF("ScreenSelectDifficulty",ssprintf("CursorP%dOffsetYFromPicture",p+1))
#define SHADOW_CHOOSE_COMMAND				THEME->GetMetric ("ScreenSelectDifficulty","ShadowChooseCommand")
#define SHADOW_LENGTH_X						THEME->GetMetricF("ScreenSelectDifficulty","ShadowLengthX")
#define SHADOW_LENGTH_Y						THEME->GetMetricF("ScreenSelectDifficulty","ShadowLengthY")
#define OK_CHOOSE_COMMAND					THEME->GetMetric ("ScreenSelectDifficulty","OKChooseCommand")
#define DISABLED_COLOR						THEME->GetMetricC("ScreenSelectDifficulty","DisabledColor")


ScreenSelectDifficulty::ScreenSelectDifficulty() : ScreenSelect( "ScreenSelectDifficulty" )
{
	m_CurrentPage = PAGE_1;
	int p;
	for( p=0; p<NUM_PLAYERS; p++ )
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
			CString sInfoFile = ssprintf( "ScreenSelectDifficulty info %s", m_ModeChoices[page][choice].name );
			CString sPictureFile = ssprintf( "ScreenSelectDifficulty picture %s", m_ModeChoices[page][choice].name );

			m_sprPicture[page][choice].SetName( ssprintf("PicturePage%dChoice%d",page+1,choice+1) );
			m_sprPicture[page][choice].Load( THEME->GetPathToG(sPictureFile) );
			m_framePages.AddChild( &m_sprPicture[page][choice] );

			m_sprInfo[page][choice].SetName( ssprintf("InfoPage%dChoice%d",page+1,choice+1) );
			m_sprInfo[page][choice].Load( THEME->GetPathToG(sInfoFile) );
			m_framePages.AddChild( &m_sprInfo[page][choice] );
		}

		m_sprMore[page].SetName( ssprintf("MorePage%d",page+1) );
		m_sprMore[page].Load( THEME->GetPathToG( ssprintf("ScreenSelectDifficulty more page%d",page+1) ) );
		m_framePages.AddChild( &m_sprMore[page] );

		m_sprExplanation[page].SetName( ssprintf("ExplanationPage%d",page+1) );
		m_sprExplanation[page].Load( THEME->GetPathToG( "ScreenSelectDifficulty explanation") );
		m_sprExplanation[page].StopAnimating();
		m_sprExplanation[page].SetState( page );
		m_framePages.AddChild( &m_sprExplanation[page] );
	}


	for( p=0; p<NUM_PLAYERS; p++ )
	{
		CLAMP( m_iChoiceOnPage[p], 0, (int)m_ModeChoices[0].size()-1 );
		m_bChosen[p] = false;

		if( !GAMESTATE->IsHumanPlayer(p) )
			continue;

		m_sprShadow[p].SetName( "Shadow" );
		m_sprShadow[p].Load( THEME->GetPathToG( "ScreenSelectDifficulty shadow 2x1") );
		m_sprShadow[p].StopAnimating();
		m_sprShadow[p].SetState( p );
		m_sprShadow[p].SetDiffuse( RageColor(0,0,0,0.6f) );
		m_framePages.AddChild( &m_sprShadow[p] );

		m_sprCursor[p].SetName( "Cursor" );
		m_sprCursor[p].Load( THEME->GetPathToG( "ScreenSelectDifficulty cursor 2x1") );
		m_sprCursor[p].StopAnimating();
		m_sprCursor[p].SetState( p );
		m_framePages.AddChild( &m_sprCursor[p] );

		m_sprOK[p].SetName( "OK" );
		m_sprOK[p].Load( THEME->GetPathToG( "ScreenSelectDifficulty ok 2x1") );
		m_sprOK[p].SetState( p );
		m_sprOK[p].StopAnimating();
		m_sprOK[p].SetDiffuse( RageColor(1,1,1,0) );
		m_framePages.AddChild( &m_sprOK[p] );
	}

	this->AddChild( &m_framePages );
	
	m_soundChange.Load( THEME->GetPathToS( "ScreenSelectDifficulty change") );
	m_soundSelect.Load( THEME->GetPathToS( "Common start") );
	m_soundDifficult.Load( ANNOUNCER->GetPathTo("select difficulty challenge") );

	m_fLockInputTime = LOCK_INPUT_SECONDS;

	TweenOnScreen();
}

void ScreenSelectDifficulty::Update( float fDelta )
{
	ScreenSelect::Update( fDelta );
	m_fLockInputTime = max( 0, m_fLockInputTime-fDelta );
}

void ScreenSelectDifficulty::HandleScreenMessage( const ScreenMessage SM )
{
	ScreenSelect::HandleScreenMessage( SM );

	switch( SM )
	{
	case SM_BeginFadingOut:
		TweenOffScreen();
		SCREENMAN->PostMessageToTopScreen( SM_AllDoneChoosing, SLEEP_AFTER_TWEEN_OFF_SECONDS );	// nofify parent that we're finished
		m_Menu.m_MenuTimer.Stop();
		break;
	}
}

int ScreenSelectDifficulty::GetSelectionIndex( PlayerNumber pn )
{
	int index = 0;
	for( int page=0; page<m_CurrentPage; page++ )
		index += m_ModeChoices[page].size();
	index += m_iChoiceOnPage[pn];
	return index;
}

void ScreenSelectDifficulty::UpdateSelectableChoices()
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
				m_sprInfo[page][i].SetDiffuse( RageColor(1,1,1,1) );
				m_sprPicture[page][i].SetDiffuse( RageColor(1,1,1,1) );
			}
			else
			{
				m_sprInfo[page][i].SetDiffuse( DISABLED_COLOR );
				m_sprPicture[page][i].SetDiffuse( DISABLED_COLOR );
			}
		}
	}

	for( int p=0; p<NUM_PLAYERS; p++ )
		if( GAMESTATE->IsHumanPlayer(p) )
		{
			MenuRight( (PlayerNumber)p );
			MenuLeft( (PlayerNumber)p );
		}
}

void ScreenSelectDifficulty::MenuLeft( PlayerNumber pn )
{
//	if( m_fLockInputTime > 0 )
//		return;
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


void ScreenSelectDifficulty::MenuRight( PlayerNumber pn )
{
//	if( m_fLockInputTime > 0 )
//		return;
	if( m_bChosen[pn] )
		return;
	if( m_iChoiceOnPage[pn] == (int)m_ModeChoices[m_CurrentPage].size()-1 )	// can't go right any more
	{
		if( m_ModeChoices[m_CurrentPage+1].size()==0 )	// there is no page 2
			return;

		if( m_CurrentPage < NUM_PAGES-1 )
			ChangePage( (Page)(m_CurrentPage+1) );
	}
	else
		ChangeWithinPage( pn, m_iChoiceOnPage[pn]+1, false );
}

void ScreenSelectDifficulty::ChangePage( Page newPage )
{
	int p;

	// If anyone has already chosen, don't allow changing of pages
	for( p=0; p<NUM_PLAYERS; p++ )
		if( GAMESTATE->IsHumanPlayer(p) && m_bChosen[p] )
			return;

	bool bPageIncreasing = newPage > m_CurrentPage;
	m_CurrentPage = newPage;

	if( newPage == PAGE_2 )
	{
		/* XXX: only play this once (I thought we already did that?) */
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
	m_framePages.SetX( (float)newPage*-SCREEN_WIDTH );
}

void ScreenSelectDifficulty::ChangeWithinPage( PlayerNumber pn, int iNewChoice, bool bChangingPages )
{
	for( int p=0; p<NUM_PLAYERS; p++ )
	{
		if( !GAMESTATE->IsHumanPlayer(p) )
			continue;	// skip

		if( p!=pn && m_CurrentPage==PAGE_1 )
			continue;	// skip

		m_iChoiceOnPage[p] = iNewChoice;

		float fCursorX = GetCursorX( (PlayerNumber)p );
		float fCursorY = GetCursorY( (PlayerNumber)p );

		m_sprCursor[p].StopTweening();
		m_sprCursor[p].BeginTweening( 0.2f, bChangingPages ? TWEEN_LINEAR : TWEEN_DECELERATE );
		m_sprCursor[p].SetX( fCursorX );
		m_sprCursor[p].SetY( fCursorY );

		m_sprShadow[p].StopTweening();
		m_sprShadow[p].BeginTweening( 0.2f, bChangingPages ? TWEEN_LINEAR : TWEEN_DECELERATE );
		m_sprShadow[p].SetX( fCursorX + SHADOW_LENGTH_X );
		m_sprShadow[p].SetY( fCursorY + SHADOW_LENGTH_Y );
	}

	/* If we're changing pages, it's ChangePage's responsibility to play this
	 * (so we don't play it more than once). */
	if(!bChangingPages)
		m_soundChange.Play();
}

void ScreenSelectDifficulty::MenuStart( PlayerNumber pn )
{
	if( m_fLockInputTime > 0 )
		return;
	if( m_bChosen[pn] == true )
		return;
	m_bChosen[pn] = true;

	for( int page=0; page<NUM_PAGES; page++ )
		OFF_COMMAND( m_sprMore[page] );

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
		SOUND->PlayOnceFromDir( ANNOUNCER->GetPathTo(ssprintf("ScreenSelectDifficulty comment %s",mc.name)) );
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
	for( p=0; p<NUM_PLAYERS; p++ )
	{
		if( GAMESTATE->IsHumanPlayer((PlayerNumber)p)  &&  m_bChosen[p] == false )
			return;
	}
	this->PostScreenMessage( SM_BeginFadingOut, SLEEP_AFTER_CHOICE_SECONDS );	// tell our owner it's time to move on
}

void ScreenSelectDifficulty::TweenOnScreen() 
{
	unsigned p;

	for( int page=0; page<NUM_PAGES; page++ )
	{
		SET_XY_AND_ON_COMMAND( m_sprExplanation[page] );
		SET_XY_AND_ON_COMMAND( m_sprMore[page] );

		for( unsigned c=0; c<m_ModeChoices[page].size(); c++ )
		{			
			SET_XY_AND_ON_COMMAND( m_sprInfo[page][c] );
			SET_XY_AND_ON_COMMAND( m_sprPicture[page][c] );
		}
	}

	for( p=0; p<NUM_PLAYERS; p++ )
	{
		if( !GAMESTATE->IsHumanPlayer((PlayerNumber)p) )
			continue;

		float fCursorX = GetCursorX( (PlayerNumber)p );
		float fCursorY = GetCursorY( (PlayerNumber)p );

		m_sprCursor[p].SetXY( fCursorX, fCursorY );
		ON_COMMAND( m_sprCursor[p] );

		m_sprShadow[p].SetXY( fCursorX + SHADOW_LENGTH_X, fCursorY + SHADOW_LENGTH_Y );
		ON_COMMAND( m_sprShadow[p] );
	}
}

void ScreenSelectDifficulty::TweenOffScreen()
{	
	const int page = m_CurrentPage;

	OFF_COMMAND( m_sprExplanation[page] );
	OFF_COMMAND( m_sprMore[page] );

	for( int p=0; p<NUM_PLAYERS; p++ )
	{
		if( !GAMESTATE->IsHumanPlayer(p) )
			continue;

		OFF_COMMAND( m_sprCursor[p] );
		OFF_COMMAND( m_sprOK[p] );
		OFF_COMMAND( m_sprShadow[p] );
	}

	for( unsigned c=0; c<m_ModeChoices[page].size(); c++ )
	{
		OFF_COMMAND( m_sprInfo[page][c] );
		OFF_COMMAND( m_sprPicture[page][c] );
	}
}


float ScreenSelectDifficulty::GetCursorX( PlayerNumber pn )
{
	return m_sprPicture[m_CurrentPage][m_iChoiceOnPage[pn]].GetX() + CURSOR_OFFSET_X_FROM_PICTURE(pn);
}

float ScreenSelectDifficulty::GetCursorY( PlayerNumber pn )
{
	return m_sprPicture[m_CurrentPage][m_iChoiceOnPage[pn]].GetY() + CURSOR_OFFSET_Y_FROM_PICTURE(pn);
}
