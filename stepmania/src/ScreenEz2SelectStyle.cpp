#include "stdafx.h"
/****************************************
ScreenEzSelectPlayer,cpp
Desc: See Header
Copyright (C):
Andrew Livy

NOTES: Although cleaner, can still do with
a polish :)
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
#include "GameConstantsAndTypes.h"
#include "Background.h"
#include "GameState.h"
#include "RageException.h"
#include "RageTimer.h"

/* Constants */

#define HELP_TEXT				THEME->GetMetric("ScreenEz2SelectStyle","HelpText")
#define TIMER_SECONDS			THEME->GetMetricI("ScreenEz2SelectStyle","TimerSeconds")
#define NEXT_SCREEN				THEME->GetMetric("ScreenEz2SelectStyle","NextScreen")


const ScreenMessage SM_GoToPrevScreen		=	ScreenMessage(SM_User + 1);
const ScreenMessage SM_GoToNextScreen		=	ScreenMessage(SM_User + 2);


enum DStyles {
	DS_EASY,
	DS_HARD,
	DS_REAL,
	DS_CLUB
};

const float OPT_X[NUM_EZ2STYLE_GRAPHICS] = { 
	CENTER_X+200, // This is the pad X
	CENTER_X-200, // This is the pad X
	CENTER_X-198, // This is the 1p X
	CENTER_X+195, // This is the 2p X
}; // tells us the default X position
const float OPT_Y[NUM_EZ2STYLE_GRAPHICS] = {
	CENTER_Y+130,
	CENTER_Y+130,
	CENTER_Y+115,
	CENTER_Y+115,
}; // tells us the default Y position


const float TWEEN_TIME		= 0.35f;


float ez2p_lasttimercheck[2];
int ez2p_bounce=0; // used for the bouncing of the '1p' and '2p' images
int ez2p_direct=0; // direction of the bouncing of the '1p' and '2p' images

/************************************
ScreenEz2SelectStyle (Constructor)
Desc: Sets up the screen display
************************************/

ScreenEz2SelectStyle::ScreenEz2SelectStyle()
{
	LOG->Trace( "ScreenEz2SelectStyle::ScreenEz2SelectStyle()" );

	m_iSelectedStyle=DS_EASY; // start on EASY

	// Load in the sprites we will be working with.
	for( int i=0; i<NUM_EZ2STYLE_GRAPHICS; i++ )
	{
		m_sprBackground[i].Load( THEME->GetPathTo("Graphics",ssprintf("select style preview game %d style %d",GAMESTATE->m_CurGame,i)) );
		m_sprBackground[i].SetXY( CENTER_X, CENTER_Y );
		m_sprBackground[i].SetZoom( 1 );
		this->AddSubActor( &m_sprBackground[i] );
	}

	/* Setup The 1Player Scrolling List */
	m_ScrList.SetNumberVisibleElements( 3 );
	m_ScrList.CreateNewElement( "select style info game 2 style 1" );
	m_ScrList.CreateNewElement( "select style info game 2 style 2" );
	m_ScrList.CreateNewElement( "select style info game 2 style 3" );
	m_ScrList.CreateNewElement( "select style info game 2 style 0" ); // Excess so that the user is tricked into thinking
	m_ScrList.CreateNewElement( "select style info game 2 style 1" ); // the list is infinite
	m_ScrList.CreateNewElement( "select style info game 2 style 2" );
	m_ScrList.CreateNewElement( "select style info game 2 style 3" );
	m_ScrList.CreateNewElement( "select style info game 2 style 0" );
	m_ScrList.CreateNewElement( "select style info game 2 style 1" );
	m_ScrList.CreateNewElement( "select style info game 2 style 2" );
	m_ScrList.CreateNewElement( "select style info game 2 style 3" );
	m_ScrList.CreateNewElement( "select style info game 2 style 0" );
	m_ScrList.SetXY(CENTER_X, CENTER_Y);
	m_ScrList.SetCurrentPosition( DS_EASY );

	/* Setup the 2Player Scrolling List */
	m_ScrList_2ply.SetNumberVisibleElements( 3 );
	m_ScrList_2ply.CreateNewElement( "select style info game 2 style 1" );
	m_ScrList_2ply.CreateNewElement( "select style info game 2 style 2" );
	m_ScrList_2ply.CreateNewElement( "select style info game 2 style 3" ); // Excess so that the user is tricked into thinking
	m_ScrList_2ply.CreateNewElement( "select style info game 2 style 1" ); // the list is infinite
	m_ScrList_2ply.CreateNewElement( "select style info game 2 style 2" );
	m_ScrList_2ply.CreateNewElement( "select style info game 2 style 3" );
	m_ScrList_2ply.CreateNewElement( "select style info game 2 style 1" );
	m_ScrList_2ply.CreateNewElement( "select style info game 2 style 2" );
	m_ScrList_2ply.CreateNewElement( "select style info game 2 style 3" );
	m_ScrList_2ply.SetXY(CENTER_X, CENTER_Y);
	m_ScrList_2ply.SetCurrentPosition( DS_EASY );

	this->AddSubActor( &m_ScrList );
	this->AddSubActor( &m_ScrList_2ply );

	// figure out on load which list we should put up.
	if (GAMESTATE->m_CurStyle == STYLE_EZ2_SINGLE_VERSUS) // if we are using two players
	{
		m_ScrList.SetZoom( 0 ); // Hide the list for just 1 player 
	}
	else
	{
		m_ScrList_2ply.SetZoom( 0 ); // Otherwise hide the 2 player list.
	} 

// Load in the sprites we will be working with.
	for( i=0; i<NUM_EZ2STYLE_GRAPHICS; i++ )
	{
		CString sOptFileName;
		switch( i )
		{
		case 0:	sOptFileName = "select difficulty hard picture";	break;
		case 1:	sOptFileName = "select difficulty hard picture";	break;
		case 2:	sOptFileName = "select difficulty medium picture";	break;
		case 3:	sOptFileName = "select difficulty easy picture";	break;
		}
		m_sprOpt[i].Load( THEME->GetPathTo("Graphics",sOptFileName) );
		m_sprOpt[i].SetXY( OPT_X[i], OPT_Y[i] );
		this->AddSubActor( &m_sprOpt[i] );
	}

	m_Menu.Load( 	
		THEME->GetPathTo("Graphics","select style background"), 
		THEME->GetPathTo("Graphics","select style top edge"),
		HELP_TEXT, true, TIMER_SECONDS
		);
	this->AddSubActor( &m_Menu );

	m_soundSelect.Load( THEME->GetPathTo("Sounds","menu start") );

	GAMESTATE->m_bPlayersCanJoin = true;

	MUSIC->LoadAndPlayIfNotAlready( THEME->GetPathTo("Sounds","select style music") );

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
DrawPrimitives
Desc: Draws the screen =P
************************************/

void ScreenEz2SelectStyle::DrawPrimitives()
{
/*	if (m_iSelectedPlayer != 2) // no need to animate graphics if we have no graphics to animate ;)
	{
		AnimateGraphics();
	}
*/
	AnimateBackground();

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
		m_soundSelect.PlayRandom();
		GAMESTATE->m_PlayMode = PLAY_MODE_ARCADE;
		this->SendScreenMessage( SM_GoToNextScreen, 0 );
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


/************************************
MenuBack
Desc: Actions performed when a player 
presses the button bound to back
************************************/

void ScreenEz2SelectStyle::MenuBack( PlayerNumber p )
{
	MUSIC->Stop();

	m_Menu.TweenOffScreenToBlack( SM_GoToPrevScreen, true );
	GAMESTATE->m_CurStyle = STYLE_NONE; // Make sure that both players can scroll around title menu...

//	m_Fade.CloseWipingLeft( SM_GoToPrevScreen );

//	TweenOffScreen();
}

/************************************
MenuDown
Desc: Actions performed when a player 
presses the button bound to down
************************************/

void ScreenEz2SelectStyle::MenuDown( PlayerNumber p )
{
	MenuStart(p);
}


/************************************
SetFadedStyles
Desc: Fades out non-highlighted items
depending on the users choice.
************************************/
void ScreenEz2SelectStyle::SetFadedStyles()
{

}

/************************************
MenuRight
Desc: Actions performed when a player 
presses the button bound to right
************************************/
void ScreenEz2SelectStyle::MenuRight( PlayerNumber p )
{
	m_ScrList.ShiftRight();
	if (m_iSelectedStyle == 3) // wrap around
		m_iSelectedStyle = 0;
	else
		m_iSelectedStyle++;

	switch (m_iSelectedStyle)
	{
		case DS_EASY: GAMESTATE->m_CurStyle = STYLE_EZ2_SINGLE;	break;
		case DS_HARD: GAMESTATE->m_CurStyle = STYLE_EZ2_SINGLE_HARD; break;
		case DS_REAL: GAMESTATE->m_CurStyle = STYLE_EZ2_REAL; break;
		case DS_CLUB: GAMESTATE->m_CurStyle = STYLE_EZ2_DOUBLE; break;
	}
}

/************************************
MenuLeft
Desc: Actions performed when a player 
presses the button bound to left
************************************/
void ScreenEz2SelectStyle::MenuLeft( PlayerNumber p )
{
	m_ScrList.ShiftLeft();
	if (m_iSelectedStyle == 0) // wrap around
		m_iSelectedStyle = 3;
	else
		m_iSelectedStyle--;

	switch (m_iSelectedStyle)
	{
		case DS_EASY: GAMESTATE->m_CurStyle = STYLE_EZ2_SINGLE;	break;
		case DS_HARD: GAMESTATE->m_CurStyle = STYLE_EZ2_SINGLE_HARD; break;
		case DS_REAL: GAMESTATE->m_CurStyle = STYLE_EZ2_REAL; break;
		case DS_CLUB: GAMESTATE->m_CurStyle = STYLE_EZ2_DOUBLE; break;
	}
}

/************************************
MenuStart
Desc: Actions performed when a player 
presses the button bound to start
************************************/
void ScreenEz2SelectStyle::MenuStart( PlayerNumber p )
{

//	if( p!=PLAYER_INVALID  && !GAMESTATE->m_bIsJoined[p] )
//	{
//		SOUND->PlayOnceStreamed( THEME->GetPathTo("Sounds","menu start") );
//		GAMESTATE->m_bIsJoined[p] = true;
//		SCREENMAN->RefreshCreditsMessages();
//		m_soundSelect.PlayRandom();
//		return;	// don't fall through
//	}

	m_soundSelect.PlayRandom();
	this->ClearMessageQueue();
	GAMESTATE->m_PlayMode = PLAY_MODE_ARCADE;
//	GAMESTATE->m_bPlayersCanJoin = false;

	CString sCurStyleName = GAMESTATE->GetCurrentStyleDef()->m_szName;
	sCurStyleName.MakeLower();
	if(	     -1!=sCurStyleName.Find("single") )	SOUND->PlayOnceStreamedFromDir( ANNOUNCER->GetPathTo("select style comment single") );
	else if( -1!=sCurStyleName.Find("versus") )	SOUND->PlayOnceStreamedFromDir( ANNOUNCER->GetPathTo("select style comment versus") );
	else if( -1!=sCurStyleName.Find("double") )	SOUND->PlayOnceStreamedFromDir( ANNOUNCER->GetPathTo("select style comment double") );
	else if( -1!=sCurStyleName.Find("couple") )	SOUND->PlayOnceStreamedFromDir( ANNOUNCER->GetPathTo("select style comment couple") );
	else if( -1!=sCurStyleName.Find("solo") )	SOUND->PlayOnceStreamedFromDir( ANNOUNCER->GetPathTo("select style comment solo") );


	m_Menu.TweenOffScreenToMenu( SM_GoToNextScreen );
}

/************************************
TweenOffScreen
Desc: Squashes graphics before the screen
changes state.
************************************/
void ScreenEz2SelectStyle::TweenOffScreen()
{

}

/************************************
AnimateGraphics
Desc: Animates the 1p/2p selection
************************************/
void ScreenEz2SelectStyle::AnimateGraphics()
{

	//if (bounce < 10 && direct == 0 && wait == 2) // Bounce 1p/2p up
/*	if (TIMER->GetTimeSinceStart() > ez2p_lasttimercheck[0] + 0.01f && ez2p_direct == 0)
	{
		ez2p_lasttimercheck[0] = TIMER->GetTimeSinceStart();
		ez2p_bounce+=1;
	
		m_sprPly[2].SetXY( OPT_XP[2], OPT_YP[2] - ez2p_bounce);
		m_sprPly[3].SetXY( OPT_XP[3], OPT_YP[3] - ez2p_bounce);


		if (ez2p_bounce == 10)
		{
			ez2p_direct = 1;
		}
	}
	else if (TIMER->GetTimeSinceStart() > ez2p_lasttimercheck[0] + 0.01f && ez2p_direct == 1) // bounce 1p/2p down
	{
		ez2p_lasttimercheck[0] = TIMER->GetTimeSinceStart();
		ez2p_bounce-=1;

		m_sprPly[2].SetXY( OPT_XP[2], OPT_YP[2] - ez2p_bounce);
		m_sprPly[3].SetXY( OPT_XP[3], OPT_YP[3] - ez2p_bounce);

		if (ez2p_bounce == 0)
		{
			ez2p_direct = 0;
		}
	}
*/
}

/************************************
AnimateBackground
Desc: Animates the Background
************************************/
void ScreenEz2SelectStyle::AnimateBackground()
{
	if ((m_iSelectedStyle == 0) || (m_iSelectedPlayer == 2 && m_iSelectedStyle == 3)) // EASY background
	{
		m_sprBackground[3].SetHeight(SCREEN_HEIGHT * 1.7f);
		m_sprBackground[3].SetWidth(SCREEN_WIDTH * 1.7f);
		m_sprBackground[3].SetEffectSpinning(1.0f);
	}
	else if (m_iSelectedStyle == 3 && m_iSelectedPlayer != 2) // CLUB background
	{
		m_sprBackground[3].SetHeight(0);
		m_sprBackground[3].SetWidth(0);
		m_sprBackground[3].SetEffectNone();
		m_sprBackground[2].SetHeight(SCREEN_HEIGHT * 3.3f);
		m_sprBackground[2].SetWidth(SCREEN_WIDTH * 3.3f);
		m_sprBackground[2].SetEffectSpinning(0.5f);
		m_sprBackground[2].SetXY( CENTER_X, -250 );
	}
	else if (m_iSelectedStyle == 2) // REAL background
	{
		m_sprBackground[3].SetHeight(0);
		m_sprBackground[3].SetWidth(0);
		m_sprBackground[3].SetEffectNone();
		m_sprBackground[2].SetHeight(0);
		m_sprBackground[2].SetWidth(0);
		m_sprBackground[2].SetEffectNone();
		m_sprBackground[1].SetHeight(SCREEN_HEIGHT * 1.7f);
		m_sprBackground[1].SetWidth(SCREEN_WIDTH * 1.7f);
		m_sprBackground[1].SetEffectSpinning(2.1f);
	}
	else if (m_iSelectedStyle == 1) // HARD background
	{
		m_sprBackground[3].SetHeight(0);
		m_sprBackground[3].SetWidth(0);
		m_sprBackground[3].SetEffectNone();
		m_sprBackground[2].SetHeight(0);
		m_sprBackground[2].SetWidth(0);
		m_sprBackground[2].SetEffectNone();
		m_sprBackground[1].SetHeight(0);
		m_sprBackground[1].SetWidth(0);
		m_sprBackground[1].SetEffectNone();
		m_sprBackground[0].SetHeight(SCREEN_HEIGHT * 1.7f);
		m_sprBackground[0].SetWidth(SCREEN_WIDTH * 1.7f);
		m_sprBackground[0].SetEffectSpinning(1.0f);
	}

}

/************************************
Update
Desc: Animates the 1p/2p selection
************************************/
void ScreenEz2SelectStyle::Update( float fDeltaTime )
{
	Screen::Update( fDeltaTime );

	fDeltaTime /= .01f;

	ez2_bounce = fmod((ez2_bounce+fDeltaTime), 20);

	/* 0..10..19 -> 10..0..9 */
	int offset = abs(10-ez2_bounce);
	m_sprOpt[2].SetXY( OPT_X[2], OPT_Y[2] - offset);
	m_sprOpt[3].SetXY( OPT_X[3], OPT_Y[3] - offset);
}

/************************************
TweenPlyOffScreen
Desc: Squashes Player Graphics off screen
When selected.
************************************/
void ScreenEz2SelectStyle::TweenPlyOffScreen()
{

}