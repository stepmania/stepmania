#include "global.h"
/*
-----------------------------------------------------------------------------
 Class: ScreenSelectDifficultyEX

 Desc: See header.

 Copyright (c) 2001-2003 by the person(s) listed below.  All rights reserved.
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


#define	ANIMATE_MODE_SWITCH						THEME->GetMetricB("ScreenSelectDifficultyEX","AnimateModeSwitch")
#define	ANIMATE_MODE_SWITCH_LOCK_TIME			THEME->GetMetricF("ScreenSelectDifficultyEX","AnimateModeSwitchLockTime")
#define ANIMATE_MODE_SWITCH_INFO_OFF_COMMAND	THEME->GetMetric("ScreenSelectDifficultyEX","AnimateModeSwitchInfoOFFCommand")
#define ANIMATE_MODE_SWITCH_INFO_ON_COMMAND		THEME->GetMetric("ScreenSelectDifficultyEX","AnimateModeSwitchInfoONCommand")
#define ANIMATE_MODE_SWITCH_PICTURE_OFF_COMMAND	THEME->GetMetric("ScreenSelectDifficultyEX","AnimateModeSwitchPictureOFFCommand")
#define ANIMATE_MODE_SWITCH_PICTURE_ON_COMMAND	THEME->GetMetric("ScreenSelectDifficultyEX","AnimateModeSwitchPictureONCommand")
#define CURSOR_OFFSET_X_FROM_PICTURE( p )		THEME->GetMetricF("ScreenSelectDifficultyEX",ssprintf("CursorP%dOffsetXFromPicture",p+1))
#define CURSOR_OFFSET_Y_FROM_PICTURE( p )		THEME->GetMetricF("ScreenSelectDifficultyEX",ssprintf("CursorP%dOffsetYFromPicture",p+1))
#define DIFFICULTYICON_OFF_COMMAND( p )			THEME->GetMetric("ScreenSelectDifficultyEX",ssprintf("DifficultyIcon%dOFFCommand",p+1))
#define DIFFICULTYICON_ON_COMMAND( p )			THEME->GetMetric("ScreenSelectDifficultyEX",ssprintf("DifficultyIcon%dONCommand",p+1))
#define DIFFICULTYICON_X( p )					THEME->GetMetricF("ScreenSelectDifficultyEX",ssprintf("DifficultyIcon%dX",p+1))
#define DIFFICULTYICON_Y( p )					THEME->GetMetricF("ScreenSelectDifficultyEX",ssprintf("DifficultyIcon%dY",p+1))
#define DIFFICULTYTEXT_COLOR( p )				THEME->GetMetricC("ScreenSelectDifficultyEX",ssprintf("DifficultyText%dCOLOR",p+1))
#define DIFFICULTYTEXT_OFF_COMMAND( p )			THEME->GetMetric("ScreenSelectDifficultyEX",ssprintf("DifficultyText%dOFFCommand",p+1))
#define DIFFICULTYTEXT_ON_COMMAND( p )			THEME->GetMetric("ScreenSelectDifficultyEX",ssprintf("DifficultyText%dONCommand",p+1))
#define DISABLED_COLOR							THEME->GetMetricC("ScreenSelectDifficultyEX","DisabledColor")
#define ICONBAR_OFF_COMMAND						THEME->GetMetric("ScreenSelectDifficultyEX","IconBarOFFCommand")
#define ICONBAR_ON_COMMAND						THEME->GetMetric("ScreenSelectDifficultyEX","IconBarONCommand")
#define INFO_OFF_COMMAND( p )					THEME->GetMetric("ScreenSelectDifficultyEX",ssprintf("InfoP%dOFFCommand",p+1))
#define INFO_ON_COMMAND( p )					THEME->GetMetric("ScreenSelectDifficultyEX",ssprintf("InfoP%dONCommand",p+1))
#define INFO_X( p )								THEME->GetMetricF("ScreenSelectDifficultyEX",ssprintf("InfoXP%d",p+1))
#define INFO_Y( p )								THEME->GetMetricF("ScreenSelectDifficultyEX",ssprintf("InfoYP%d",p+1))
#define LOCK_INPUT_SECONDS						THEME->GetMetricF("ScreenSelectDifficultyEX","LockInputSeconds")
#define PICTURE_OFF_COMMAND( p )				THEME->GetMetric("ScreenSelectDifficultyEX",ssprintf("PictureP%dOFFCommand",p+1))
#define PICTURE_ON_COMMAND( p )					THEME->GetMetric("ScreenSelectDifficultyEX",ssprintf("PictureP%dONCommand",p+1))
#define PICTURE_X( p )							THEME->GetMetricF("ScreenSelectDifficultyEX",ssprintf("PictureXP%d",p+1))
#define PICTURE_Y( p )							THEME->GetMetricF("ScreenSelectDifficultyEX",ssprintf("PictureYP%d",p+1))
#define SLEEP_AFTER_CHOICE_SECONDS				THEME->GetMetricF("ScreenSelectDifficultyEX","SleepAfterChoiceSeconds")
#define SLEEP_AFTER_TWEEN_OFF_SECONDS			THEME->GetMetricF("ScreenSelectDifficultyEX","SleepAfterTweenOffSeconds")


ScreenSelectDifficultyEX::ScreenSelectDifficultyEX() : ScreenSelect( "ScreenSelectDifficultyEX" )
{
	m_CurrentPage = PAGE_1;
	int p;
	for( p=0; p<NUM_PLAYERS; p++ )
	{
		m_iChoice[p] = 0;
		m_bChosen[p] = false;
	}
	
	m_NumModes = 0;
	for( unsigned c=0; c<m_aModeChoices.size(); c++ )
	{
		m_ModeChoices.push_back( m_aModeChoices[c] );
		m_NumModes++;
	}
	
	m_sprIconBar.Load( THEME->GetPathToG("ScreenSelectDifficultyEX icon bar.png") );
	this->AddChild( &m_sprIconBar );

	for( unsigned k=0; k < m_NumModes || k <= 7 ; k++ )
	{
		CString MOO = m_aModeChoices[k].name;
		if( IsValidModeName(MOO) )
		{
			// We cannot show more than 8 icons at a time, BTW
			m_sprDifficultyIcon[k].Load( THEME->GetPathToG("ScreenSelectDifficultyEX icons 1x10") );
			m_sprDifficultyIcon[k].SetX( DIFFICULTYICON_X(k) );
			m_sprDifficultyIcon[k].SetY( DIFFICULTYICON_Y(k) );
			m_sprDifficultyIcon[k].SetZoom( 1 );
			this->AddChild( &m_sprDifficultyIcon[k] );
			
			m_textDifficultyText[k].SetHorizAlign( Actor::align_left );		
			m_textDifficultyText[k].LoadFromFont( THEME->GetPathToF("Common normal") );
			m_textDifficultyText[k].SetX( DIFFICULTYICON_X(k) + 20 );
			m_textDifficultyText[k].SetY( DIFFICULTYICON_Y(k) );
			m_textDifficultyText[k].SetZoom( .5 );
			m_textDifficultyText[k].SetShadowLength( 1 );	//No shadow, jus outline
			m_textDifficultyText[k].SetDiffuse( DIFFICULTYTEXT_COLOR(k) );
			this->AddChild( &m_textDifficultyText[k] );
		}
	}
	SetDifficultyIconText( false );	// Make sure we show the names!

	for( p=0; p<NUM_PLAYERS; p++ )
	{
		if( GAMESTATE->IsPlayerEnabled(p) ) // Only show their cursor, if they're enabled
		{
			m_sprHighlight[p].SetHorizAlign( Actor::align_left );
			m_sprHighlight[p].Load( (PlayerNumber)p, false );
			this->AddChild( &m_sprHighlight[p] );
			this->MoveToHead( &m_sprHighlight[p] );
		}

		CLAMP( m_iChoice[p], 0, (int)m_ModeChoices.size()-1 );
		m_bChosen[p] = false;

		if( !GAMESTATE->IsPlayerEnabled((PlayerNumber)p) ) { continue; }
		float fCursorX = GetCursorX( (PlayerNumber)p );
		float fCursorY = GetCursorY( (PlayerNumber)p );
		
		CString sInfoFile = ssprintf( "ScreenSelectDifficultyEX info %s", m_ModeChoices[0].name );
		CString sPictureFile = ssprintf( "ScreenSelectDifficultyEX picture %s", m_ModeChoices[0].name );
		
		m_sprPicture[p].Load( THEME->GetPathToG(sPictureFile) );
		m_sprPicture[p].SetXY( PICTURE_X(p), PICTURE_Y(p) );
		m_framePages.AddChild( &m_sprPicture[p] );
		
		m_sprInfo[p].Load( THEME->GetPathToG(sInfoFile) );
		m_sprInfo[p].SetXY( INFO_X(p), INFO_Y(p) );
		m_framePages.AddChild( &m_sprInfo[p] );	

		m_sprCursor[p].Load( THEME->GetPathToG( "ScreenSelectDifficultyEX cursor 2x1") );
		m_sprCursor[p].SetZoom( 0 );
		m_sprCursor[p].StopAnimating();
		m_sprCursor[p].SetState( p );
		m_sprCursor[p].SetXY( fCursorX, fCursorY );
		m_framePages.AddChild( &m_sprCursor[p] );

		m_sprOK[p].Load( THEME->GetPathToG( "ScreenSelectDifficultyEX ok 2x1") );
		m_sprOK[p].SetState( p );
		m_sprOK[p].StopAnimating();
		m_sprOK[p].SetDiffuse( RageColor(1,1,1,1) );
		m_sprOK[p].SetXY( fCursorX, fCursorY );
		m_framePages.AddChild( &m_sprOK[p] );
	}

	this->AddChild( &m_framePages );
	m_soundChange.Load( THEME->GetPathToS( "ScreenSelectDifficulty change") );
	m_soundSelect.Load( THEME->GetPathToS( "Common start") );
	m_soundDifficult.Load( ANNOUNCER->GetPathTo("select difficulty challenge") );
	m_fLockInputTime = LOCK_INPUT_SECONDS;
	TweenOnScreen();

	SetAllPlayersSelection( 0, false );
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

bool ScreenSelectDifficultyEX::IsACourse( int mIndex )
{
	CString	MODECHOICENAME = m_ModeChoices[mIndex].name;
	if( MODECHOICENAME.Left(7) != "arcade-" ) { return true; }
	else { return false; }
}

void ScreenSelectDifficultyEX::SetAllPlayersSelection( int iChoice, bool bSwitchingModes )
{
	for( int p=0; p<NUM_PLAYERS; p++ )
	{
		m_iChoice[p] = iChoice;
		if( bSwitchingModes && ANIMATE_MODE_SWITCH )
		{
			// Only do this if we are going from a course~normal or vice-vera
			m_fLockInputTime = ANIMATE_MODE_SWITCH_LOCK_TIME;

			m_sprPicture[p].Command( ANIMATE_MODE_SWITCH_PICTURE_OFF_COMMAND );
			m_sprInfo[p].Command( ANIMATE_MODE_SWITCH_INFO_OFF_COMMAND );

			/* It would be logical to seperate these, as I don't want the pic
				to change until the OFF command has completed. Unfortunatly, 
				until I find a way to correctly do that, sometimes the pic
				changes before the OFF is even done, when OFF is set to only
				take 0.0secs! Perhaps a loop here until GetTweenTimeLeft() = 0? */

			m_sprPicture[p].Command( ANIMATE_MODE_SWITCH_PICTURE_ON_COMMAND );
			m_sprInfo[p].Command( ANIMATE_MODE_SWITCH_INFO_ON_COMMAND );			
		}
		
		CString sInfoFile = ssprintf( "ScreenSelectDifficultyEX info %s", m_ModeChoices[m_iChoice[p]].name );
		CString sPictureFile = ssprintf( "ScreenSelectDifficultyEX picture %s", m_ModeChoices[m_iChoice[p]].name );
		m_sprInfo[p].Load( THEME->GetPathToG(sInfoFile) );
		m_sprPicture[p].Load( THEME->GetPathToG(sPictureFile) );
		
		int iDiffIndex = GetSelectionIndex((PlayerNumber)p);
		m_sprHighlight[p].SetBarWidth( 20 + m_sprDifficultyIcon[iDiffIndex].GetZoomedWidth() + (m_textDifficultyText[iDiffIndex].GetWidestLineWidthInSourcePixels() /2) );
		m_sprHighlight[p].SetXY( DIFFICULTYICON_X(m_iChoice[p]) + 35, DIFFICULTYICON_Y(m_iChoice[p]) );
	}
	
	m_soundChange.Play();
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
			if( m_bChosen[pl] && IsACourse(iNewChoice) ) { return; }
		}
	
		// Going from a course/mode to a normal difficulty, or vice-versa. 
		if( IsACourse(iNewChoice) && !IsACourse(m_iChoice[p]) || !IsACourse(iNewChoice) && IsACourse(m_iChoice[p]) )
		{ 
			SetAllPlayersSelection( iNewChoice, true );
			return; 
		}

		// Going between courses/modes
		if( IsACourse(iNewChoice) && IsACourse(m_iChoice[p]) )
		{
			SetAllPlayersSelection( iNewChoice, false );
			return;
		}

		m_iChoice[p] = iNewChoice;
		CString sInfoFile = ssprintf( "ScreenSelectDifficultyEX info %s", m_ModeChoices[m_iChoice[p]].name );
		CString sPictureFile = ssprintf( "ScreenSelectDifficultyEX picture %s", m_ModeChoices[m_iChoice[p]].name );
		m_sprInfo[p].Load( THEME->GetPathToG(sInfoFile) );
		m_sprPicture[p].Load( THEME->GetPathToG(sPictureFile) );
		
		int iDiffIndex = GetSelectionIndex((PlayerNumber)p);
		m_sprHighlight[p].SetBarWidth( 20 + m_sprDifficultyIcon[iDiffIndex].GetZoomedWidth() + (m_textDifficultyText[iDiffIndex].GetWidestLineWidthInSourcePixels() /2) );
		m_sprHighlight[p].SetXY( DIFFICULTYICON_X(m_iChoice[p]) + 30, DIFFICULTYICON_Y(m_iChoice[p]) );
	}
	
	m_soundChange.Play();
}

bool ScreenSelectDifficultyEX::IsValidModeName( CString ModeName )
{
	if( ModeName == "arcade-beginner" || ModeName == "beginner" || ModeName == "arcade-easy" || ModeName == "easy" || ModeName == "arcade-medium" || ModeName == "medium" || ModeName == "arcade-hard" || ModeName == "hard" || ModeName == "nonstop" || ModeName == "oni" || ModeName == "endless" || ModeName == "rave" || ModeName == "battle" )
	{
		return true;
	}
	else { return false; };
}

int ScreenSelectDifficultyEX::GetIconIndex( CString DiffName )
{
	if( DiffName == "arcade-beginner" || DiffName == "beginner" ) { return 0; }
	else if( DiffName == "arcade-easy" || DiffName == "easy" ) { return 1; }
	else if( DiffName == "arcade-medium" || DiffName == "medium" ) { return 2; }
	else if( DiffName == "arcade-hard" || DiffName == "hard" ) { return 3; }
	else if( DiffName == "nonstop" ) { return 4; }
	else if( DiffName == "oni" || DiffName == "challenge" ) { return 5; }
	else if( DiffName == "endless" ) { return 6; }
	else if( DiffName == "rave" || DiffName == "battle" ) { return 7; }
	else { return 10; };
}

void ScreenSelectDifficultyEX::SetDifficultyIconText( bool bDisplayCourseItems )
{
	for( unsigned k=0; k < m_aModeChoices.size(); k++ )
	{
		CString	MMNAME = m_ModeChoices[k].name;
		MMNAME.Replace( "arcade-", "" );
		m_sprDifficultyIcon[k].SetState( GetIconIndex(MMNAME.c_str()) );
		m_textDifficultyText[k].SetDiffuse( RageColor(1,1,1,1) );

		/* Grammatically correct names. There has to be a better way to
			do this.. Anyone? -- Miryo */
		MMNAME.Replace( "beginner", "Beginner");
		MMNAME.Replace( "easy", "Light" );
		MMNAME.Replace( "medium", "Standard" );
		MMNAME.Replace( "hard", "Heavy" );
		MMNAME.Replace( "nonstop", "Nonstop");
		MMNAME.Replace( "oni", "Oni");
		MMNAME.Replace( "endless", "Endless");
		MMNAME.Replace( "rave", "Rave");

		m_textDifficultyText[k].SetText( MMNAME.c_str() );
		m_textDifficultyText[k].SetDiffuse( DIFFICULTYTEXT_COLOR(k) );
	}
}

void ScreenSelectDifficultyEX::MenuStart( PlayerNumber pn )
{
	if( m_fLockInputTime > 0 ) { return; }
	if( m_bChosen[pn] == true ) { return; }
	m_bChosen[pn] = true;
	const ModeChoice& mc = m_ModeChoices[m_iChoice[pn]];
	bool AnotherPlayerSelected = false;

	int p;
	for( p=0; p<NUM_PLAYERS; p++ )
	{
		if(p != pn && m_bChosen[p]) { AnotherPlayerSelected = true; }
	}

	if( !IsACourse( m_iChoice[pn] ) || !AnotherPlayerSelected)
	{
		// Disable the courses
		for( unsigned e=0; e < m_NumModes || e <= 7; e++ )
		{
			if( IsACourse(e) )
			{
				m_textDifficultyText[e].SetDiffuse( RageColor(.5,.5,.5,1) );
				m_sprDifficultyIcon[e].SetDiffuse( RageColor(.5,.5,.5,1) );
			}
		}
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

	for( p=0; p<NUM_PLAYERS; p++ )
	{
		if( !GAMESTATE->IsPlayerEnabled((PlayerNumber)p) ) { continue; }

		float fCursorX = GetCursorX( (PlayerNumber)p );
		float fCursorY = GetCursorY( (PlayerNumber)p );

		m_sprInfo[p].Command( INFO_ON_COMMAND(p) );
		m_sprPicture[p].Command( PICTURE_ON_COMMAND(p) );
		
		m_sprHighlight[p].SetXY( DIFFICULTYICON_X(1) + 30, DIFFICULTYICON_Y(1) );
		m_sprHighlight[p].Command( "diffusealpha,0;linear,0.3;diffusealpha,1" );
	}

	m_sprIconBar.Command( ICONBAR_ON_COMMAND );
	for( unsigned di=0; di < m_aModeChoices.size(); di++ )
	{
		m_sprDifficultyIcon[di].Command( DIFFICULTYICON_ON_COMMAND(di) );
		m_textDifficultyText[di].Command( DIFFICULTYTEXT_ON_COMMAND(di) );
	}
}

void ScreenSelectDifficultyEX::TweenOffScreen()
{	
	for( int p=0; p<NUM_PLAYERS; p++ )
	{
		if( !GAMESTATE->IsPlayerEnabled((PlayerNumber)p) ) { continue; }

		m_sprInfo[p].Command( INFO_OFF_COMMAND(p) );
		m_sprPicture[p].Command( PICTURE_OFF_COMMAND(p) );
		m_sprHighlight[p].SetXY( -600,-600 );
		m_sprOK[p].Command( "linear,0.3;diffusealpha,0" );
	}
	
	m_sprIconBar.Command( ICONBAR_OFF_COMMAND );
	for( unsigned di=0; di < m_aModeChoices.size(); di++ )
	{
		m_sprDifficultyIcon[di].Command( DIFFICULTYICON_OFF_COMMAND(di) );
		m_textDifficultyText[di].Command( DIFFICULTYTEXT_OFF_COMMAND(di) );
	}
}


float ScreenSelectDifficultyEX::GetCursorX( PlayerNumber pn )
{
	return m_sprPicture[pn].GetX();
}

float ScreenSelectDifficultyEX::GetCursorY( PlayerNumber pn )
{
	return m_sprPicture[pn].GetY();
}
