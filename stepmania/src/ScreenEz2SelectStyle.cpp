/****************************************
ScreenEzSelectPlayer,cpp
Desc: See Header
Copyright (C):
Andrew Livy

NOTES: Needs a good cleanup. Code is messy
and sloppy and err needs cleaning up ;)
*****************************************/

/* Includes */

#include "stdafx.h"
#include "ScreenEz2SelectStyle.h"
#include "ScreenManager.h"
#include "PrefsManager.h"
#include "RageMusic.h"
#include "ScreenTitleMenu.h"
#include "ScreenCaution.h"
#include "GameConstantsAndTypes.h"
#include "PrefsManager.h"
#include "ScreenSelectDifficulty.h"
#include "ScreenSandbox.h"
#include "GameManager.h"
#include "RageLog.h"
#include "AnnouncerManager.h"
#include "GameConstantsAndTypes.h"
#include "Background.h"
#include "ScreenSelectGroup.h"
#include "GameState.h"
#include "RageException.h"
#include "RageTimer.h"

/* Constants */

// const CString BG_ANIMS_DIR = "BGAnimations\\";

const ScreenMessage SM_GoToPrevState		=	ScreenMessage(SM_User + 1);
const ScreenMessage SM_GoToNextState		=	ScreenMessage(SM_User + 2);
/*
const CString DANCE_STYLES[] = {
	"easy",
	"hard",
	"real",
	"double",
};
*/
enum DStyles {
	DS_CLUB,
	DS_EASY,
	DS_HARD,
	DS_REAL

};

const float TWEEN_TIME		= 0.35f;
const D3DXCOLOR OPT_NOT_SELECTED = D3DXCOLOR(0.3f,0.3f,0.3f,1);
const D3DXCOLOR OPT_SELECTED = D3DXCOLOR(1.0f,1.0f,1.0f,1);


const float OPT_X[NUM_EZ2STYLE_GRAPHICS] = { 
	CENTER_X-350,
	CENTER_X,
	CENTER_X+350,
	CENTER_X+700,
}; // tells us the default X position
const float OPT_Y[NUM_EZ2STYLE_GRAPHICS] = {
	CENTER_Y,
	CENTER_Y,
	CENTER_Y,
	CENTER_Y,
}; // tells us the default Y position

const float CLUB_X[NUM_EZ2STYLE_GRAPHICS] = {
	OPT_X[0]+0,
	OPT_X[0]-350,
	OPT_X[2]+0,
	OPT_X[1]+0,
};

const float EASY_X[NUM_EZ2STYLE_GRAPHICS] = {
	OPT_X[1]+0,
	OPT_X[0]+0,
	OPT_X[0]-350,
	OPT_X[2]+0,
};

const float HARD_X[NUM_EZ2STYLE_GRAPHICS] = {
	OPT_X[2]+0,
	OPT_X[1]+0,
	OPT_X[0]+0,
	OPT_X[0]-350,
};

const float REAL_X[NUM_EZ2STYLE_GRAPHICS] = {
	OPT_X[3]+0,
	OPT_X[2]+0,
	OPT_X[1]+0,
	OPT_X[0]+0,
};

const float OPT_XP[NUM_EZ2P_GRAPHICS] = { 
	CENTER_X+200, // This is the pad X
	CENTER_X-200, // This is the pad X
	CENTER_X-198, // This is the 1p X
	CENTER_X+195, // This is the 2p X
}; // tells us the default X position
const float OPT_YP[NUM_EZ2P_GRAPHICS] = {
	CENTER_Y+130,
	CENTER_Y+130,
	CENTER_Y+115,
	CENTER_Y+115,
}; // tells us the default Y position


#define SKIP_SELECT_DIFFICULTY		THEME->GetMetricB("General","SkipSelectDifficulty")


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

	m_iSelectedStyle=0;
	ez2p_lasttimercheck[0] = TIMER->GetTimeSinceStart();

	// Load in the sprites we will be working with.
	for( int i=0; i<NUM_EZ2STYLE_GRAPHICS; i++ )
	{
		m_sprBackground[i].Load( THEME->GetPathTo("Graphics",ssprintf("select style preview game %d style %d",GAMESTATE->m_CurGame,i)) );
		m_sprBackground[i].SetXY( CENTER_X, CENTER_Y );
		m_sprBackground[i].SetZoom( 1 );
		this->AddSubActor( &m_sprBackground[i] );
	}

	for( i=0; i<NUM_EZ2STYLE_GRAPHICS; i++ )
	{
		m_sprOpt[i].Load( THEME->GetPathTo("Graphics",ssprintf("select style info game %d style %d",GAMESTATE->m_CurGame,i)) );
		m_sprOpt[i].SetXY( OPT_X[i], OPT_Y[i] );
		m_sprOpt[i].SetZoom( 1 );
		this->AddSubActor( &m_sprOpt[i] );
	}

	for( i=0; i<NUM_EZ2P_GRAPHICS; i++ )
	{
		CString sPlyFileName;
		switch( i )
		{
		case 0:	sPlyFileName = "select difficulty hard picture";	break;
		case 1:	sPlyFileName = "select difficulty hard picture";	break;
		case 2:	sPlyFileName = "select difficulty medium picture";	break;
		case 3:	sPlyFileName = "select difficulty easy picture";	break;
		}
		m_sprPly[i].Load( THEME->GetPathTo("Graphics",sPlyFileName) );
		m_sprPly[i].SetXY( OPT_XP[i], OPT_YP[i] );
		m_sprPly[i].SetZoom( 1 );
		this->AddSubActor( &m_sprPly[i] );
	}

	m_Menu.Load( 	
		THEME->GetPathTo("Graphics","select style background"), 
		THEME->GetPathTo("Graphics","select style top edge"),
		ssprintf("Use %c %c to select, then press START", char(1), char(2) ),
		false, true, 40 
		);
	this->AddSubActor( &m_Menu );

	m_soundChange.Load( THEME->GetPathTo("Sounds","select style change") );
	m_soundSelect.Load( THEME->GetPathTo("Sounds","menu start") );
	m_soundInvalid.Load( THEME->GetPathTo("Sounds","menu invalid") );

	/* Chris:  if EZ2 shouldn't play a sound here, then make a slient sound file.  */
	SOUND->PlayOnceStreamedFromDir( ANNOUNCER->GetPathTo(ANNOUNCER_SELECT_STYLE_INTRO) );


	if( !MUSIC->IsPlaying() )
	{
		MUSIC->Load( THEME->GetPathTo("Sounds","select style music") );
        MUSIC->Play( true );
	}

	/*
	TODO:  Get all the EZ2 specific stuff out of here
	*/

	if ( GAMESTATE->m_bSideIsJoined[PLAYER_1] && GAMESTATE->m_CurStyle == STYLE_EZ2_SINGLE) //if p1 already selected hide graphic.
	{
		m_iSelectedPlayer = 0;
		m_sprPly[1].BeginTweening( 0 );
		m_sprPly[1].SetTweenZoomY( 0 );
		m_sprPly[2].BeginTweening( 0 );
		m_sprPly[2].SetTweenZoomY( 0 );
	}
	else if ( GAMESTATE->m_bSideIsJoined[PLAYER_2] && GAMESTATE->m_CurStyle == STYLE_EZ2_SINGLE) //if p2 already selected hide graphic.
	{
		m_iSelectedPlayer = 1;
		m_sprPly[3].BeginTweening( 0 );
		m_sprPly[3].SetTweenZoomY( 0 );
		m_sprPly[0].BeginTweening( 0 );
		m_sprPly[0].SetTweenZoomY( 0 );
	}	
	else // if both are already selected, hide the graphics alltogether
	{
		m_iSelectedPlayer = 2;
		m_sprPly[1].BeginTweening( 0 );
		m_sprPly[1].SetTweenZoomY( 0 );
		m_sprPly[2].BeginTweening( 0 );
		m_sprPly[2].SetTweenZoomY( 0 );
		m_sprPly[3].BeginTweening( 0 );
		m_sprPly[3].SetTweenZoomY( 0 );
		m_sprPly[0].BeginTweening( 0 );
		m_sprPly[0].SetTweenZoomY( 0 );

		// hide the CLUB graphic...
		m_sprOpt[0].BeginTweening( 0 );
		m_sprOpt[0].SetTweenZoomY( 0 );
		m_iSelectedStyle = 1; // make sure we DONT have CLUB selected
		m_sprOpt[DS_EASY].SetXY( EASY_X[m_iSelectedStyle]-350, OPT_Y[0] );
		m_sprOpt[DS_HARD].SetXY( HARD_X[m_iSelectedStyle]-350, OPT_Y[0] );
		m_sprOpt[DS_REAL].SetXY( HARD_X[m_iSelectedStyle], OPT_Y[0] );	
		MenuLeft( PLAYER_1 ); // shift left so that we're clean again.

	}
	GAMESTATE->m_CurStyle = STYLE_NONE; // why reset this? because we want player2 to be able to input at this stage.
	
	
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
	if (m_iSelectedPlayer != 2) // no need to animate graphics if we have no graphics to animate ;)
	{
		AnimateGraphics();
	}
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
	
		if (m_iSelectedPlayer == 2) // both players
		{
			// m_soundInvalid.PlayRandom();
			// return;
			
			if (m_iSelectedStyle == 0) // easy
			{
			}
			else if (m_iSelectedStyle == 1) // hard
			{
			//	m_soundInvalid.PlayRandom();
			//	return;
				GAMESTATE->m_CurStyle = STYLE_EZ2_SINGLE_HARD_VERSUS;	
			}
			else if (m_iSelectedStyle == 2) // real
			{
				GAMESTATE->m_CurStyle = STYLE_EZ2_REAL_VERSUS;	
				//m_soundInvalid.PlayRandom();
				//return;
			}

		}
		else
		{
			if (m_iSelectedStyle == 0) // easy
			{
				GAMESTATE->m_CurStyle = STYLE_EZ2_SINGLE;	
			}
			else if (m_iSelectedStyle == 1) // hard
			{
				GAMESTATE->m_CurStyle = STYLE_EZ2_SINGLE_HARD;	
			}
			else if (m_iSelectedStyle == 2) // real
			{
				GAMESTATE->m_CurStyle = STYLE_EZ2_REAL;	
			}
			else // club
			{
				GAMESTATE->m_CurStyle = STYLE_EZ2_DOUBLE;
//				GAMESTATE->m_MasterPlayerNumber = PLAYER_1;
			}
		}

		m_soundSelect.PlayRandom();
		GAMESTATE->m_PlayMode = PLAY_MODE_ARCADE;
		SCREENMAN->SetNewScreen( new ScreenSelectGroup );

		break;
	case SM_GoToPrevState:
		MUSIC->Stop();
		SCREENMAN->SetNewScreen( new ScreenTitleMenu );
		break;
	case SM_GoToNextState:
		if( SKIP_SELECT_DIFFICULTY )
			SCREENMAN->SetNewScreen( new ScreenSelectGroup );
		else
			SCREENMAN->SetNewScreen( new ScreenSelectDifficulty );
		break;
	}
}


/************************************
MenuBack
Desc: Actions performed when a player 
presses the button bound to back
************************************/

void ScreenEz2SelectStyle::MenuBack( const PlayerNumber p )
{
	MUSIC->Stop();

	m_Menu.TweenOffScreenToBlack( SM_GoToPrevState, true );
	GAMESTATE->m_CurStyle = STYLE_NONE; // Make sure that both players can scroll around title menu...

//	m_Fade.CloseWipingLeft( SM_GoToPrevState );

//	TweenOffScreen();
}

/************************************
MenuDown
Desc: Actions performed when a player 
presses the button bound to down
************************************/

void ScreenEz2SelectStyle::MenuDown( const PlayerNumber p )
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
	m_sprOpt[0].SetTweenDiffuseColor( OPT_NOT_SELECTED );
	m_sprOpt[1].SetTweenDiffuseColor( OPT_NOT_SELECTED );
	m_sprOpt[2].SetTweenDiffuseColor( OPT_NOT_SELECTED );
	m_sprOpt[3].SetTweenDiffuseColor( OPT_NOT_SELECTED );
	if (m_iSelectedStyle != 3)
	{
		m_sprOpt[m_iSelectedStyle + 1].SetTweenDiffuseColor( OPT_SELECTED );
	}
	else
	{
		m_sprOpt[0].SetTweenDiffuseColor( OPT_SELECTED );
	}
	if (m_iSelectedPlayer == 2 && m_iSelectedStyle == DS_REAL)
	{
		m_sprOpt[1].SetTweenDiffuseColor( OPT_SELECTED );
	}
}

/************************************
MenuRight
Desc: Actions performed when a player 
presses the button bound to right
************************************/
void ScreenEz2SelectStyle::MenuRight( PlayerNumber p )
{
	if (m_iSelectedPlayer != 2) // Single player
	{
		if (((m_iSelectedPlayer == 0 && p == PLAYER_2) || (m_iSelectedPlayer == 1 && p == PLAYER_1)) != TRUE) // make sure players who haven't selected yet can't choose a style
		{
			if( m_iSelectedStyle == 3 )		// wrap to the first dance style
				m_iSelectedStyle = 0; // which is (club) easy (hard)
			else
				m_iSelectedStyle = m_iSelectedStyle + 1; // otherwise shuffle up a style...

			if(m_iSelectedStyle == 2) // If it's HARD and CLUB needs to appear from the other side...
			{
				m_sprOpt[0].SetXY( REAL_X[m_iSelectedStyle]+700, OPT_Y[0] ); // First move it over the other side off-screen...
			}
			else if(m_iSelectedStyle == 3) // If it's REAL and EASY needs to appear from the other side...
			{
				m_sprOpt[1].SetXY( CLUB_X[m_iSelectedStyle]+700, OPT_Y[0] ); // First move it over the other side off-screen...
			}
			else if(m_iSelectedStyle == 0) // If it's CLUB and HARD needs to appear from the other side...
			{
				m_sprOpt[2].SetXY( EASY_X[m_iSelectedStyle]+700, OPT_Y[0] ); // First move it over the other side off-screen...
				m_sprOpt[3].SetXY( EASY_X[m_iSelectedStyle]+1050, OPT_Y[0] ); // REAL must also move, due to the fact that it is the end item at the start, it moves differently to the rest.
			}

			m_sprOpt[0].BeginTweening( 0.2f, TWEEN_BIAS_BEGIN );
			m_sprOpt[0].SetTweenX( CLUB_X[m_iSelectedStyle] );
			m_sprOpt[0].SetTweenY( OPT_Y[m_iSelectedStyle] );


			m_sprOpt[1].BeginTweening( 0.2f, TWEEN_BIAS_BEGIN );
			m_sprOpt[1].SetTweenX( EASY_X[m_iSelectedStyle] );
			m_sprOpt[1].SetTweenY( OPT_Y[m_iSelectedStyle] );

			m_sprOpt[2].BeginTweening( 0.2f, TWEEN_BIAS_BEGIN );
			m_sprOpt[2].SetTweenX( HARD_X[m_iSelectedStyle] );
			m_sprOpt[2].SetTweenY( OPT_Y[m_iSelectedStyle] );

			m_sprOpt[3].BeginTweening( 0.2f, TWEEN_BIAS_BEGIN );
			m_sprOpt[3].SetTweenX( REAL_X[m_iSelectedStyle] );
			m_sprOpt[3].SetTweenY( OPT_Y[m_iSelectedStyle] );
			
			SetFadedStyles();
		}
	}
	else // Two Players (means NO club option...)
	{
		if (((m_iSelectedPlayer == 0 && p == PLAYER_2) || (m_iSelectedPlayer == 1 && p == PLAYER_1)) != TRUE) // make sure players who haven't selected yet can't choose a style
		{
			if( m_iSelectedStyle == DS_REAL )		// wrap to the last dance style
				m_iSelectedStyle = DS_EASY;
			else
				m_iSelectedStyle = m_iSelectedStyle + 1;

			if( m_iSelectedStyle == DS_REAL ) // (REALLY EASY)
			{
				m_sprOpt[DS_HARD].SetXY( CLUB_X[m_iSelectedStyle]+700, OPT_Y[0] ); // First move it over the other side off-screen...
				m_sprOpt[DS_REAL].SetXY( REAL_X[m_iSelectedStyle]+350, OPT_Y[0] );	
				m_sprOpt[DS_EASY].SetXY( CLUB_X[m_iSelectedStyle]+350, OPT_Y[0] );
			}
			else if( m_iSelectedStyle == DS_HARD ) // (REALLY REAL)
			{
				m_sprOpt[DS_EASY].SetXY( REAL_X[m_iSelectedStyle]+700, OPT_Y[0] ); // First move it over the other side off-screen...
				m_sprOpt[DS_HARD].SetXY( HARD_X[m_iSelectedStyle]+350, OPT_Y[0] );	
				m_sprOpt[DS_REAL].SetXY( REAL_X[m_iSelectedStyle]+350, OPT_Y[0] );					
			}
			else if( m_iSelectedStyle == DS_EASY ) // (REALLY HARD)
			{
				m_sprOpt[DS_REAL].SetXY( HARD_X[m_iSelectedStyle]+700, OPT_Y[0] ); // First move it over the other side off-screen...
				m_sprOpt[DS_EASY].SetXY( EASY_X[m_iSelectedStyle]+350, OPT_Y[0] );		
				m_sprOpt[DS_HARD].SetXY( HARD_X[m_iSelectedStyle]+350, OPT_Y[0] );					
			}


			/* NOTE: Because we're really shifting three values using a setup for four values
			   the DS_ values are shifted out of phase by 1, i.e. DS_REAL is now actually DS_EASY
			   Confused? I was =) Anyhow, this will only happen if there are two players and we don't want 
			   them playing double
			*/
			if (m_iSelectedStyle == DS_REAL)
			{
				m_sprOpt[DS_EASY].BeginTweening( 0.2f, TWEEN_BIAS_BEGIN );
				m_sprOpt[DS_EASY].SetTweenX( CLUB_X[m_iSelectedStyle] );
				m_sprOpt[DS_EASY].SetTweenY( OPT_Y[m_iSelectedStyle] );
				m_sprOpt[DS_HARD].BeginTweening( 0.2f, TWEEN_BIAS_BEGIN );
				m_sprOpt[DS_HARD].SetTweenX( CLUB_X[m_iSelectedStyle] + 350 );
				m_sprOpt[DS_HARD].SetTweenY( OPT_Y[m_iSelectedStyle] );
				m_sprOpt[DS_REAL].BeginTweening( 0.2f, TWEEN_BIAS_BEGIN );
				m_sprOpt[DS_REAL].SetTweenX( REAL_X[m_iSelectedStyle] );
				m_sprOpt[DS_REAL].SetTweenY( OPT_Y[m_iSelectedStyle] );	
			}	
			else if (m_iSelectedStyle == DS_HARD)
			{
				m_sprOpt[DS_EASY].BeginTweening( 0.2f, TWEEN_BIAS_BEGIN );
				m_sprOpt[DS_EASY].SetTweenX( REAL_X[m_iSelectedStyle] + 350 );
				m_sprOpt[DS_EASY].SetTweenY( OPT_Y[m_iSelectedStyle] );
				m_sprOpt[DS_HARD].BeginTweening( 0.2f, TWEEN_BIAS_BEGIN );
				m_sprOpt[DS_HARD].SetTweenX( HARD_X[m_iSelectedStyle] );
				m_sprOpt[DS_HARD].SetTweenY( OPT_Y[m_iSelectedStyle] );
				m_sprOpt[DS_REAL].BeginTweening( 0.2f, TWEEN_BIAS_BEGIN );
				m_sprOpt[DS_REAL].SetTweenX( REAL_X[m_iSelectedStyle] );
				m_sprOpt[DS_REAL].SetTweenY( OPT_Y[m_iSelectedStyle] );	
			}
			else if (m_iSelectedStyle == DS_EASY)
			{
				m_sprOpt[DS_EASY].BeginTweening( 0.2f, TWEEN_BIAS_BEGIN );
				m_sprOpt[DS_EASY].SetTweenX( EASY_X[m_iSelectedStyle] );
				m_sprOpt[DS_EASY].SetTweenY( OPT_Y[m_iSelectedStyle] );
				m_sprOpt[DS_HARD].BeginTweening( 0.2f, TWEEN_BIAS_BEGIN );
				m_sprOpt[DS_HARD].SetTweenX( HARD_X[m_iSelectedStyle] );
				m_sprOpt[DS_HARD].SetTweenY( OPT_Y[m_iSelectedStyle] );
				m_sprOpt[DS_REAL].BeginTweening( 0.2f, TWEEN_BIAS_BEGIN );
				m_sprOpt[DS_REAL].SetTweenX( HARD_X[m_iSelectedStyle] + 350 );
				m_sprOpt[DS_REAL].SetTweenY( OPT_Y[m_iSelectedStyle] );	
			}
		}
		SetFadedStyles();
	}
}

/************************************
MenuLeft
Desc: Actions performed when a player 
presses the button bound to left
************************************/
void ScreenEz2SelectStyle::MenuLeft( PlayerNumber p )
{
	if (m_iSelectedPlayer != 2) // Single player
	{
		if (((m_iSelectedPlayer == 0 && p == PLAYER_2) || (m_iSelectedPlayer == 1 && p == PLAYER_1)) != TRUE) // make sure players who haven't selected yet can't choose a style
		{
			if( m_iSelectedStyle == 0 )		// wrap to the last dance style
				m_iSelectedStyle = 3;
			else
				m_iSelectedStyle = m_iSelectedStyle - 1;

			if( m_iSelectedStyle == 3 )
			{
				m_sprOpt[3].SetXY( CLUB_X[m_iSelectedStyle]-700, OPT_Y[0] ); // First move it over the other side off-screen...		
				m_sprOpt[2].SetXY( CLUB_X[m_iSelectedStyle]-1050, OPT_Y[0] );	
			}
			else if( m_iSelectedStyle == 2 )
			{
				m_sprOpt[1].SetXY( HARD_X[m_iSelectedStyle]-700, OPT_Y[0] ); // First move it over the other side off-screen...			
			}
			else if( m_iSelectedStyle == 1 )
			{
				m_sprOpt[0].SetXY( EASY_X[m_iSelectedStyle]-1050, OPT_Y[0] ); // First move it over the other side off-screen...			
			}

			m_sprOpt[0].BeginTweening( 0.2f, TWEEN_BIAS_BEGIN );
			m_sprOpt[0].SetTweenX( CLUB_X[m_iSelectedStyle] );
			m_sprOpt[0].SetTweenY( OPT_Y[m_iSelectedStyle] );	

			m_sprOpt[1].BeginTweening( 0.2f, TWEEN_BIAS_BEGIN );
			m_sprOpt[1].SetTweenX( EASY_X[m_iSelectedStyle] );
			m_sprOpt[1].SetTweenY( OPT_Y[m_iSelectedStyle] );

			m_sprOpt[2].BeginTweening( 0.2f, TWEEN_BIAS_BEGIN );
			m_sprOpt[2].SetTweenX( HARD_X[m_iSelectedStyle] );
			m_sprOpt[2].SetTweenY( OPT_Y[m_iSelectedStyle] );

			m_sprOpt[3].BeginTweening( 0.2f, TWEEN_BIAS_BEGIN );
			m_sprOpt[3].SetTweenX( REAL_X[m_iSelectedStyle] );
			m_sprOpt[3].SetTweenY( OPT_Y[m_iSelectedStyle] );	

			SetFadedStyles();
		}
	}
	else // Two Players (means NO club option...)
	{
		if (((m_iSelectedPlayer == 0 && p == PLAYER_2) || (m_iSelectedPlayer == 1 && p == PLAYER_1)) != TRUE) // make sure players who haven't selected yet can't choose a style
		{
			if( m_iSelectedStyle == DS_EASY )		// wrap to the last dance style
				m_iSelectedStyle = DS_REAL;
			else
				m_iSelectedStyle = m_iSelectedStyle - 1;

			if( m_iSelectedStyle == DS_REAL ) // (REALLY EASY)
			{
				m_sprOpt[DS_HARD].SetXY( CLUB_X[m_iSelectedStyle], OPT_Y[0] ); // First move it over the other side off-screen...
				m_sprOpt[DS_REAL].SetXY( REAL_X[m_iSelectedStyle]-350, OPT_Y[0] );				
			}
			else if( m_iSelectedStyle == DS_HARD ) // (REALLY REAL)
			{
				m_sprOpt[DS_EASY].SetXY( REAL_X[m_iSelectedStyle], OPT_Y[0] ); // First move it over the other side off-screen...
				m_sprOpt[DS_HARD].SetXY( HARD_X[m_iSelectedStyle]-350, OPT_Y[0] );			
			}
			else if( m_iSelectedStyle == DS_EASY ) // (REALLY HARD)
			{
				m_sprOpt[DS_REAL].SetXY( HARD_X[m_iSelectedStyle], OPT_Y[0] ); // First move it over the other side off-screen...
				m_sprOpt[DS_EASY].SetXY( EASY_X[m_iSelectedStyle]-350, OPT_Y[0] );				
			}


			/* NOTE: Because we're really shifting three values using a setup for four values
			   the DS_ values are shifted out of phase by 1, i.e. DS_REAL is now actually DS_EASY
			   Confused? I was =) Anyhow, this will only happen if there are two players and we don't want 
			   them playing double
			*/
			if (m_iSelectedStyle == DS_REAL)
			{
				m_sprOpt[DS_EASY].BeginTweening( 0.2f, TWEEN_BIAS_BEGIN );
				m_sprOpt[DS_EASY].SetTweenX( CLUB_X[m_iSelectedStyle] );
				m_sprOpt[DS_EASY].SetTweenY( OPT_Y[m_iSelectedStyle] );
				m_sprOpt[DS_HARD].BeginTweening( 0.2f, TWEEN_BIAS_BEGIN );
				m_sprOpt[DS_HARD].SetTweenX( CLUB_X[m_iSelectedStyle] + 350 );
				m_sprOpt[DS_HARD].SetTweenY( OPT_Y[m_iSelectedStyle] );
				m_sprOpt[DS_REAL].BeginTweening( 0.2f, TWEEN_BIAS_BEGIN );
				m_sprOpt[DS_REAL].SetTweenX( REAL_X[m_iSelectedStyle] );
				m_sprOpt[DS_REAL].SetTweenY( OPT_Y[m_iSelectedStyle] );	
			}	
			else if (m_iSelectedStyle == DS_HARD)
			{
				m_sprOpt[DS_EASY].BeginTweening( 0.2f, TWEEN_BIAS_BEGIN );
				m_sprOpt[DS_EASY].SetTweenX( REAL_X[m_iSelectedStyle] + 350 );
				m_sprOpt[DS_EASY].SetTweenY( OPT_Y[m_iSelectedStyle] );
				m_sprOpt[DS_HARD].BeginTweening( 0.2f, TWEEN_BIAS_BEGIN );
				m_sprOpt[DS_HARD].SetTweenX( HARD_X[m_iSelectedStyle] );
				m_sprOpt[DS_HARD].SetTweenY( OPT_Y[m_iSelectedStyle] );
				m_sprOpt[DS_REAL].BeginTweening( 0.2f, TWEEN_BIAS_BEGIN );
				m_sprOpt[DS_REAL].SetTweenX( REAL_X[m_iSelectedStyle] );
				m_sprOpt[DS_REAL].SetTweenY( OPT_Y[m_iSelectedStyle] );	
			}
			else if (m_iSelectedStyle == DS_EASY)
			{
				m_sprOpt[DS_EASY].BeginTweening( 0.2f, TWEEN_BIAS_BEGIN );
				m_sprOpt[DS_EASY].SetTweenX( EASY_X[m_iSelectedStyle] );
				m_sprOpt[DS_EASY].SetTweenY( OPT_Y[m_iSelectedStyle] );
				m_sprOpt[DS_HARD].BeginTweening( 0.2f, TWEEN_BIAS_BEGIN );
				m_sprOpt[DS_HARD].SetTweenX( HARD_X[m_iSelectedStyle] );
				m_sprOpt[DS_HARD].SetTweenY( OPT_Y[m_iSelectedStyle] );
				m_sprOpt[DS_REAL].BeginTweening( 0.2f, TWEEN_BIAS_BEGIN );
				m_sprOpt[DS_REAL].SetTweenX( HARD_X[m_iSelectedStyle] + 350 );
				m_sprOpt[DS_REAL].SetTweenY( OPT_Y[m_iSelectedStyle] );	
			}


			SetFadedStyles();
		}
	}
}

/************************************
MenuStart
Desc: Actions performed when a player 
presses the button bound to start
************************************/
void ScreenEz2SelectStyle::MenuStart( PlayerNumber p )
{
//	GAMESTATE->m_CurStyle = DANCE_STYLES[m_iSelectedStyle];
	
	if ((m_iSelectedPlayer == 0 && p == PLAYER_2) || (m_iSelectedPlayer == 1 && p == PLAYER_1))
	{
		//if (p == PLAYER_2) // CURRENTLY PLAYER 2 IS BROKEN
		//{
		//	m_soundInvalid.PlayRandom();
		//	return;
		//}
		m_soundSelect.PlayRandom();
		TweenPlyOffScreen();
		m_iSelectedPlayer = 2; // set to BOTH players now.

		// hide the CLUB graphic...
		m_sprOpt[0].BeginTweening( 0 );
		m_sprOpt[0].SetTweenZoomY( 0 );
		m_iSelectedStyle = 1; // make sure we DONT have CLUB selected
		m_sprOpt[DS_EASY].SetXY( EASY_X[m_iSelectedStyle]-350, OPT_Y[0] );
		m_sprOpt[DS_HARD].SetXY( HARD_X[m_iSelectedStyle]-350, OPT_Y[0] );
		m_sprOpt[DS_REAL].SetXY( HARD_X[m_iSelectedStyle], OPT_Y[0] );	
		MenuLeft( p ); // shift left so that we're clean again.
	}
	else
	{
		if (m_iSelectedPlayer == 2) // both players
		{
			// m_soundInvalid.PlayRandom();
			// return;
			
			if (m_iSelectedStyle == 0 || m_iSelectedStyle == 3) // easy
			{
				GAMESTATE->m_CurStyle = STYLE_EZ2_SINGLE_VERSUS;	
			}
			else if (m_iSelectedStyle == 1) // hard
			{
				GAMESTATE->m_CurStyle = STYLE_EZ2_SINGLE_HARD_VERSUS;	
			}
			else if (m_iSelectedStyle == 2) // real
			{
				GAMESTATE->m_CurStyle = STYLE_EZ2_REAL_VERSUS;	
			}

		}
		else
		{
			if (m_iSelectedStyle == 0) // easy
			{
				GAMESTATE->m_CurStyle = STYLE_EZ2_SINGLE;	
			}
			else if (m_iSelectedStyle == 1) // hard
			{
				GAMESTATE->m_CurStyle = STYLE_EZ2_SINGLE_HARD;	
			}
			else if (m_iSelectedStyle == 2) // real
			{
				GAMESTATE->m_CurStyle = STYLE_EZ2_REAL;	
				//m_soundInvalid.PlayRandom();
				//return;
			}
			else // club
			{
				GAMESTATE->m_CurStyle = STYLE_EZ2_DOUBLE;
				GAMESTATE->m_CurStyle = STYLE_EZ2_DOUBLE;
//				GAMESTATE->m_MasterPlayerNumber = PLAYER_1;
				//m_soundInvalid.PlayRandom();
				//return;
			}
		}
		m_soundSelect.PlayRandom();
	
		this->ClearMessageQueue();
	
		GAMESTATE->m_PlayMode = PLAY_MODE_ARCADE;

		m_Menu.TweenOffScreenToMenu( SM_GoToNextState );

		TweenOffScreen();
	}

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
	if (TIMER->GetTimeSinceStart() > ez2p_lasttimercheck[0] + 0.01f && ez2p_direct == 0)
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
TweenPlyOffScreen
Desc: Squashes Player Graphics off screen
When selected.
************************************/
void ScreenEz2SelectStyle::TweenPlyOffScreen()
{
			m_sprPly[1].BeginTweening( MENU_ELEMENTS_TWEEN_TIME );
			m_sprPly[1].SetTweenZoomY( 0 );
			m_sprPly[2].BeginTweening( MENU_ELEMENTS_TWEEN_TIME );
			m_sprPly[2].SetTweenZoomY( 0 );
			m_sprPly[0].BeginTweening( MENU_ELEMENTS_TWEEN_TIME );
			m_sprPly[0].SetTweenZoomY( 0 );
			m_sprPly[3].BeginTweening( MENU_ELEMENTS_TWEEN_TIME );
			m_sprPly[3].SetTweenZoomY( 0 );
}