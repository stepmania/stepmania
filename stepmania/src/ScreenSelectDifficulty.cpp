#include "global.h"
/*
-----------------------------------------------------------------------------
 Class: ScreenSelectDifficulty

 Desc: Testing the Screen class.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/

#include "ScreenSelectDifficulty.h"
#include "ScreenManager.h"
#include "PrefsManager.h"
#include "GameManager.h"
#include "RageLog.h"
#include "GameConstantsAndTypes.h"
#include "AnnouncerManager.h"
#include "GameState.h"
#include "RageSoundManager.h"
#include "ThemeManager.h"


const float LOCK_INPUT_TIME = 0.30f;	// lock input while waiting for tweening to complete

#define MORE_X( page )			THEME->GetMetricF("ScreenSelectDifficulty",ssprintf("MorePage%dX",page+1))
#define MORE_Y( page )			THEME->GetMetricF("ScreenSelectDifficulty",ssprintf("MorePage%dY",page+1))
#define EXPLANATION_X( page )	THEME->GetMetricF("ScreenSelectDifficulty",ssprintf("ExplanationPage%dX",page+1))
#define EXPLANATION_Y( page )	THEME->GetMetricF("ScreenSelectDifficulty",ssprintf("ExplanationPage%dY",page+1))
#define CHOICES_ON_PAGE( page )	THEME->GetMetric ("ScreenSelectDifficulty",ssprintf("ChoicesOnPage%d",page+1))
#define CHOICE_X( page, choice ) THEME->GetMetricF("ScreenSelectDifficulty",ssprintf("Page%dChoice%dX",page+1,choice+1))
#define CHOICE_Y( page, choice ) THEME->GetMetricF("ScreenSelectDifficulty",ssprintf("Page%dChoice%dY",page+1,choice+1))
#define CURSOR_OFFSET_X( p )	THEME->GetMetricF("ScreenSelectDifficulty",ssprintf("CursorOffsetP%dX",p+1))
#define CURSOR_OFFSET_Y( i )	THEME->GetMetricF("ScreenSelectDifficulty",ssprintf("CursorOffsetP%dY",i+1))
#define CURSOR_SHADOW_LENGTH_X	THEME->GetMetricF("ScreenSelectDifficulty","CursorShadowLengthX")
#define CURSOR_SHADOW_LENGTH_Y	THEME->GetMetricF("ScreenSelectDifficulty","CursorShadowLengthY")
#define INITIAL_CHOICE			THEME->GetMetricI("ScreenSelectDifficulty","InitialChoice")
#define HELP_TEXT				THEME->GetMetric("ScreenSelectDifficulty","HelpText")
#define TIMER_SECONDS			THEME->GetMetricI("ScreenSelectDifficulty","TimerSeconds")
#define NEXT_SCREEN_ARCADE		THEME->GetMetric ("ScreenSelectDifficulty","NextScreenArcade")
#define NEXT_SCREEN_ONI			THEME->GetMetric ("ScreenSelectDifficulty","NextScreenOni")
#define NEXT_SCREEN_BATTLE		THEME->GetMetric ("ScreenSelectDifficulty","NextScreenBattle")


float CURSOR_X( int page, int choice, int p ) { return CHOICE_X(page,choice) + CURSOR_OFFSET_X(p); }
float CURSOR_Y( int page, int choice, int p ) { return CHOICE_Y(page,choice) + CURSOR_OFFSET_Y(p); }


const ScreenMessage SM_StartTweeningOffScreen	= ScreenMessage(SM_User + 3);
const ScreenMessage SM_StartFadingOut			= ScreenMessage(SM_User + 4);


ScreenSelectDifficulty::ScreenSelectDifficulty()
{
	LOG->Trace( "ScreenSelectDifficulty::ScreenSelectDifficulty()" );

	// Reset the current PlayMode
	GAMESTATE->m_PlayMode = PLAY_MODE_INVALID;
	
	m_Menu.Load(
		THEME->GetPathTo("BGAnimations","select difficulty"), 
		THEME->GetPathTo("Graphics","select difficulty top edge"),
		HELP_TEXT, true, true, TIMER_SECONDS
		);
	this->AddChild( &m_Menu );

	// parse ModeChoices
	for( int page=0; page<NUM_PAGES; page++ )
	{
		CStringArray asBits;
		split( CHOICES_ON_PAGE(page), ",", asBits );
		if( asBits.size() > MAX_CHOICES_PER_PAGE )
			RageException::Throw( "Choices exceed max number of choices per page." );

		unsigned choice;

		for( choice=0; choice<asBits.size(); choice++ )
		{
			Difficulty dc = StringToDifficulty(asBits[choice]);
			PlayMode pm = StringToPlayMode(asBits[choice]);
			
			if( dc!=DIFFICULTY_INVALID )	// valid difficulty
			{
				ModeChoice mc = {
					GAMESTATE->m_CurGame,
					PLAY_MODE_ARCADE,
					GAMESTATE->m_CurStyle,
					dc,
					"",
					GAMESTATE->GetNumSidesJoined() };
				strcpy( mc.name, DifficultyToString(dc) );
				m_ModeChoices[page].push_back( mc );
			}
			else if( pm!=PLAY_MODE_INVALID )	// valid play mode
			{
				ModeChoice mc = {
					GAMESTATE->m_CurGame,
					pm,
					GAMESTATE->m_CurStyle,
					DIFFICULTY_MEDIUM,
					"",
					GAMESTATE->GetNumSidesJoined() };
				strcpy( mc.name, PlayModeToString(pm) );
				m_ModeChoices[page].push_back( mc );
			}
			else
				RageException::Throw( "Invalid Page%dChoice%d value '%s'.", page+1, choice+1, asBits[choice].GetString() );
		}


		for( choice=0; choice<m_ModeChoices[page].size(); choice++ )
		{
			CString sHeaderFile = ssprintf( "select difficulty header %s", m_ModeChoices[page][choice].name );
			CString sPictureFile = ssprintf( "select difficulty picture %s", m_ModeChoices[page][choice].name );

			float fChoiceX = CHOICE_X(page,choice);
			float fChoiceY = CHOICE_Y(page,choice);

			m_sprPicture[page][choice].Load( THEME->GetPathTo("Graphics",sPictureFile) );
			m_sprPicture[page][choice].SetXY( fChoiceX, fChoiceY );
			m_sprPicture[page][choice].SetVertAlign( align_top );
			m_framePages.AddChild( &m_sprPicture[page][choice] );

			m_sprHeader[page][choice].Load( THEME->GetPathTo("Graphics",sHeaderFile) );
			m_sprHeader[page][choice].SetXY( fChoiceX, fChoiceY );
			m_sprHeader[page][choice].SetVertAlign( align_bottom );
			m_framePages.AddChild( &m_sprHeader[page][choice] );
		}

		
		m_sprMoreArrows[page].Load( THEME->GetPathTo("Graphics", ssprintf("select difficulty more page%d",page+1) ) );
		m_sprMoreArrows[page].SetXY( MORE_X(page), MORE_Y(page) );
		m_framePages.AddChild( &m_sprMoreArrows[page] );

		m_sprExplanation[page].Load( THEME->GetPathTo("Graphics", "select difficulty explanation") );
		m_sprExplanation[page].SetXY( EXPLANATION_X(page), EXPLANATION_Y(page) );
		m_sprExplanation[page].StopAnimating();
		m_sprExplanation[page].SetState( page );
		m_framePages.AddChild( &m_sprExplanation[page] );
	}


	m_CurrentPage = PAGE_1;

	for( int p=0; p<NUM_PLAYERS; p++ )
	{
		m_iChoiceOnPage[p] = (INITIAL_CHOICE-1);
		CLAMP( m_iChoiceOnPage[p], 0, (int)m_ModeChoices[0].size()-1 );
		m_bChosen[p] = false;

		if( !GAMESTATE->IsPlayerEnabled((PlayerNumber)p) )
			continue;

		m_sprJoinMessagehadow[p].Load( THEME->GetPathTo("Graphics", "select difficulty cursor 2x1") );
		m_sprJoinMessagehadow[p].StopAnimating();
		m_sprJoinMessagehadow[p].SetState( p );
		m_sprJoinMessagehadow[p].TurnShadowOff();
		m_sprJoinMessagehadow[p].SetDiffuse( RageColor(0,0,0,0.6f) );
		m_framePages.AddChild( &m_sprJoinMessagehadow[p] );

		m_sprCursor[p].Load( THEME->GetPathTo("Graphics", "select difficulty cursor 2x1") );
		m_sprCursor[p].StopAnimating();
		m_sprCursor[p].SetState( p );
		m_sprCursor[p].TurnShadowOff();
		m_sprCursor[p].SetEffectGlowShift();
		m_framePages.AddChild( &m_sprCursor[p] );

		m_sprOK[p].Load( THEME->GetPathTo("Graphics", "select difficulty ok 2x1") );
		m_sprOK[p].SetState( p );
		m_sprOK[p].StopAnimating();
		m_sprOK[p].SetDiffuse( RageColor(1,1,1,0) );
		m_framePages.AddChild( &m_sprOK[p] );
	}

	this->AddChild( &m_framePages );
	
	m_soundChange.Load( THEME->GetPathTo("Sounds", "select difficulty change") );
	m_soundSelect.Load( THEME->GetPathTo("Sounds", "menu start") );
	m_soundDifficult.Load( ANNOUNCER->GetPathTo("select difficulty challenge") );

	SOUNDMAN->PlayOnceFromDir( ANNOUNCER->GetPathTo("select difficulty intro") );

	m_Menu.TweenOnScreenFromMenu( SM_None );
	TweenOnScreen();

	SOUNDMAN->PlayMusic( THEME->GetPathTo("Sounds","select difficulty music") );

	m_fLockInputTime = LOCK_INPUT_TIME;
}


ScreenSelectDifficulty::~ScreenSelectDifficulty()
{
	LOG->Trace( "ScreenSelectDifficulty::~ScreenSelectDifficulty()" );

}


void ScreenSelectDifficulty::Input( const DeviceInput& DeviceI, const InputEventType type, const GameInput &GameI, const MenuInput &MenuI, const StyleInput &StyleI )
{
	LOG->Trace( "ScreenSelectDifficulty::Input()" );

	if( m_Menu.IsClosing() )
		return;

	if( m_fLockInputTime > 0 )
		return;

	Screen::Input( DeviceI, type, GameI, MenuI, StyleI );	// default input handler
}

void ScreenSelectDifficulty::Update( float fDeltaTime )
{
	Screen::Update( fDeltaTime );
	m_fLockInputTime = max( m_fLockInputTime-fDeltaTime, 0 );
}

void ScreenSelectDifficulty::DrawPrimitives()
{
	m_Menu.DrawBottomLayer();
	Screen::DrawPrimitives();
	m_Menu.DrawTopLayer();
}

void ScreenSelectDifficulty::HandleScreenMessage( const ScreenMessage SM )
{
	int p;

	switch( SM )
	{
	case SM_MenuTimer:
		for( p=0; p<NUM_PLAYERS; p++ )
			if( GAMESTATE->IsPlayerEnabled(p) )
				MenuStart( (PlayerNumber)p );
		break;
	case SM_GoToPrevScreen:
		SCREENMAN->SetNewScreen( "ScreenTitleMenu" );
		break;
	case SM_GoToNextScreen:
		for( p=0; p<NUM_PLAYERS; p++ )
		{
			if( !GAMESTATE->IsPlayerEnabled(p) )
				continue;		// skip
			const ModeChoice& mc = m_ModeChoices[m_CurrentPage][m_iChoiceOnPage[p]];
			GAMESTATE->m_PlayMode = mc.pm;
			GAMESTATE->m_PreferredDifficulty[p] = mc.dc;
		}

		switch( GAMESTATE->m_PlayMode )
		{
		case PLAY_MODE_ARCADE:	SCREENMAN->SetNewScreen( NEXT_SCREEN_ARCADE );	break;
		case PLAY_MODE_NONSTOP:
		case PLAY_MODE_ONI:
		case PLAY_MODE_ENDLESS:	SCREENMAN->SetNewScreen( NEXT_SCREEN_ONI );		break;
		case PLAY_MODE_BATTLE:	SCREENMAN->SetNewScreen( NEXT_SCREEN_BATTLE );	break;
		default:	ASSERT(0);
		}
		break;
	case SM_StartTweeningOffScreen:
		TweenOffScreen();
		this->SendScreenMessage( SM_StartFadingOut, 0.8f );
		break;
	case SM_StartFadingOut:
		m_Menu.TweenOffScreenToMenu( SM_GoToNextScreen );
		break;
	}
}

void ScreenSelectDifficulty::MenuLeft( PlayerNumber pn )
{
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

		float fCursorX = CURSOR_X(m_CurrentPage,m_iChoiceOnPage[p],p);
		float fCursorY = CURSOR_Y(m_CurrentPage,m_iChoiceOnPage[p],p);

		m_sprCursor[p].StopTweening();
		m_sprCursor[p].BeginTweening( 0.2f, bChangingPages ? TWEEN_LINEAR : TWEEN_BIAS_BEGIN );
		m_sprCursor[p].SetTweenX( fCursorX - CURSOR_SHADOW_LENGTH_X );
		m_sprCursor[p].SetTweenY( fCursorY - CURSOR_SHADOW_LENGTH_Y );

		m_sprJoinMessagehadow[p].StopTweening();
		m_sprJoinMessagehadow[p].BeginTweening( 0.2f, bChangingPages ? TWEEN_LINEAR : TWEEN_BIAS_BEGIN );
		m_sprJoinMessagehadow[p].SetTweenX( fCursorX );
		m_sprJoinMessagehadow[p].SetTweenY( fCursorY );
	}

	m_soundChange.Play();
}

void ScreenSelectDifficulty::MenuStart( PlayerNumber pn )
{
	if( m_bChosen[pn] == true )
		return;
	m_bChosen[pn] = true;

	for( unsigned page=0; page<NUM_PAGES; page++ )
		m_sprMoreArrows[page].FadeOff( 0, "fade", 0.5f );

	const ModeChoice& mc = m_ModeChoices[m_CurrentPage][m_iChoiceOnPage[pn]];
	SOUNDMAN->PlayOnceFromDir( ANNOUNCER->GetPathTo(ssprintf("select difficulty comment %s",mc.name)) );

	/* XXX: This will play the same announcer twice at the same time; that'll probably
	 * result in an echo effect. */
	if( m_CurrentPage == PAGE_2 )
	{
		// choose this for all the other players too
		for( int p=0; p<NUM_PLAYERS; p++ )
		{
			if( m_bChosen[p] )
				continue;
		
			MenuStart( (PlayerNumber)p );
		}
	}

	float fCursorX = CURSOR_X(m_CurrentPage,m_iChoiceOnPage[pn],pn);
	float fCursorY = CURSOR_Y(m_CurrentPage,m_iChoiceOnPage[pn],pn);

	m_sprCursor[pn].BeginTweening( 0.2f );
	m_sprCursor[pn].BeginTweening( 0.2f );
	m_sprCursor[pn].SetTweenX( fCursorX );
	m_sprCursor[pn].SetTweenY( fCursorY );

	m_sprOK[pn].SetX( fCursorX );
	m_sprOK[pn].SetY( fCursorY );
	m_sprOK[pn].SetDiffuse( RageColor(1,1,1,0) );
	m_sprOK[pn].SetZoom( 2 );

	m_sprOK[pn].BeginTweening( 0.2f );
	m_sprOK[pn].SetTweenZoom( 1 );
	m_sprOK[pn].SetTweenDiffuse( RageColor(1,1,1,1) );

	m_sprJoinMessagehadow[pn].BeginTweening( 0.2f );
	m_sprJoinMessagehadow[pn].BeginTweening( 0.2f );
	m_sprJoinMessagehadow[pn].SetDiffuse( RageColor(0,0,0,0) );

	m_soundSelect.Play();

	// check to see if everyone has chosen
	for( int p=0; p<NUM_PLAYERS; p++ )
	{
		if( GAMESTATE->IsPlayerEnabled((PlayerNumber)p)  &&  m_bChosen[p] == false )
			return;
	}
	m_Menu.StopTimer();
	this->SendScreenMessage( SM_StartTweeningOffScreen, 0.7f );
}

void ScreenSelectDifficulty::MenuBack( PlayerNumber pn )
{
	m_Menu.TweenOffScreenToBlack( SM_GoToPrevScreen, true );
}

void ScreenSelectDifficulty::TweenOffScreen()
{
	unsigned p;

	for( p=0; p < m_SubActors.size(); p++ )
		m_SubActors[p]->StopTweening();
	

	const int page = m_CurrentPage;

	m_sprExplanation[page].SetXY( EXPLANATION_X(page), EXPLANATION_Y(page) );
	m_sprExplanation[page].BeginTweening( 0.5, Actor::TWEEN_BOUNCE_BEGIN );
	m_sprExplanation[page].SetTweenXY( EXPLANATION_X(page)+700, EXPLANATION_Y(page) );

	m_sprMoreArrows[page].BeginTweening( 0.5 );
	m_sprMoreArrows[page].SetTweenDiffuse( RageColor(1,1,1,0) );


	for( p=0; p<NUM_PLAYERS; p++ )
	{
		if( !GAMESTATE->IsPlayerEnabled((PlayerNumber)p) )
			continue;

		m_sprCursor[p].BeginTweening( 0.3f );
		m_sprCursor[p].SetTweenZoom( 0 );

		m_sprOK[p].BeginTweening( 0.3f );
		m_sprOK[p].SetTweenZoom( 0 );

		m_sprJoinMessagehadow[p].BeginTweening( 0.3f );
		m_sprJoinMessagehadow[p].SetTweenDiffuse( RageColor(0,0,0,0) );
	}

	{
		for( unsigned c=0; c<m_ModeChoices[page].size(); c++ )
		{
			const float fPause = c*0.2f;

			// roll up
			m_sprPicture[page][c].FadeOff( fPause, "foldy bounce", 0.3f );
			
			// fly off
			m_sprHeader[page][c].FadeOff( fPause+0.3f, "left far accelerate", 0.4f );
		}
	}
}

void ScreenSelectDifficulty::TweenOnScreen() 
{
	unsigned p;

	for( p=0; p<NUM_PAGES; p++ )
	{
		m_sprExplanation[p].FadeOn( 0, "right bounce", 0.3f );
		m_sprMoreArrows[p].FadeOn( 0, "fade", 0.5f );
	}

	for( p=0; p<NUM_PLAYERS; p++ )
	{
		if( !GAMESTATE->IsPlayerEnabled((PlayerNumber)p) )
			continue;

		float fCursorX = CURSOR_X(m_CurrentPage,m_iChoiceOnPage[p],p);
		float fCursorY = CURSOR_Y(m_CurrentPage,m_iChoiceOnPage[p],p);


		m_sprCursor[p].SetXY( fCursorX, fCursorY );
		m_sprCursor[p].FadeOn( 0, "SpinZ Zoom Fade", 0.3f );

		m_sprJoinMessagehadow[p].SetXY( fCursorX, fCursorY );
		RageColor colorOriginal = m_sprJoinMessagehadow[p].GetDiffuse();
		m_sprJoinMessagehadow[p].SetDiffuse( RageColor(0,0,0,0) );
		m_sprJoinMessagehadow[p].BeginTweening( 0.3f );
		m_sprJoinMessagehadow[p].SetTweenDiffuse( colorOriginal );
	}

	{
		const int p = m_CurrentPage;
		for( unsigned c=0; c<m_ModeChoices[m_CurrentPage].size(); c++ )
		{
			const float fPause = c*0.2f;

			// fly on
			m_sprHeader[p][c].FadeOn( fPause, "left far accelerate", 0.4f );

			// roll down
			m_sprPicture[p][c].FadeOn( fPause+0.4f, "foldy bounce", 0.3f );
		}
	}
}
