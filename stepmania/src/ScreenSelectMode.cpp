#include "stdafx.h"
/****************************************
ScreenEzSelectPlayer,cpp
Desc: See Header
Copyright (C):
Andrew Livy
Chris Danford
*****************************************/

/* Includes */

#include "ScreenSelectMode.h"
#include "ScreenManager.h"
#include "PrefsManager.h"
#include "RageSoundManager.h"
#include "GameConstantsAndTypes.h"
#include "PrefsManager.h"
#include "GameManager.h"
#include "RageLog.h"
#include "AnnouncerManager.h"
#include "GameState.h"
#include "RageException.h"
#include "RageTimer.h"
#include "GameState.h"
#include "ThemeManager.h"

/* Constants */

const ScreenMessage SM_GoToPrevScreen		=	ScreenMessage(SM_User + 1);
const ScreenMessage SM_GoToNextScreen		=	ScreenMessage(SM_User + 2);


#define JOIN_FRAME_X( p )				THEME->GetMetricF("ScreenSelectMode",ssprintf("JoinFrameP%dX",p+1))
#define JOIN_FRAME_Y( i )				THEME->GetMetricF("ScreenSelectMode",ssprintf("JoinFrameP%dY",i+1))
#define JOIN_MESSAGE_X( p )				THEME->GetMetricF("ScreenSelectMode",ssprintf("JoinMessageP%dX",p+1))
#define JOIN_MESSAGE_Y( i )				THEME->GetMetricF("ScreenSelectMode",ssprintf("JoinMessageP%dY",i+1))
#define GUIDE_X							THEME->GetMetricF("ScreenSelectMode","GuideX")
#define GUIDE_Y							THEME->GetMetricF("ScreenSelectMode","GuideY")
#define HELP_TEXT						THEME->GetMetric("ScreenSelectMode","HelpText")
#define TIMER_SECONDS					THEME->GetMetricI("ScreenSelectMode","TimerSeconds")
#define NEXT_SCREEN						THEME->GetMetric("ScreenSelectMode","NextScreen")
#define	SCROLLING_ELEMENT_SPACING		THEME->GetMetricI("ScreenSelectMode","ScrollingElementSpacing")
#define SCROLLING_LIST_X				THEME->GetMetricF("ScreenSelectMode","ScrollingListX")
#define SCROLLING_LIST_Y				THEME->GetMetricF("ScreenSelectMode","ScrollingListY")
#define SELECTION_SPECIFIC_BG_ANIMATIONS	THEME->GetMetricB("ScreenSelectMode","SelectionSpecificBGAnimations")
#define BOUNCE_JOIN_MESSAGE				THEME->GetMetricB("ScreenSelectMode","BounceJoinMessage")
#define FOLD_ON_JOIN					THEME->GetMetricB("ScreenSelectMode","FoldOnJoin")

const float TWEEN_TIME		= 0.35f;


/************************************
ScreenSelectMode (Constructor)
Desc: Sets up the screen display
************************************/

ScreenSelectMode::ScreenSelectMode()
{
	LOG->Trace( "ScreenSelectMode::ScreenSelectMode()" );
	GAMESTATE->m_bPlayersCanJoin = true;
	SCREENMAN->RefreshCreditsMessages();

	
	GAMESTATE->m_PlayMode = PLAY_MODE_ARCADE;	// the only mode you can select on this screen


	/*********** TODO: MAKE THIS WORK FOR ALL GAME STYLES! *************/
	m_ChoiceListFrame.Load( THEME->GetPathTo("Graphics","select mode list frame"));
	m_ChoiceListFrame.SetXY( SCROLLING_LIST_X, SCROLLING_LIST_Y);
	this->AddChild( &m_ChoiceListFrame );

	m_ScrollingList.SetXY( CENTER_X, SCROLLING_LIST_Y );
	m_ScrollingList.SetSpacing( SCROLLING_ELEMENT_SPACING );
	m_ScrollingList.SetNumberVisible( 9 );
	this->AddChild( &m_ScrollingList );

	m_ChoiceListHighlight.Load( THEME->GetPathTo("Graphics","select mode list highlight"));
	m_ChoiceListHighlight.SetXY( CENTER_X, SCROLLING_LIST_Y);
	this->AddChild( &m_ChoiceListHighlight );

	m_Guide.Load( THEME->GetPathTo("Graphics","select mode guide"));
	m_Guide.SetXY( GUIDE_X, GUIDE_Y );
	this->AddChild( &m_Guide );

	for( int p=0; p<NUM_PLAYERS; p++ )
	{
		m_sprJoinFrame[p].Load( THEME->GetPathTo("Graphics","select player join frame 1x2") );
		m_sprJoinFrame[p].StopAnimating();
		m_sprJoinFrame[p].SetState( p );
		m_sprJoinFrame[p].SetXY( JOIN_FRAME_X(p), JOIN_FRAME_Y(p) );
		this->AddChild( &m_sprJoinFrame[p] );

		if( GAMESTATE->m_bSideIsJoined[p] )
			m_sprJoinFrame[p].SetZoomY( 0 );

		m_sprJoinMessage[p].Load( THEME->GetPathTo("Graphics","select player join message 2x2") );
		m_sprJoinMessage[p].StopAnimating();
		m_sprJoinMessage[p].SetState( p );
		m_sprJoinMessage[p].SetXY( JOIN_MESSAGE_X(p), JOIN_MESSAGE_Y(p) );
		if( BOUNCE_JOIN_MESSAGE )
			m_sprJoinMessage[p].SetEffectBouncing( RageVector3(0,10,0), 0.5f );
		this->AddChild( &m_sprJoinMessage[p] );
	
		if( GAMESTATE->m_bSideIsJoined[p] )
		{
			m_sprJoinMessage[p].SetState( p+NUM_PLAYERS );

			if( FOLD_ON_JOIN )
			{
				m_sprJoinMessage[p].SetZoomY( 0 );
				m_sprJoinFrame[p].SetZoomY( 0 );
			}
		}
	}
	

	m_Menu.Load( 	
		THEME->GetPathTo("BGAnimations","select style"), 
		THEME->GetPathTo("Graphics","select style top edge"),
		HELP_TEXT, true, true, TIMER_SECONDS
		);
	this->AddChild( &m_Menu );

	m_soundSelect.Load( THEME->GetPathTo("Sounds","menu start") );
	m_soundChange.Load( THEME->GetPathTo("Sounds","select style change"), true );

	SOUNDMAN->PlayOnceFromDir( ANNOUNCER->GetPathTo("select style intro") );

	SOUNDMAN->PlayMusic( THEME->GetPathTo("Sounds","select style music") );

	RefreshModeChoices();

	if( SELECTION_SPECIFIC_BG_ANIMATIONS )
	{
		CString sGameName = GAMESTATE->GetCurrentGameDef()->m_szName;

		for( unsigned i=0; i<m_apPossibleModeChoices.size(); i++ )
		{
			CString sChoiceName = m_apPossibleModeChoices[i]->name;
			m_Infotext[i].Load( THEME->GetPathTo("Graphics",ssprintf("select mode infotext %s %s", sGameName.GetString(), sChoiceName.GetString())) );	
			m_Infotext[i].SetXY( GUIDE_X, GUIDE_Y );
			this->AddChild( &m_Infotext[i] );
			m_Infotext[i].SetDiffuse( RageColor(0,0,0,0));
		}
	}
	AfterChange();

	TweenOnScreen();
	m_Menu.TweenOnScreenFromBlack( SM_None );

}

/************************************
~ScreenSelectMode (Destructor)
Desc: Writes line to log when screen
is terminated.
************************************/
ScreenSelectMode::~ScreenSelectMode()
{
	LOG->Trace( "ScreenSelectMode::~ScreenSelectMode()" );
}

/************************************
Update
Desc: Animates the 1p/2p selection
************************************/
void ScreenSelectMode::Update( float fDeltaTime )
{
	m_BGAnimations[ m_ScrollingList.GetSelection() ].Update( fDeltaTime );
	Screen::Update( fDeltaTime );
}

/************************************
DrawPrimitives
Desc: Draws the screen =P
************************************/

void ScreenSelectMode::DrawPrimitives()
{
	m_Menu.DrawBottomLayer();
	m_BGAnimations[ m_ScrollingList.GetSelection() ].Draw();
	Screen::DrawPrimitives();
	m_Menu.DrawTopLayer();
}

/************************************
Input
Desc: Handles player input.
************************************/
void ScreenSelectMode::Input( const DeviceInput& DeviceI, const InputEventType type, const GameInput &GameI, const MenuInput &MenuI, const StyleInput &StyleI )
{
	LOG->Trace( "ScreenSelectMode::Input()" );

	if( m_Menu.IsClosing() )
		return;

	Screen::Input( DeviceI, type, GameI, MenuI, StyleI );	// default input handler
}

/************************************
HandleScreenMessage
Desc: Handles Screen Messages and changes
	game states.
************************************/
void ScreenSelectMode::HandleScreenMessage( const ScreenMessage SM )
{
	Screen::HandleScreenMessage( SM );

	switch( SM )
	{
	case SM_MenuTimer:
		MenuStart( PLAYER_INVALID );
		m_Menu.StopTimer();

		TweenOffScreen();
		m_Menu.TweenOffScreenToMenu( SM_GoToNextScreen );
		break;
	case SM_GoToPrevScreen:
		SOUNDMAN->music->StopPlaying();
		SCREENMAN->SetNewScreen( "ScreenTitleMenu" );
		break;
	case SM_GoToNextScreen:
		// Put the mode choice into effect
		int iSelection = m_ScrollingList.GetSelection();
		const ModeChoice* pChoice = m_apPossibleModeChoices[ iSelection ];
		GAMESTATE->m_CurStyle = pChoice->style;
		GAMESTATE->m_PlayMode = pChoice->pm;
		for( int p=0; p<NUM_PLAYERS; p++ )
			GAMESTATE->m_PreferredDifficulty[p] = pChoice->dc;

		switch( GAMESTATE->m_PlayMode )
		{
		case PLAY_MODE_ARCADE:
			SCREENMAN->SetNewScreen( NEXT_SCREEN );
			break;
		case PLAY_MODE_ONI:
		case PLAY_MODE_ENDLESS:
			SCREENMAN->SetNewScreen( "ScreenSelectCourse" );
			break;
		}
		break;
	}
}

void ScreenSelectMode::RefreshModeChoices()
{
	int iNumSidesJoined = GAMESTATE->GetNumSidesJoined();

	GAMEMAN->GetModesChoicesForGame( GAMESTATE->m_CurGame, m_apPossibleModeChoices );
	ASSERT( !m_apPossibleModeChoices.empty() );

	int i;

	// remove ModeChoices that won't work with the current number of players
	for( i=m_apPossibleModeChoices.size()-1; i>=0; i-- )
		if( m_apPossibleModeChoices[i]->numSidesJoinedToPlay != iNumSidesJoined )
			m_apPossibleModeChoices.erase( m_apPossibleModeChoices.begin()+i, 
										  m_apPossibleModeChoices.begin()+i+1 );

	CString sGameName = GAMESTATE->GetCurrentGameDef()->m_szName;

	CStringArray asGraphicPaths;
	for( unsigned j=0; j<m_apPossibleModeChoices.size(); j++ )
	{
		const ModeChoice* pChoice = m_apPossibleModeChoices[j];
		asGraphicPaths.push_back( THEME->GetPathTo("Graphics", ssprintf("select mode choice %s %s", sGameName.GetString(), pChoice->name) ) );
	}

	m_ScrollingList.Load( asGraphicPaths );


	if( SELECTION_SPECIFIC_BG_ANIMATIONS )
	{
		CString sGameName = GAMESTATE->GetCurrentGameDef()->m_szName;

		for( unsigned i=0; i<m_apPossibleModeChoices.size(); i++ )
		{
			CString sChoiceName = m_apPossibleModeChoices[i]->name;
			m_BGAnimations[i].LoadFromAniDir( THEME->GetPathTo("BGAnimations",ssprintf("select mode %s %s", sGameName.GetString(), sChoiceName.GetString())) );	
		}
	}
}


/************************************
MenuBack
Desc: Actions performed when a player 
presses the button bound to back
************************************/
void ScreenSelectMode::MenuBack( PlayerNumber pn )
{
	SOUNDMAN->music->StopPlaying();

	m_Menu.TweenOffScreenToBlack( SM_GoToPrevScreen, true );
}

void ScreenSelectMode::AfterChange()
{
	if( SELECTION_SPECIFIC_BG_ANIMATIONS )
	{
		for( unsigned i=0; i<m_apPossibleModeChoices.size(); i++ )
		{
			if(i == m_ScrollingList.GetSelection())
			{
				m_Infotext[i].SetDiffuse(RageColor(1,1,1,1));
			}
			else
			{
				m_Infotext[i].SetDiffuse(RageColor(0,0,0,0));
			}
		}
	}
//	CString sGameName = GAMESTATE->GetCurrentGameDef()->m_szName;
//	const ModeChoice& choice = m_aPossibleModeChoices[ m_ScrollingList.GetSelection() ];
}

void ScreenSelectMode::MenuLeft( PlayerNumber pn )
{
	m_ScrollingList.Left();
	m_soundChange.Play();
	AfterChange();
}

void ScreenSelectMode::MenuRight( PlayerNumber pn )
{
	m_ScrollingList.Right();
	m_soundChange.Play();
	AfterChange();
}

/************************************
MenuDown
Desc: Actions performed when a player 
presses the button bound to down
************************************/
void ScreenSelectMode::MenuDown( PlayerNumber pn )
{
	if( GAMESTATE->m_bSideIsJoined[pn] )	// already joined
		return;	// ignore

	MenuStart( pn );
}

/************************************
MenuStart
Desc: Actions performed when a player 
presses the button bound to start
************************************/
void ScreenSelectMode::MenuStart( PlayerNumber pn )
{
	if( pn == PLAYER_INVALID )
		pn = GAMESTATE->m_MasterPlayerNumber;

	if( !GAMESTATE->m_bSideIsJoined[pn] )
	{
		// join them
		GAMESTATE->m_bSideIsJoined[pn] = true;
		SCREENMAN->RefreshCreditsMessages();
		m_soundSelect.Play();

		m_sprJoinMessage[pn].SetState( pn + NUM_PLAYERS );

		if( FOLD_ON_JOIN )
		{
			m_sprJoinMessage[pn].BeginTweening( 0.25f );
			m_sprJoinMessage[pn].SetTweenZoomY( 0 );
			m_sprJoinFrame[pn].BeginTweening( 0.25f );
			m_sprJoinFrame[pn].SetTweenZoomY( 0 );
		}

		RefreshModeChoices();

		m_ScrollingList.SetSelection( 0 );
	}
	else
	{
		// made a selection
		m_soundSelect.Play();

		TweenOffScreen();
		m_Menu.TweenOffScreenToMenu( SM_GoToNextScreen );
		GAMESTATE->m_bPlayersCanJoin = false;
		SCREENMAN->RefreshCreditsMessages();
	}
}

void ScreenSelectMode::TweenOnScreen()
{
	float fOriginalZoomY = m_ScrollingList.GetZoomY();
	m_ScrollingList.BeginTweening( 0.5f );
	m_ScrollingList.SetTweenZoomY( fOriginalZoomY );

	for( int p=0; p<NUM_PLAYERS; p++ )
	{
		float fOffScreenOffset = float( (p==PLAYER_1) ? -SCREEN_WIDTH/2 : +SCREEN_WIDTH/2 );

		float fOriginalX;
		
		fOriginalX = m_sprJoinMessage[p].GetX();
		m_sprJoinMessage[p].SetX( m_sprJoinMessage[p].GetX()+fOffScreenOffset );
		m_sprJoinMessage[p].BeginTweening( 0.5f, Actor::TWEEN_BOUNCE_END );
		m_sprJoinMessage[p].SetTweenX( fOriginalX );

		fOriginalX = m_sprJoinFrame[p].GetX();
		m_sprJoinFrame[p].SetX( m_sprJoinMessage[p].GetX()+fOffScreenOffset );
		m_sprJoinFrame[p].BeginTweening( 0.5f, Actor::TWEEN_BOUNCE_END );
		m_sprJoinFrame[p].SetTweenX( fOriginalX );
	}
}

void ScreenSelectMode::TweenOffScreen()
{
	m_ScrollingList.BeginTweening( 0.5f );
	m_ScrollingList.SetTweenZoomY( 0 );

	for( int p=0; p<NUM_PLAYERS; p++ )
	{
		float fOffScreenOffset = float( (p==PLAYER_1) ? -SCREEN_WIDTH : +SCREEN_WIDTH );

		m_sprJoinMessage[p].BeginTweening( 0.5f, Actor::TWEEN_BIAS_END );
		m_sprJoinMessage[p].SetTweenX( m_sprJoinMessage[p].GetX()+fOffScreenOffset );
		m_sprJoinFrame[p].BeginTweening( 0.5f, Actor::TWEEN_BIAS_END );
		m_sprJoinFrame[p].SetTweenX( m_sprJoinMessage[p].GetX()+fOffScreenOffset );
	}
}
