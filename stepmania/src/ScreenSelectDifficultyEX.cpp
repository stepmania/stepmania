#include "global.h"
/*
-----------------------------------------------------------------------------
 Class: ScreenSelectDifficultyEX

 Desc: See header.

 Copyright (c) 2001-2003 by the person(s) listed below.  All rights reserved.
	Chris Danford
	Kevin Slaughter
-----------------------------------------------------------------------------
*/

#include "AnnouncerManager.h"
#include "GameManager.h"
#include "GameState.h"
#include "ModeChoice.h"
#include "PrefsManager.h"
#include "RageLog.h"
#include "ScreenManager.h"
#include "ScreenSelectDifficultyEX.h"
#include "ThemeManager.h"


#define CURSOR_OFFSET_X_FROM_PICTURE( p )	THEME->GetMetricF("ScreenSelectDifficultyEX",ssprintf("CursorP%dOffsetXFromPicture",p+1))
#define CURSOR_OFFSET_Y_FROM_PICTURE( p )	THEME->GetMetricF("ScreenSelectDifficultyEX",ssprintf("CursorP%dOffsetYFromPicture",p+1))
#define DISABLED_COLOR						THEME->GetMetricC("ScreenSelectDifficultyEX","DisabledColor")
#define INFO_X( p )							THEME->GetMetricF("ScreenSelectDifficultyEX",ssprintf("InfoXP%d",p+1))
#define INFO_Y( p )							THEME->GetMetricF("ScreenSelectDifficultyEX",ssprintf("InfoYP%d",p+1))
#define LOCK_INPUT_SECONDS					THEME->GetMetricF("ScreenSelectDifficultyEX","LockInputSeconds")
#define PICTURE_X( p )						THEME->GetMetricF("ScreenSelectDifficultyEX",ssprintf("PictureXP%d",p+1))
#define PICTURE_Y( p )						THEME->GetMetricF("ScreenSelectDifficultyEX",ssprintf("PictureYP%d",p+1))
#define SLEEP_AFTER_CHOICE_SECONDS			THEME->GetMetricF("ScreenSelectDifficultyEX","SleepAfterChoiceSeconds")
#define SLEEP_AFTER_TWEEN_OFF_SECONDS		THEME->GetMetricF("ScreenSelectDifficultyEX","SleepAfterTweenOffSeconds")



ScreenSelectDifficultyEX::ScreenSelectDifficultyEX() : ScreenSelect( "ScreenSelectDifficultyEX" )
{
	m_CurrentPage = PAGE_1;
	for( int p=0; p<NUM_PLAYERS; p++ )
	{
		m_iChoice[p] = 0;
		m_bChosen[p] = false;
	}

	for( unsigned c=0; c<m_aModeChoices.size(); c++ )
	{
		m_ModeChoices.push_back( m_aModeChoices[c] );
	}
	
	//m_sprMore.Load( THEME->GetPathTo("Graphics", "ScreenSelectDifficultyEX more page1") );
	//m_framePages.AddChild( &m_sprMore );

	//m_sprExplanation.Load( THEME->GetPathTo("Graphics", "ScreenSelectDifficultyEX explanation") );
	//m_sprExplanation.StopAnimating();
	//m_sprExplanation.SetState( 1 );
	//m_framePages.AddChild( &m_sprExplanation );

	for( p=0; p<NUM_PLAYERS; p++ )
	{
		CLAMP( m_iChoice[p], 0, (int)m_ModeChoices.size()-1 );
		m_bChosen[p] = false;

		if( !GAMESTATE->IsPlayerEnabled((PlayerNumber)p) ) { continue; }
		float fCursorX = GetCursorX( (PlayerNumber)p );
		float fCursorY = GetCursorY( (PlayerNumber)p );
		
		CString sInfoFile = ssprintf( "ScreenSelectDifficultyEX info %s", m_ModeChoices[0].name );
		CString sPictureFile = ssprintf( "ScreenSelectDifficultyEX picture %s", m_ModeChoices[0].name );
		
		m_sprPicture[p].Load( THEME->GetPathTo("Graphics",sPictureFile) );
		m_sprPicture[p].SetX( PICTURE_X(p) );
		m_sprPicture[p].SetY( PICTURE_Y(p) );
		m_framePages.AddChild( &m_sprPicture[p] );
		
		m_sprInfo[p].Load( THEME->GetPathTo("Graphics",sInfoFile) );
		m_sprInfo[p].SetX( INFO_X(p) );
		m_sprInfo[p].SetY( INFO_Y(p) );
		m_framePages.AddChild( &m_sprInfo[p] );	

		m_sprCursor[p].Load( THEME->GetPathTo("Graphics", "ScreenSelectDifficultyEX cursor 2x1") );
		m_sprCursor[p].SetZoom( 0 );
		m_sprCursor[p].StopAnimating();
		m_sprCursor[p].SetState( p );
		m_sprCursor[p].SetX( fCursorX );
		m_sprCursor[p].SetY( fCursorY );
		m_framePages.AddChild( &m_sprCursor[p] );

		m_sprOK[p].Load( THEME->GetPathTo("Graphics", "ScreenSelectDifficultyEX ok 2x1") );
		m_sprOK[p].SetState( p );
		m_sprOK[p].StopAnimating();
		m_sprOK[p].SetDiffuse( RageColor(1,1,1,1) );
		m_sprOK[p].SetX( fCursorX );
		m_sprOK[p].SetY( fCursorY );
		m_framePages.AddChild( &m_sprOK[p] );
	}

	this->AddChild( &m_framePages );
	
	m_soundChange.Load( THEME->GetPathTo("Sounds", "ScreenSelectDifficulty change") );
	m_soundSelect.Load( THEME->GetPathTo("Sounds", "Common start") );
	m_soundDifficult.Load( ANNOUNCER->GetPathTo("select difficulty challenge") );

	m_fLockInputTime = LOCK_INPUT_SECONDS;

	TweenOnScreen();
}

void ScreenSelectDifficultyEX::Update( float fDelta )
{
	ScreenSelect::Update( fDelta );
	m_fLockInputTime = max( 0, m_fLockInputTime-fDelta );
}

void ScreenSelectDifficultyEX::HandleScreenMessage( const ScreenMessage SM )
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

int ScreenSelectDifficultyEX::GetSelectionIndex( PlayerNumber pn )
{
	int index = 0;
	for( int page=0; page<m_CurrentPage; page++ )
		index += m_ModeChoices.size();
	index += m_iChoice[pn];
	return index;
}

void ScreenSelectDifficultyEX::UpdateSelectableChoices()
{
	for( int p=0; p<NUM_PLAYERS; p++ )
	{
		/* XXX: If a player joins during the tween-in, this diffuse change
		 * will be undone by the tween.  Hmm. */
		for( unsigned i=0; i<m_ModeChoices.size(); i++ )
		{
			/* If the icon is text, use a dimmer diffuse, or we won't be
			 * able to see the glow. */
			if( GAMESTATE->IsPlayable(m_ModeChoices[i]) )
			{
				m_sprInfo[p].SetDiffuse( RageColor(1,1,1,1) );
				m_sprPicture[p].SetDiffuse( RageColor(1,1,1,1) );
			}
			else
			{
				m_sprInfo[p].SetDiffuse( DISABLED_COLOR );
				m_sprPicture[p].SetDiffuse( DISABLED_COLOR );
			}
		}
	}

	for( int pa=0; pa<NUM_PLAYERS; pa++ )
		if( GAMESTATE->IsPlayerEnabled(pa) )
		{
			MenuRight( (PlayerNumber)pa );
			MenuLeft( (PlayerNumber)pa );
		}
}

void ScreenSelectDifficultyEX::MenuLeft( PlayerNumber pn )
{
	if( m_fLockInputTime > 0 ) { return; }
	if( m_bChosen[pn] ) { return; }
	if( m_iChoice[pn] == 0 ) { return; }
	else { Change( pn, m_iChoice[pn]-1 ); }
}


void ScreenSelectDifficultyEX::MenuRight( PlayerNumber pn )
{
	if( m_fLockInputTime > 0 ) { return; }
	if( m_bChosen[pn] ) { return; }
	if( m_iChoice[pn] < (int)m_ModeChoices.size()-1 ) { Change( pn, m_iChoice[pn]+1 ); }
}

bool ScreenSelectDifficultyEX::PlayerIsOnCourse( PlayerNumber pn )
{
	if( m_iChoice[pn] > 3 ) { return true; }  
	else { return false; }
}

bool ScreenSelectDifficultyEX::IsACourse( int mIndex )
{
	if( mIndex > 3 ) { return true; }
	else { return false; }
}

void ScreenSelectDifficultyEX::SetAllPlayersSelection( int iChoice, bool bSwitchingModes )
{
	for( int p=0; p<NUM_PLAYERS; p++ )
	{
		m_iChoice[p] = iChoice;
		if( bSwitchingModes )
		{
			// Only do this if we are going from a course~normal or vice-vera
			m_fLockInputTime = 0.5f;
			m_sprPicture[p].StopTweening();
			m_sprInfo[p].StopTweening();
			m_sprPicture[p].FadeOn( 0.0, "foldy bounce", 0.3f );
			m_sprInfo[p].FadeOn( 0.0, "foldy bounce", 0.3f );
		}
		
		CString sInfoFile = ssprintf( "ScreenSelectDifficultyEX info %s", m_ModeChoices[m_iChoice[p]].name );
		CString sPictureFile = ssprintf( "ScreenSelectDifficultyEX picture %s", m_ModeChoices[m_iChoice[p]].name );
		m_sprInfo[p].Load( THEME->GetPathTo( "Graphics", sInfoFile ) );
		m_sprPicture[p].Load( THEME->GetPathTo( "Graphics", sPictureFile ) );
	}
	
	m_soundChange.Play();
}

bool ScreenSelectDifficultyEX::AllPlayersAreOnCourse()
{
	bool OC = false;
	for( int p=0; p<NUM_PLAYERS; p++ )
	{
		if( IsACourse(m_iChoice[p]) ) { OC = true; }
	}

	return OC;
}

void ScreenSelectDifficultyEX::Change( PlayerNumber pn, int iNewChoice )
{
	for( int p=0; p<NUM_PLAYERS; p++ )
	{
		if( !GAMESTATE->IsPlayerEnabled(p) ) { continue; }
		if( p!=pn && m_CurrentPage==PAGE_1 ) { continue; }
		
		// If one player has already chosen, do not allow us to switch between
		// Course/Mode & Difficulty
		for( int pl=0; pl<NUM_PLAYERS; pl++ )
		{
			if( m_bChosen[pl] && IsACourse(iNewChoice) )
			{
					return;
			}
		}
	
		// Going from a non-mode/course to a normal difficulty. 
		if( IsACourse(iNewChoice) && !IsACourse(m_iChoice[p]) || !IsACourse(iNewChoice) && IsACourse(m_iChoice[p]) )
		{ 
			SetAllPlayersSelection( iNewChoice, true );
			return; 
		}

		// Going from course~course/mode~mode
		if( IsACourse(iNewChoice) && IsACourse(m_iChoice[p]) )
		{
			SetAllPlayersSelection( iNewChoice, false );
			return;
		}

		m_iChoice[p] = iNewChoice;
		CString sInfoFile = ssprintf( "ScreenSelectDifficultyEX info %s", m_ModeChoices[m_iChoice[p]].name );
		CString sPictureFile = ssprintf( "ScreenSelectDifficultyEX picture %s", m_ModeChoices[m_iChoice[p]].name );
		m_sprInfo[p].Load( THEME->GetPathTo( "Graphics", sInfoFile ) );
		m_sprPicture[p].Load( THEME->GetPathTo( "Graphics", sPictureFile ) );
	}
	
	m_soundChange.Play();
}

void ScreenSelectDifficultyEX::MenuStart( PlayerNumber pn )
{
	if( m_fLockInputTime > 0 ) { return; }
	if( m_bChosen[pn] == true ) { return; }
	m_bChosen[pn] = true;
	const ModeChoice& mc = m_ModeChoices[m_iChoice[pn]];
	bool AnotherPlayerSelected = false;

	for( int p=0; p<NUM_PLAYERS; p++ )
	{
		if(p != pn && m_bChosen[p]) { AnotherPlayerSelected = true; }
	}

	if( !IsACourse( m_iChoice[pn] ) || !AnotherPlayerSelected)
	{
		SOUNDMAN->PlayOnceFromDir( ANNOUNCER->GetPathTo(ssprintf("ScreenSelectDifficulty comment %s",mc.name)) );
		m_soundSelect.Play();
	}

	if( IsACourse( m_iChoice[pn] ) )
	{	
		// If this is course/mode, choose this for all the other players too
		for( p=0; p<NUM_PLAYERS; p++ )
		{
			if( m_bChosen[p] ) { continue; }
			MenuStart( (PlayerNumber)p );
		}
	}

	// Show cursor only when a selection is made
	m_sprOK[pn].SetZoom( 1 );
	m_sprCursor[pn].SetZoom( 1 );
	m_sprOK[pn].SetX( GetCursorX(pn) );
	m_sprOK[pn].SetY( GetCursorY(pn) );
	
	m_sprCursor[pn].FadeOn( 0.0, "foldy bounce", 0.3f );
	m_sprOK[pn].FadeOn( 0.0, "foldy bounce", 0.3f );


	// check to see if everyone has chosen
	for( p=0; p<NUM_PLAYERS; p++ )
	{
		if( GAMESTATE->IsPlayerEnabled((PlayerNumber)p)  &&  m_bChosen[p] == false )
		{ 
			return;
		}
	}
	this->PostScreenMessage( SM_BeginFadingOut, SLEEP_AFTER_CHOICE_SECONDS );	// tell our owner it's time to move on
}

void ScreenSelectDifficultyEX::TweenOnScreen() 
{
	unsigned p;

	for( int page=0; page<NUM_PAGES; page++ )
	{
		//m_sprExplanation.Command( "linear,0.5;bounceend,0.5" );
		//m_sprMore.Command( "linear,0.5;bounceend,0.5" );

	}

	for( p=0; p<NUM_PLAYERS; p++ )
	{
		if( !GAMESTATE->IsPlayerEnabled((PlayerNumber)p) ) { continue; }

		float fCursorX = GetCursorX( (PlayerNumber)p );
		float fCursorY = GetCursorY( (PlayerNumber)p );

		m_sprCursor[p].SetXY( fCursorX, fCursorY );
	}
}

void ScreenSelectDifficultyEX::TweenOffScreen()
{	
	const int page = m_CurrentPage;

	//m_sprExplanation.Command( "linear,0.5;bounceend,0.5" );
	//m_sprMore.Command( "linear,0.5;bounceend,0.5" );

	for( int p=0; p<NUM_PLAYERS; p++ )
	{
		if( !GAMESTATE->IsPlayerEnabled((PlayerNumber)p) ) { continue; }
		
		m_sprPicture[p].FadeOff( 0.0, "fade", 0.5f );
		m_sprInfo[p].FadeOff( 0.0, "fade", 0.5f );

		m_sprCursor[p].FadeOff( 0.0, "fade", 0.5f );
		m_sprOK[p].FadeOff( 0.0, "fade", 0.5f );
	}
}


float ScreenSelectDifficultyEX::GetCursorX( PlayerNumber pn )
{
	return m_sprPicture[pn].GetX();// + CURSOR_OFFSET_X_FROM_PICTURE(pn);
}

float ScreenSelectDifficultyEX::GetCursorY( PlayerNumber pn )
{
	return m_sprPicture[pn].GetY();// + CURSOR_OFFSET_Y_FROM_PICTURE(pn);
}
