#include "stdafx.h"
/****************************************
ScreenEzSelectPlayer,cpp
Desc: See Header
Copyright (C):
Andrew Livy
*****************************************/

/* Includes */

#include "ScreenEz2SelectStyle.h"
#include "ScreenManager.h"
#include "PrefsManager.h"
#include "RageMusic.h"
#include "GameConstantsAndTypes.h"
#include "PrefsManager.h"
#include "GameManager.h"
#include "RageLog.h"
#include "AnnouncerManager.h"
#include "GameState.h"
#include "RageException.h"
#include "RageTimer.h"
#include "GameState.h"

/* Constants */

const ScreenMessage SM_GoToPrevScreen		=	ScreenMessage(SM_User + 1);
const ScreenMessage SM_GoToNextScreen		=	ScreenMessage(SM_User + 2);


#define CURSOR_X( p )			THEME->GetMetricF("ScreenEz2SelectStyle",ssprintf("CursorP%dX",p+1))
#define CURSOR_Y( i )			THEME->GetMetricF("ScreenEz2SelectStyle",ssprintf("CursorP%dY",i+1))
#define CONTROLLER_X( p )		THEME->GetMetricF("ScreenEz2SelectStyle",ssprintf("ControllerP%dX",p+1))
#define CONTROLLER_Y( i )		THEME->GetMetricF("ScreenEz2SelectStyle",ssprintf("ControllerP%dY",i+1))
#define HELP_TEXT				THEME->GetMetric("ScreenEz2SelectStyle","HelpText")
#define TIMER_SECONDS			THEME->GetMetricI("ScreenEz2SelectStyle","TimerSeconds")
#define NEXT_SCREEN				THEME->GetMetric("ScreenEz2SelectStyle","NextScreen")
#define USE_METRIC_FOR_MENU		THEME->GetMetricI("ScreenEz2SelectStyle","UseMetricForMenu")
#define	SCROLLING_ELEMENT_SPACING THEME->GetMetricI("ScreenEz2SelectStyle","ScrollingElementSpacing")
#define	DISABLE_2P THEME->GetMetricI("ScreenEz2SelectStyle","Disable2p")
#define SCROLLING_LIST_Y THEME->GetMetricF("ScreenEz2SelectStyle","ScrollingListY")

const float TWEEN_TIME		= 0.35f;


/************************************
ScreenEz2SelectStyle (Constructor)
Desc: Sets up the screen display
************************************/

ScreenEz2SelectStyle::ScreenEz2SelectStyle()
{
	LOG->Trace( "ScreenEz2SelectStyle::ScreenEz2SelectStyle()" );

	
	GAMESTATE->m_PlayMode = PLAY_MODE_ARCADE;	// the only mode you can select on this screen
	
	/*********** TODO: MAKE THIS WORK FOR ALL GAME STYLES! *************/
	for (int i=0; i<=3; i++)
	{
		this->AddChild( &m_BGAnim[i] );	
		m_BGAnim[i].UpdateMetrics(i);
		m_BGAnim[i].SetZoom(0.0f); // hide the background
	}
	m_BGAnim[0].SetZoom(1.0f);

	m_ScrollingList.SetXY( CENTER_X, SCROLLING_LIST_Y );
	m_ScrollingList.SetSpacing( SCROLLING_ELEMENT_SPACING );
	m_ScrollingList.SetNumberVisible( 9 );
	this->AddChild( &m_ScrollingList );


	for( int p=0; p<NUM_PLAYERS; p++ )
	{
		if( GAMESTATE->m_bSideIsJoined[p] )	// if side is already joined
			continue;	// don't show bobbing join and blob

		m_sprControllers[p].Load( THEME->GetPathTo("Graphics","select player controller") );
		m_sprControllers[p].SetXY( CONTROLLER_X(p), CONTROLLER_Y(p) );
		this->AddChild( &m_sprControllers[p] );

		m_sprCursors[p].Load( THEME->GetPathTo("Graphics",ssprintf("select player cursor p%d",p+1)) );
		m_sprCursors[p].SetXY( CURSOR_X(p), CURSOR_Y(p) );
		m_sprCursors[p].SetEffectBouncing( D3DXVECTOR3(0,10,0), 0.5f );
		this->AddChild( &m_sprCursors[p] );
	}
	

	m_Menu.Load( 	
		THEME->GetPathTo("BGAnimations","select style"), 
		THEME->GetPathTo("Graphics","select style top edge"),
		HELP_TEXT, true, true, TIMER_SECONDS
		);
	this->AddChild( &m_Menu );

	m_soundSelect.Load( THEME->GetPathTo("Sounds","menu start") );
	m_soundChange.Load( THEME->GetPathTo("Sounds","select style change") );

	SOUND->PlayOnceStreamedFromDir( ANNOUNCER->GetPathTo("select style intro") );

	MUSIC->LoadAndPlayIfNotAlready( THEME->GetPathTo("Sounds","select style music") );

	if (USE_METRIC_FOR_MENU == 1) // get it from the metrics
	{
		RefreshStylesAndListFromMetrics();
	}
	else // get it from the styles......
	{
		RefreshStylesAndList();
	}

	TweenOnScreen();
	m_Menu.TweenOnScreenFromBlack( SM_None );

}

/************************************
~ScreenEz2SelectStyle (Destructor)
Desc: Writes line to log when screen
is terminated.
************************************/
ScreenEz2SelectStyle::~ScreenEz2SelectStyle()
{
	LOG->Trace( "ScreenEz2SelectStyle::~ScreenEz2SelectStyle()" );
}

/************************************
Update
Desc: Animates the 1p/2p selection
************************************/
void ScreenEz2SelectStyle::Update( float fDeltaTime )
{
	Screen::Update( fDeltaTime );
}

/************************************
DrawPrimitives
Desc: Draws the screen =P
************************************/

void ScreenEz2SelectStyle::DrawPrimitives()
{
	m_Menu.DrawBottomLayer();
	Screen::DrawPrimitives();
	m_Menu.DrawTopLayer();
}

/************************************
Input
Desc: Handles player input.
************************************/
void ScreenEz2SelectStyle::Input( const DeviceInput& DeviceI, const InputEventType type, const GameInput &GameI, const MenuInput &MenuI, const StyleInput &StyleI )
{
	LOG->Trace( "ScreenEz2SelectStyle::Input()" );

	if( m_Menu.IsClosing() )
		return;

	Screen::Input( DeviceI, type, GameI, MenuI, StyleI );	// default input handler
}

/************************************
HandleScreenMessage
Desc: Handles Screen Messages and changes
	game states.
************************************/
void ScreenEz2SelectStyle::HandleScreenMessage( const ScreenMessage SM )
{
	Screen::HandleScreenMessage( SM );

	switch( SM )
	{
	case SM_MenuTimer:
		{
			bool bAtLeastOneJoined = false;
			for( int p=0; p<NUM_PLAYERS; p++ )
				if( GAMESTATE->m_bSideIsJoined[p] )
					bAtLeastOneJoined = true;

			if( !bAtLeastOneJoined )
			{
				MenuStart(PLAYER_1);
				m_Menu.StopTimer();
			}
	
			TweenOffScreen();
			m_Menu.TweenOffScreenToMenu( SM_GoToNextScreen );
		}
		break;
	case SM_GoToPrevScreen:
		MUSIC->Stop();
		SCREENMAN->SetNewScreen( "ScreenTitleMenu" );
		break;
	case SM_GoToNextScreen:
		SCREENMAN->SetNewScreen( NEXT_SCREEN );
		break;
	}
}

void ScreenEz2SelectStyle::RefreshStylesAndListFromMetrics()
{
	int numstyles = THEME->GetMetricI("ScreenEz2SelectStyle","NumStyles"); // get the number of styles we need
	int numstyles2p = THEME->GetMetricI("ScreenEz2SelectStyle","NumStyles2p"); // get the number of styles we need

	int iNumSidesJoined = 0;
	for( int c=0; c<2; c++ )
		if( GAMESTATE->m_bSideIsJoined[c] )
			iNumSidesJoined++;	// left side, and right side

	CStringArray asGraphicPaths;	
	if( iNumSidesJoined == 2 ) // we got two players
	{		
		for (int i=0; i<numstyles2p; i++) // load in the graphics we want to use
		{
			asGraphicPaths.Add( THEME->GetPathTo("Graphics",ssprintf("%s",THEME->GetMetric("ScreenEz2SelectStyle",ssprintf("StyleGraphic2p%d",i) ) ) ) );
		}
	}
	else // else we got one player
	{
		for (int i=0; i<numstyles; i++) // load in the graphics we want to use
		{
			asGraphicPaths.Add( THEME->GetPathTo("Graphics",ssprintf("%s",THEME->GetMetric("ScreenEz2SelectStyle",ssprintf("StyleGraphic%d",i) ) ) ) );
		}
	}

	m_ScrollingList.Load( asGraphicPaths );
}

void ScreenEz2SelectStyle::RefreshStylesAndList()
{
	GAMEMAN->GetGameplayStylesForGame( GAMESTATE->m_CurGame, m_aPossibleStyles );
	ASSERT( m_aPossibleStyles.GetSize() > 0 );	// every game should have at least one Style, or else why have the Game? :-)

	// strip out Styles that don't work for the current number of players
	int iNumSidesJoined = 0;
	for( int c=0; c<2; c++ )
		if( GAMESTATE->m_bSideIsJoined[c] )
			iNumSidesJoined++;	// left side, and right side

	for( int i=m_aPossibleStyles.GetSize()-1; i>=0; i-- )
	{
		Style style = m_aPossibleStyles[i];

		switch( GAMEMAN->GetStyleDefForStyle(style)->m_StyleType )
		{
		case StyleDef::ONE_PLAYER_ONE_CREDIT: // if the current style is for 1player	
			if( iNumSidesJoined!=1 ) // and we have two (or more) players
				m_aPossibleStyles.RemoveAt( i ); // remove the element
			break;
		case StyleDef::ONE_PLAYER_TWO_CREDITS: // if the current style is for 1player on both sides
		case StyleDef::TWO_PLAYERS_TWO_CREDITS: // if the current style for 2players
			if( iNumSidesJoined!=2 ) // and we don't have two players
				m_aPossibleStyles.RemoveAt( i ); // remove the element
			break;
		default:	ASSERT(0);
		}
	}

	CStringArray asGraphicPaths;

	for( i=0; i<m_aPossibleStyles.GetSize(); i++ )
	{
		Style style = m_aPossibleStyles[i];
		CString sGameName = GAMESTATE->GetCurrentGameDef()->m_szName;
		CString sStyleName = GAMEMAN->GetStyleDefForStyle(style)->m_szName;

		asGraphicPaths.Add( THEME->GetPathTo("Graphics",ssprintf("select style info %s %s",sGameName,sStyleName)) );
	}
	m_ScrollingList.Load( asGraphicPaths );
}

/************************************
MenuBack
Desc: Actions performed when a player 
presses the button bound to back
************************************/
void ScreenEz2SelectStyle::MenuBack( PlayerNumber pn )
{
	MUSIC->Stop();

	m_Menu.TweenOffScreenToBlack( SM_GoToPrevScreen, true );
}



void ScreenEz2SelectStyle::MenuLeft( PlayerNumber pn )
{
	m_BGAnim[m_ScrollingList.GetSelection()].SetZoom(0.0f); // hide old background
	m_ScrollingList.Left();
	m_soundChange.Play();
	m_BGAnim[m_ScrollingList.GetSelection()].SetZoom(1.0f); // show new background

//	m_BGAnim.UpdateMetrics(m_ScrollingList.GetSelection());
}

void ScreenEz2SelectStyle::MenuRight( PlayerNumber pn )
{
	m_BGAnim[m_ScrollingList.GetSelection()].SetZoom(0.0f); // hide old background
	m_ScrollingList.Right();
	m_soundChange.Play();
	m_BGAnim[m_ScrollingList.GetSelection()].SetZoom(1.0f); // show new background

//	m_BGAnim.UpdateMetrics(m_ScrollingList.GetSelection());
}

/************************************
MenuDown
Desc: Actions performed when a player 
presses the button bound to down
************************************/
void ScreenEz2SelectStyle::MenuDown( PlayerNumber pn )
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
void ScreenEz2SelectStyle::MenuStart( PlayerNumber pn )
{
	if( !GAMESTATE->m_bSideIsJoined[pn] && !DISABLE_2P )
	{
		// join them
		GAMESTATE->m_bSideIsJoined[pn] = true;
		SCREENMAN->RefreshCreditsMessages();
		m_soundSelect.Play();
		m_sprCursors[pn].BeginTweening( 0.25f );
		m_sprCursors[pn].SetTweenZoomY( 0 );
		m_sprControllers[pn].BeginTweening( 0.25f );
		m_sprControllers[pn].SetTweenZoomY( 0 );
		if (USE_METRIC_FOR_MENU == 1) // get it from the metrics
		{		
			RefreshStylesAndListFromMetrics();
		}
		else
		{
			RefreshStylesAndList();
		}

		m_ScrollingList.SetSelection( 0 );

		/********** TODO: MAKE IT WORK FOR ALL GAME TYPES! ************/
		for (int i=0; i<=3; i++)
		{
			m_BGAnim[i].SetZoom(0.0f); // hide the background
		}
		m_BGAnim[0].SetZoom(1.0f);
	}
	else
	{
		// made a selection
		m_soundSelect.Play();

	if (USE_METRIC_FOR_MENU == 1) // get it from the metrics
	{
		int chosen=m_ScrollingList.GetSelection();
		int iNumSidesJoined = 0;
		for( int c=0; c<2; c++ )
			if( GAMESTATE->m_bSideIsJoined[c] )
				iNumSidesJoined++;	// left side, and right side

		GAMEMAN->GetGameplayStylesForGame( GAMESTATE->m_CurGame, m_aPossibleStyles );
	
		if( iNumSidesJoined == 2 ) // load in the Style to use based upon the list for 2players
			GAMESTATE->m_CurStyle = m_aPossibleStyles[ THEME->GetMetricI("ScreenEz2SelectStyle",ssprintf("UseStyle2p%d",chosen) ) ];
		else	// load in the Style to use based upon the list for 1player
			GAMESTATE->m_CurStyle = m_aPossibleStyles[ THEME->GetMetricI("ScreenEz2SelectStyle",ssprintf("UseStyle%d",chosen) ) ];


		int m_diSelection;
		
		if( iNumSidesJoined == 2 )
			m_diSelection = THEME->GetMetricI("ScreenEz2SelectStyle",ssprintf("UseDifficulty2p%d",chosen) );
		else
			m_diSelection = THEME->GetMetricI("ScreenEz2SelectStyle",ssprintf("UseDifficulty%d",chosen) );

		switch( m_diSelection )
		{
			case 0:	
				GAMESTATE->m_PreferredDifficultyClass[PLAYER_1] = CLASS_EASY;		
				GAMESTATE->m_PreferredDifficultyClass[PLAYER_2] = CLASS_EASY;					
				break;
			case 1:	
				GAMESTATE->m_PreferredDifficultyClass[PLAYER_1] = CLASS_MEDIUM;	
				GAMESTATE->m_PreferredDifficultyClass[PLAYER_2] = CLASS_MEDIUM;	
				break;
			case 2:	
				GAMESTATE->m_PreferredDifficultyClass[PLAYER_1] = CLASS_HARD;	
				GAMESTATE->m_PreferredDifficultyClass[PLAYER_2] = CLASS_HARD;	
				break;
		}		

	}
	else // get it from the scrolling list
	{
		GAMESTATE->m_CurStyle = m_aPossibleStyles[m_ScrollingList.GetSelection()];
	}
		TweenOffScreen();
		m_Menu.TweenOffScreenToMenu( SM_GoToNextScreen );
	}
}

void ScreenEz2SelectStyle::TweenOnScreen()
{
	float fOriginalZoomY = m_ScrollingList.GetZoomY();
	m_ScrollingList.BeginTweening( 0.5f );
	m_ScrollingList.SetTweenZoomY( fOriginalZoomY );

	for( int p=0; p<NUM_PLAYERS; p++ )
	{
		float fOffScreenOffset = float( (p==PLAYER_1) ? -SCREEN_WIDTH/2 : +SCREEN_WIDTH/2 );

		float fOriginalX;
		
		fOriginalX = m_sprCursors[p].GetX();
		m_sprCursors[p].SetX( m_sprCursors[p].GetX()+fOffScreenOffset );
		m_sprCursors[p].BeginTweening( 0.5f, Actor::TWEEN_BOUNCE_END );
		m_sprCursors[p].SetTweenX( fOriginalX );

		fOriginalX = m_sprControllers[p].GetX();
		m_sprControllers[p].SetX( m_sprCursors[p].GetX()+fOffScreenOffset );
		m_sprControllers[p].BeginTweening( 0.5f, Actor::TWEEN_BOUNCE_END );
		m_sprControllers[p].SetTweenX( fOriginalX );
	}
}

void ScreenEz2SelectStyle::TweenOffScreen()
{
	m_ScrollingList.BeginTweening( 0.5f );
	m_ScrollingList.SetTweenZoomY( 0 );

	for( int p=0; p<NUM_PLAYERS; p++ )
	{
		float fOffScreenOffset = float( (p==PLAYER_1) ? -SCREEN_WIDTH : +SCREEN_WIDTH );

		m_sprCursors[p].BeginTweening( 0.5f, Actor::TWEEN_BIAS_END );
		m_sprCursors[p].SetTweenX( m_sprCursors[p].GetX()+fOffScreenOffset );
		m_sprControllers[p].BeginTweening( 0.5f, Actor::TWEEN_BIAS_END );
		m_sprControllers[p].SetTweenX( m_sprCursors[p].GetX()+fOffScreenOffset );
	}
}
