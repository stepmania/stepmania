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
#include "GameManager.h"
#include "ThemeManager.h"
#include "PrefsManager.h"
#include "ScreenManager.h"
#include "GameState.h"
#include "AnnouncerManager.h"
#include "ModeChoice.h"


#define NUM_CHOICES_ON_PAGE_1				THEME->GetMetricI("ScreenSelectDifficulty","NumChoicesOnPage1")
#define LOCK_INPUT_SECONDS					THEME->GetMetricF("ScreenSelectDifficulty","LockInputSeconds")
#define SLEEP_AFTER_CHOICE_SECONDS			THEME->GetMetricF("ScreenSelectDifficulty","SleepAfterChoiceSeconds")
#define SLEEP_AFTER_TWEEN_OFF_SECONDS		THEME->GetMetricF("ScreenSelectDifficulty","SleepAfterTweenOffSeconds")
#define MORE_ON_COMMAND( page )				THEME->GetMetric ("ScreenSelectDifficulty",ssprintf("MorePage%dOnCommand",page+1))
#define MORE_OFF_COMMAND( page )			THEME->GetMetric ("ScreenSelectDifficulty",ssprintf("MorePage%dOffCommand",page+1))
#define EXPLANATION_ON_COMMAND( page )		THEME->GetMetric ("ScreenSelectDifficulty",ssprintf("ExplanationPage%dOnCommand",page+1))
#define EXPLANATION_OFF_COMMAND( page )		THEME->GetMetric ("ScreenSelectDifficulty",ssprintf("ExplanationPage%dOffCommand",page+1))
#define INFO_ON_COMMAND( page, choice )		THEME->GetMetric ("ScreenSelectDifficulty",ssprintf("InfoPage%dChoice%dOnCommand",page+1,choice+1))
#define INFO_OFF_COMMAND( page, choice )	THEME->GetMetric ("ScreenSelectDifficulty",ssprintf("InfoPage%dChoice%dOffCommand",page+1,choice+1))
#define PICTURE_ON_COMMAND( page, choice )	THEME->GetMetric ("ScreenSelectDifficulty",ssprintf("PicturePage%dChoice%dOnCommand",page+1,choice+1))
#define PICTURE_OFF_COMMAND( page, choice )	THEME->GetMetric ("ScreenSelectDifficulty",ssprintf("PicturePage%dChoice%dOffCommand",page+1,choice+1))
#define CURSOR_ON_COMMAND					THEME->GetMetric ("ScreenSelectDifficulty","CursorOnCommand")
#define CURSOR_CHOOSE_COMMAND				THEME->GetMetric ("ScreenSelectDifficulty","CursorChooseCommand")
#define CURSOR_OFF_COMMAND					THEME->GetMetric ("ScreenSelectDifficulty","CursorOffCommand")
#define CURSOR_OFFSET_X_FROM_PICTURE( p )	THEME->GetMetricF("ScreenSelectDifficulty",ssprintf("CursorP%dOffsetXFromPicture",p+1))
#define CURSOR_OFFSET_Y_FROM_PICTURE( p )	THEME->GetMetricF("ScreenSelectDifficulty",ssprintf("CursorP%dOffsetYFromPicture",p+1))
#define SHADOW_ON_COMMAND					THEME->GetMetric ("ScreenSelectDifficulty","ShadowOnCommand")
#define SHADOW_CHOOSE_COMMAND				THEME->GetMetric ("ScreenSelectDifficulty","ShadowChooseCommand")
#define SHADOW_OFF_COMMAND					THEME->GetMetric ("ScreenSelectDifficulty","ShadowOffCommand")
#define SHADOW_LENGTH_X						THEME->GetMetricF("ScreenSelectDifficulty","ShadowLengthX")
#define SHADOW_LENGTH_Y						THEME->GetMetricF("ScreenSelectDifficulty","ShadowLengthY")
#define OK_ON_COMMAND						THEME->GetMetric ("ScreenSelectDifficulty","OKOnCommand")
#define OK_CHOOSE_COMMAND					THEME->GetMetric ("ScreenSelectDifficulty","OKChooseCommand")
#define OK_OFF_COMMAND						THEME->GetMetric ("ScreenSelectDifficulty","OKOffCommand")
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

			m_sprPicture[page][choice].Load( THEME->GetPathTo("Graphics",sPictureFile) );
			m_framePages.AddChild( &m_sprPicture[page][choice] );

			m_sprInfo[page][choice].Load( THEME->GetPathTo("Graphics",sInfoFile) );
			m_framePages.AddChild( &m_sprInfo[page][choice] );
		}

		
		m_sprMore[page].Load( THEME->GetPathTo("Graphics", ssprintf("ScreenSelectDifficulty more page%d",page+1) ) );
		m_framePages.AddChild( &m_sprMore[page] );

		m_sprExplanation[page].Load( THEME->GetPathTo("Graphics", "ScreenSelectDifficulty explanation") );
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

		m_sprShadow[p].Load( THEME->GetPathTo("Graphics", "ScreenSelectDifficulty shadow 2x1") );
		m_sprShadow[p].StopAnimating();
		m_sprShadow[p].SetState( p );
		m_sprShadow[p].SetDiffuse( RageColor(0,0,0,0.6f) );
		m_framePages.AddChild( &m_sprShadow[p] );

		m_sprCursor[p].Load( THEME->GetPathTo("Graphics", "ScreenSelectDifficulty cursor 2x1") );
		m_sprCursor[p].StopAnimating();
		m_sprCursor[p].SetState( p );
		m_framePages.AddChild( &m_sprCursor[p] );

		m_sprOK[p].Load( THEME->GetPathTo("Graphics", "ScreenSelectDifficulty ok 2x1") );
		m_sprOK[p].SetState( p );
		m_sprOK[p].StopAnimating();
		m_sprOK[p].SetDiffuse( RageColor(1,1,1,0) );
		m_framePages.AddChild( &m_sprOK[p] );
	}

	this->AddChild( &m_framePages );
	
	m_soundChange.Load( THEME->GetPathTo("Sounds", "ScreenSelectDifficulty change") );
	m_soundSelect.Load( THEME->GetPathTo("Sounds", "Common start") );
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
		if( GAMESTATE->IsPlayerEnabled(p) )
		{
			MenuRight( (PlayerNumber)p );
			MenuLeft( (PlayerNumber)p );
		}
}

void ScreenSelectDifficulty::MenuLeft( PlayerNumber pn )
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


void ScreenSelectDifficulty::MenuRight( PlayerNumber pn )
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

void ScreenSelectDifficulty::ChangePage( Page newPage )
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

void ScreenSelectDifficulty::ChangeWithinPage( PlayerNumber pn, int iNewChoice, bool bChangingPages )
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

void ScreenSelectDifficulty::MenuStart( PlayerNumber pn )
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
		SOUNDMAN->PlayOnceFromDir( ANNOUNCER->GetPathTo(ssprintf("ScreenSelectDifficulty comment %s",mc.name)) );
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
		if( GAMESTATE->IsPlayerEnabled((PlayerNumber)p)  &&  m_bChosen[p] == false )
			return;
	}
	this->PostScreenMessage( SM_BeginFadingOut, SLEEP_AFTER_CHOICE_SECONDS );	// tell our owner it's time to move on
}

// Err, this breaks back ...
//void ScreenSelectDifficulty::MenuBack( PlayerNumber pn )
//{
//}

void ScreenSelectDifficulty::TweenOnScreen() 
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
//			m_sprInfo[p][c].FadeOn( fPause, "left far accelerate", 0.4f );
			m_sprInfo[page][c].Command( INFO_ON_COMMAND(page,c) );

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

void ScreenSelectDifficulty::TweenOffScreen()
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
		//m_sprInfo[page][c].FadeOff( fPause+0.3f, "left far accelerate", 0.4f );
		m_sprInfo[page][c].Command( INFO_OFF_COMMAND(page,c) );
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
