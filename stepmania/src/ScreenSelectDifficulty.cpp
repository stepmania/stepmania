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
#define BEGINNER_X()			THEME->GetMetricF("ScreenSelectDifficulty","BeginnerX")
#define BEGINNER_Y()			THEME->GetMetricF("ScreenSelectDifficulty","BeginnerY")
#define EASY_X()				THEME->GetMetricF("ScreenSelectDifficulty","EasyX")
#define EASY_Y()				THEME->GetMetricF("ScreenSelectDifficulty","EasyY")
#define MEDIUM_X()				THEME->GetMetricF("ScreenSelectDifficulty","MediumX")
#define MEDIUM_Y()				THEME->GetMetricF("ScreenSelectDifficulty","MediumY")
#define HARD_X()				THEME->GetMetricF("ScreenSelectDifficulty","HardX")
#define HARD_Y()				THEME->GetMetricF("ScreenSelectDifficulty","HardY")
#define NONSTOP_X()				THEME->GetMetricF("ScreenSelectDifficulty","NonstopX")
#define NONSTOP_Y()				THEME->GetMetricF("ScreenSelectDifficulty","NonstopY")
#define ONI_X()					THEME->GetMetricF("ScreenSelectDifficulty","OniX")
#define ONI_Y()					THEME->GetMetricF("ScreenSelectDifficulty","OniY")
#define ENDLESS_X()				THEME->GetMetricF("ScreenSelectDifficulty","EndlessX")
#define ENDLESS_Y()				THEME->GetMetricF("ScreenSelectDifficulty","EndlessY")
#define CURSOR_OFFSET_X( p )	THEME->GetMetricF("ScreenSelectDifficulty",ssprintf("CursorOffsetP%dX",p+1))
#define CURSOR_OFFSET_Y( i )	THEME->GetMetricF("ScreenSelectDifficulty",ssprintf("CursorOffsetP%dY",i+1))
#define CURSOR_SHADOW_LENGTH_X	THEME->GetMetricF("ScreenSelectDifficulty","CursorShadowLengthX")
#define CURSOR_SHADOW_LENGTH_Y	THEME->GetMetricF("ScreenSelectDifficulty","CursorShadowLengthY")
#define INITIAL_CHOICE			THEME->GetMetricI("ScreenSelectDifficulty","InitialChoice")
#define HELP_TEXT				THEME->GetMetric("ScreenSelectDifficulty","HelpText")
#define TIMER_SECONDS			THEME->GetMetricI("ScreenSelectDifficulty","TimerSeconds")
#define NEXT_SCREEN_ARCADE		THEME->GetMetric("ScreenSelectDifficulty","NextScreenArcade")
#define NEXT_SCREEN_ONI			THEME->GetMetric("ScreenSelectDifficulty","NextScreenOni")

float CHOICE_X( int choice )
{
	switch( choice )
	{
	case 0: return BEGINNER_X();
	case 1: return EASY_X();
	case 2: return MEDIUM_X();
	case 3: return HARD_X();
	case 4: return NONSTOP_X();
	case 5: return ONI_X();
	case 6: return ENDLESS_X();
	default:	ASSERT(0);	return 0;
	}
}

float CHOICE_Y( int choice )
{
	switch( choice )
	{
	case 0: return BEGINNER_Y();
	case 1: return EASY_Y();
	case 2: return MEDIUM_Y();
	case 3: return HARD_Y();
	case 4: return NONSTOP_Y();
	case 5: return ONI_Y();
	case 6: return ENDLESS_Y();
	default:	ASSERT(0);	return 0;
	}
}

const CString CHOICE_TEXT[ScreenSelectDifficulty::NUM_CHOICES] = 
{
	"beginner",
	"easy",
	"medium",
	"hard",
	"nonstop",
	"oni",
	"endless"
};

float CURSOR_X( int choice, int p ) { return CHOICE_X(choice) + CURSOR_OFFSET_X(p); }
float CURSOR_Y( int choice, int p ) { return CHOICE_Y(choice) + CURSOR_OFFSET_Y(p); }



const ScreenMessage SM_StartTweeningOffScreen	= ScreenMessage(SM_User + 3);
const ScreenMessage SM_StartFadingOut			= ScreenMessage(SM_User + 4);


ScreenSelectDifficulty::ScreenSelectDifficulty()
{
	LOG->Trace( "ScreenSelectDifficulty::ScreenSelectDifficulty()" );

	// Reset the current PlayMode
	GAMESTATE->m_PlayMode = PLAY_MODE_INVALID;


	unsigned p;
	
	m_Menu.Load(
		THEME->GetPathTo("BGAnimations","select difficulty"), 
		THEME->GetPathTo("Graphics","select difficulty top edge"),
		HELP_TEXT, true, true, TIMER_SECONDS
		);
	this->AddChild( &m_Menu );


	for( unsigned c=0; c<NUM_CHOICES; c++ )
	{
		CString sHeaderFile = ssprintf( "select difficulty header %s", CHOICE_TEXT[c].c_str() );
		CString sPictureFile = ssprintf( "select difficulty picture %s", CHOICE_TEXT[c].c_str() );

		m_sprPicture[c].Load( THEME->GetPathTo("Graphics",sPictureFile) );
		m_sprPicture[c].SetXY( CHOICE_X(c), CHOICE_Y(c) );
		m_sprPicture[c].SetVertAlign( align_top );
		m_framePages.AddChild( &m_sprPicture[c] );

		m_sprHeader[c].Load( THEME->GetPathTo("Graphics",sHeaderFile) );
		m_sprHeader[c].SetXY( CHOICE_X(c), CHOICE_Y(c) );
		m_sprHeader[c].SetVertAlign( align_bottom );
		m_framePages.AddChild( &m_sprHeader[c] );
	}

	for( p=0; p<NUM_PAGES; p++ )
	{
		m_sprMoreArrows[p].Load( THEME->GetPathTo("Graphics", p==0 ? "select difficulty more page1" : "select difficulty more page2" ) );
		m_sprMoreArrows[p].SetXY( MORE_X(p), MORE_Y(p) );
		m_framePages.AddChild( &m_sprMoreArrows[p] );

		m_sprExplanation[p].Load( THEME->GetPathTo("Graphics", "select difficulty explanation") );
		m_sprExplanation[p].SetXY( EXPLANATION_X(p), EXPLANATION_Y(p) );
		m_sprExplanation[p].StopAnimating();
		m_sprExplanation[p].SetState( p );
		m_framePages.AddChild( &m_sprExplanation[p] );
	}


	for( p=0; p<NUM_PLAYERS; p++ )
	{
		m_Choice[p] = (Choice)(INITIAL_CHOICE-1);
		CLAMP( m_Choice[p], (Choice)0, (Choice)(NUM_CHOICES-1) );
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
		m_sprCursor[p].SetEffectGlowCamelion();
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
			switch( m_Choice[p] )
			{
			case CHOICE_BEGINNER:	GAMESTATE->m_PreferredDifficulty[p] = DIFFICULTY_BEGINNER;	break;
			case CHOICE_EASY:		GAMESTATE->m_PreferredDifficulty[p] = DIFFICULTY_EASY;		break;
			case CHOICE_NONSTOP:	// need to set preferred difficulty even for courses
			case CHOICE_ONI:
			case CHOICE_ENDLESS:
			case CHOICE_MEDIUM:		GAMESTATE->m_PreferredDifficulty[p] = DIFFICULTY_MEDIUM;	break;
			case CHOICE_HARD:		GAMESTATE->m_PreferredDifficulty[p] = DIFFICULTY_HARD;		break;
			default:	ASSERT(0);
			}
		}

		switch( m_Choice[GAMESTATE->m_MasterPlayerNumber] )
		{
		case CHOICE_BEGINNER:
		case CHOICE_EASY:
		case CHOICE_MEDIUM:
		case CHOICE_HARD:		GAMESTATE->m_PlayMode = PLAY_MODE_ARCADE;	break;
		case CHOICE_NONSTOP:	GAMESTATE->m_PlayMode = PLAY_MODE_NONSTOP;	break;
		case CHOICE_ONI:		GAMESTATE->m_PlayMode = PLAY_MODE_ONI;		break;
		case CHOICE_ENDLESS:	GAMESTATE->m_PlayMode = PLAY_MODE_ENDLESS;	break;
		default:	ASSERT(0);	// bad selection
		}

		switch( GAMESTATE->m_PlayMode )
		{
		case PLAY_MODE_ARCADE:	SCREENMAN->SetNewScreen( NEXT_SCREEN_ARCADE );	break;
		case PLAY_MODE_NONSTOP:
		case PLAY_MODE_ONI:
		case PLAY_MODE_ENDLESS:	SCREENMAN->SetNewScreen( NEXT_SCREEN_ONI );		break;
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
	if( m_Choice[pn] == 0 )	// can't go left any more
		return;
	if( m_bChosen[pn] )
		return;

	ChangeTo( pn, m_Choice[pn], m_Choice[pn]-1 );
}


void ScreenSelectDifficulty::MenuRight( PlayerNumber pn )
{
	if( m_Choice[pn] == NUM_CHOICES-1 )	// can't go right any more
		return;
	if( m_bChosen[pn] )
		return;

	ChangeTo( pn, m_Choice[pn], m_Choice[pn]+1 );
}

bool ScreenSelectDifficulty::IsOnPage2( int iItemIndex )
{
	ASSERT( iItemIndex >= 0  &&  iItemIndex < NUM_CHOICES );

	return iItemIndex >= NUM_CHOICES_ON_PAGE_1;
}

bool ScreenSelectDifficulty::SelectedSomethingOnPage2()
{
	for( int p=0; p<NUM_PLAYERS; p++ )
	{
		if( GAMESTATE->IsPlayerEnabled(p)  &&  IsOnPage2(m_Choice[p]) )
			return true;
	}
	return false;
}

void ScreenSelectDifficulty::ChangeTo( PlayerNumber pn, int iOldChoice, int iNewChoice )
{
	bool bChangedPagesFrom1To2 = !IsOnPage2(iOldChoice) && IsOnPage2(iNewChoice);
	bool bChangedPagesFrom2To1 = IsOnPage2(iOldChoice) && !IsOnPage2(iNewChoice);
	bool bChangedPages = bChangedPagesFrom1To2 || bChangedPagesFrom2To1;
	bool bSelectedSomethingOnPage1 = !IsOnPage2(iNewChoice);
	bool bSelectedSomethingOnPage2 = IsOnPage2(iNewChoice);

	bool bSomeoneMadeAChoice = false;
	int p;
	for( p=0; p<NUM_PLAYERS; p++ )
		if( GAMESTATE->IsPlayerEnabled(p) && m_bChosen[p] )
			bSomeoneMadeAChoice = true;

	if( bSomeoneMadeAChoice && (bChangedPagesFrom1To2 || bChangedPagesFrom2To1) )
		return;	// don't allow changing pages after one player has chosen

	if( bSelectedSomethingOnPage2 || bChangedPagesFrom2To1 )
	{
		// change both players
		for( int p=0; p<NUM_PLAYERS; p++ )
			m_Choice[p] = (Choice)iNewChoice;
	}
	else	// moving around in page 1
	{
		// change only the player who pressed the button
		m_Choice[pn] = (Choice)iNewChoice;

	}


	if( bChangedPagesFrom1To2 )
	{
		m_soundDifficult.Stop();
		m_soundDifficult.PlayRandom();
	}

	if( bChangedPagesFrom1To2 || bChangedPagesFrom2To1 )
	{
		m_framePages.StopTweening();
		m_framePages.BeginTweening( 0.2f );
		m_framePages.SetTweenX( bSelectedSomethingOnPage1 ? 0.0f : -SCREEN_WIDTH );
	}

	for( p=0; p<NUM_PLAYERS; p++ )
	{
		if( bSelectedSomethingOnPage2 || bChangedPagesFrom2To1 || p==pn )
		{
			m_sprCursor[p].StopTweening();
			m_sprCursor[p].BeginTweening( 0.2f, bChangedPages ? TWEEN_LINEAR : TWEEN_BIAS_BEGIN );
			m_sprCursor[p].SetTweenX( CURSOR_X(m_Choice[p],(PlayerNumber)p) - CURSOR_SHADOW_LENGTH_X );
			m_sprCursor[p].SetTweenY( CURSOR_Y(m_Choice[p],(PlayerNumber)p) - CURSOR_SHADOW_LENGTH_Y );

			m_sprJoinMessagehadow[p].StopTweening();
			m_sprJoinMessagehadow[p].BeginTweening( 0.2f, bChangedPages ? TWEEN_LINEAR : TWEEN_BIAS_BEGIN );
			m_sprJoinMessagehadow[p].SetTweenX( CURSOR_X(m_Choice[p],(PlayerNumber)p) );
			m_sprJoinMessagehadow[p].SetTweenY( CURSOR_Y(m_Choice[p],(PlayerNumber)p) );
		}
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


	m_soundSelect.Play();
	int iSelection = m_Choice[pn];

	switch( iSelection )
	{
	case 0:	SOUNDMAN->PlayOnceFromDir( ANNOUNCER->GetPathTo("select difficulty comment beginner") );	break;
	case 1:	SOUNDMAN->PlayOnceFromDir( ANNOUNCER->GetPathTo("select difficulty comment easy") );		break;
	case 2:	SOUNDMAN->PlayOnceFromDir( ANNOUNCER->GetPathTo("select difficulty comment medium") );		break;
	case 3:	SOUNDMAN->PlayOnceFromDir( ANNOUNCER->GetPathTo("select difficulty comment hard") );		break;
	case 4:	SOUNDMAN->PlayOnceFromDir( ANNOUNCER->GetPathTo("select difficulty comment nonstop") );		break;
	case 5:	SOUNDMAN->PlayOnceFromDir( ANNOUNCER->GetPathTo("select difficulty comment oni") );			break;
	case 6:	SOUNDMAN->PlayOnceFromDir( ANNOUNCER->GetPathTo("select difficulty comment endless") );		break;
	}

	/* XXX: This will play the same announcer twice at the same time; that'll probably
	 * result in an echo effect. */
	if( IsOnPage2(iSelection) )	// chose something on page 2
	{
		// choose this for all the other players too
		for( int p=0; p<NUM_PLAYERS; p++ )
		{
			if( m_bChosen[p] )
				continue;
		
			MenuStart( (PlayerNumber)p );
		}
	}

	m_sprCursor[pn].BeginTweening( 0.2f );
	m_sprCursor[pn].BeginTweening( 0.2f );
	m_sprCursor[pn].SetTweenX( CURSOR_X(iSelection, pn) );
	m_sprCursor[pn].SetTweenY( CURSOR_Y(iSelection, pn) );

	m_sprOK[pn].SetX( CURSOR_X(iSelection, pn) );
	m_sprOK[pn].SetY( CURSOR_Y(iSelection, pn) );
	m_sprOK[pn].SetDiffuse( RageColor(1,1,1,0) );
	m_sprOK[pn].SetZoom( 2 );

	m_sprOK[pn].BeginTweening( 0.2f );
	m_sprOK[pn].SetTweenZoom( 1 );
	m_sprOK[pn].SetTweenDiffuse( RageColor(1,1,1,1) );

	m_sprJoinMessagehadow[pn].BeginTweening( 0.2f );
	m_sprJoinMessagehadow[pn].BeginTweening( 0.2f );
	m_sprJoinMessagehadow[pn].SetDiffuse( RageColor(0,0,0,0) );


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
	
	for( p=0; p<NUM_PAGES; p++ )
	{
		if( p == 0  &&  SelectedSomethingOnPage2() )
			continue;	// skip

		m_sprExplanation[p].SetXY( EXPLANATION_X(p), EXPLANATION_Y(p) );
		m_sprExplanation[p].BeginTweening( 0.5, Actor::TWEEN_BOUNCE_BEGIN );
		m_sprExplanation[p].SetTweenXY( EXPLANATION_X(p)+700, EXPLANATION_Y(p) );

		m_sprMoreArrows[p].BeginTweening( 0.5 );
		m_sprMoreArrows[p].SetTweenDiffuse( RageColor(1,1,1,0) );
	}

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

	for( unsigned c=0; c<NUM_CHOICES; c++ )
	{
		if( SelectedSomethingOnPage2() != IsOnPage2(c) )	// item isn't on selected page
			continue;	// don't tween

		const float fPauseTime = c*0.2f;

		// pause
		m_sprHeader[c].BeginTweening( fPauseTime );

		m_sprPicture[c].BeginTweening( fPauseTime );

		// roll up
		m_sprHeader[c].BeginTweening( 0.3f, TWEEN_BOUNCE_BEGIN );

		m_sprPicture[c].BeginTweening( 0.3f, TWEEN_BOUNCE_BEGIN );
		m_sprPicture[c].SetTweenZoomY( 0 );

		// fly off
		m_sprHeader[c].BeginTweening( 0.4f, TWEEN_BIAS_END );
		m_sprHeader[c].SetTweenXY( CHOICE_X(c)-700, CHOICE_Y(c) );

		m_sprPicture[c].BeginTweening( 0.4f, TWEEN_BIAS_END );
		m_sprPicture[c].SetTweenXY( CHOICE_X(c)-700, CHOICE_Y(c) );
	}
}

void ScreenSelectDifficulty::TweenOnScreen() 
{
	unsigned p;

	for( p=0; p<NUM_PAGES; p++ )
	{
		m_sprExplanation[p].SetXY( EXPLANATION_X(p)+700, EXPLANATION_Y(p) );
		m_sprExplanation[p].BeginTweening( 0.3f, Actor::TWEEN_BOUNCE_END );
		m_sprExplanation[p].SetTweenXY( EXPLANATION_X(p), EXPLANATION_Y(p) );

		m_sprMoreArrows[p].SetDiffuse( RageColor(1,1,1,0) );
		m_sprMoreArrows[p].BeginTweening( 0.5 );
		m_sprMoreArrows[p].SetTweenDiffuse( RageColor(1,1,1,1) );
	}

	for( p=0; p<NUM_PLAYERS; p++ )
	{
		if( !GAMESTATE->IsPlayerEnabled((PlayerNumber)p) )
			continue;

		int iSelection = m_Choice[p];

		m_sprCursor[p].SetXY( CURSOR_X(iSelection,(PlayerNumber)p), CURSOR_Y(iSelection,(PlayerNumber)p) );
		/*
		m_sprCursor[p].SetDiffuse( RageColor(1,1,1,0) );
		m_sprCursor[p].SetRotation( D3DX_PI );
		m_sprCursor[p].SetZoom( 2 );
		m_sprCursor[p].BeginTweening( 0.3f );
		m_sprCursor[p].SetTweenDiffuse( RageColor(1,1,1,1) );
		m_sprCursor[p].SetTweenRotationZ( 0 );
		m_sprCursor[p].SetTweenZoom( 1 );
		*/
		m_sprCursor[p].FadeOn( 0, "SpinZ ZoomX ZoomY Fade", 0.3f );

		m_sprJoinMessagehadow[p].SetXY( CURSOR_X(iSelection,(PlayerNumber)p), CURSOR_Y(iSelection,(PlayerNumber)p) );
		RageColor colorOriginal = m_sprJoinMessagehadow[p].GetDiffuse();
		m_sprJoinMessagehadow[p].SetDiffuse( RageColor(0,0,0,0) );
		m_sprJoinMessagehadow[p].BeginTweening( 0.3f );
		m_sprJoinMessagehadow[p].SetTweenDiffuse( colorOriginal );
	}

	for( unsigned d=0; d<NUM_CHOICES; d++ )
	{
		const float fPauseTime = d*0.2f;

		if( SelectedSomethingOnPage2() != IsOnPage2(d) )	// item isn't on the current page
			continue;	// don't tween

		// set off screen
		m_sprHeader[d].SetXY( CHOICE_X(d)-700, CHOICE_Y(d) );
	
		m_sprPicture[d].SetXY( CHOICE_X(d)-700, CHOICE_Y(d) );
		m_sprPicture[d].SetZoomY( 0 );

		// pause
		m_sprHeader[d].BeginTweening( fPauseTime );

		m_sprPicture[d].BeginTweening( fPauseTime );

		// fly on
		m_sprHeader[d].BeginTweening( 0.5f, TWEEN_BIAS_BEGIN );
		m_sprHeader[d].SetTweenXY( CHOICE_X(d), CHOICE_Y(d) );

		m_sprPicture[d].BeginTweening( 0.5f, TWEEN_BIAS_BEGIN );
		m_sprPicture[d].SetTweenXY( CHOICE_X(d), CHOICE_Y(d) );

		// roll down
		m_sprHeader[d].BeginTweening( 0.3f, TWEEN_BOUNCE_END );

		m_sprPicture[d].BeginTweening( 0.3f, TWEEN_BOUNCE_END );
		m_sprPicture[d].SetTweenZoomY( 1 );

	}
}
