#include "global.h"
/*
-----------------------------------------------------------------------------
 Class: ScreenMapControllers

 Desc: Where the player maps device input to pad input.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/

#include "ScreenMapControllers.h"
#include "PrefsManager.h"
#include "ScreenManager.h"
#include "GameConstantsAndTypes.h"
#include "PrefsManager.h"
#include "RageLog.h"
#include "InputMapper.h"
#include "GameManager.h"
#include "GameState.h"
#include "RageSoundManager.h"
#include "ThemeManager.h"


#define HELP_TEXT			THEME->GetMetric("ScreenMapControllers","HelpText")


// reserve the 3rd slot for hard-coded keys
const int NUM_CHANGABLE_SLOTS = NUM_GAME_TO_DEVICE_SLOTS-1;


const float TITLE_Y			=	30;
const float HELP_X			=	CENTER_X;
const float HELP_Y			=	SCREEN_HEIGHT-10;
const float LINE_START_Y	=	64;
const float LINE_GAP_Y		=	28;
const float BUTTON_COLUMN_X[NUM_GAME_TO_DEVICE_SLOTS*MAX_GAME_CONTROLLERS] =
{
	50, 125, 200, 440, 515, 590 
};


ScreenMapControllers::ScreenMapControllers() : Screen("ScreenMapControllers")
{
	LOG->Trace( "ScreenMapControllers::ScreenMapControllers()" );
	
	for( int b=0; b<GAMESTATE->GetCurrentGameDef()->m_iButtonsPerController; b++ )
	{
		CString sName = GAMESTATE->GetCurrentGameDef()->m_szButtonNames[b];
		CString sSecondary = GAMESTATE->GetCurrentGameDef()->m_szSecondaryFunction[b];

		m_textName[b].LoadFromFont( THEME->GetPathToF("Common title") );
		m_textName[b].SetXY( CENTER_X, LINE_START_Y + b*LINE_GAP_Y-6 );

		m_textName[b].SetText( sName );
		m_textName[b].SetZoom( 0.7f );
		m_textName[b].SetShadowLength( 2 );
		this->AddChild( &m_textName[b] );

		m_textName2[b].LoadFromFont( THEME->GetPathToF("Common title") );
		m_textName2[b].SetXY( CENTER_X, LINE_START_Y + b*LINE_GAP_Y+6 );
		m_textName2[b].SetText( sSecondary );
		m_textName2[b].SetZoom( 0.5f );
		m_textName2[b].SetShadowLength( 2 );
		this->AddChild( &m_textName2[b] );

		for( int p=0; p<MAX_GAME_CONTROLLERS; p++ ) 
		{			
			for( int s=0; s<NUM_GAME_TO_DEVICE_SLOTS; s++ ) 
			{
				m_textMappedTo[p][b][s].LoadFromFont( THEME->GetPathToF("Common normal") );
				m_textMappedTo[p][b][s].SetXY( BUTTON_COLUMN_X[p*NUM_GAME_TO_DEVICE_SLOTS+s], LINE_START_Y + b*LINE_GAP_Y );
				m_textMappedTo[p][b][s].SetZoom( 0.5f );
				m_textMappedTo[p][b][s].SetShadowLength( 2 );
				this->AddChild( &m_textMappedTo[p][b][s] );
			}
		}
	}	

	m_textError.LoadFromFont( THEME->GetPathToF("Common normal") );
	m_textError.SetText( "" );
	m_textError.SetXY( CENTER_X, CENTER_Y );
	m_textError.SetDiffuse( RageColor(0,1,0,0) );
	m_textError.SetZoom( 0.8f );
	this->AddChild( &m_textError );


	m_iCurController = 0;
	m_iCurButton = 0;
	m_iCurSlot = 0;

	m_bWaitingForPress = false;

	m_Menu.Load( "ScreenMapControllers", false );	// no timer
	this->AddChild( &m_Menu );

	SOUNDMAN->PlayMusic( THEME->GetPathToS("ScreenMapControllers music") );

	Refresh();
}



ScreenMapControllers::~ScreenMapControllers()
{
	LOG->Trace( "ScreenMapControllers::~ScreenMapControllers()" );
}


void ScreenMapControllers::Update( float fDeltaTime )
{
	Screen::Update( fDeltaTime );

	for( int b=0; b<GAMESTATE->GetCurrentGameDef()->m_iButtonsPerController; b++ )
	{
		CString sButtonName = GAMESTATE->GetCurrentGameDef()->m_szButtonNames[b];

		if( sButtonName == "Start"  ||  sButtonName == "Back" )
			continue;
	}
}


void ScreenMapControllers::DrawPrimitives()
{
	m_Menu.DrawBottomLayer();
	Screen::DrawPrimitives();
	m_Menu.DrawTopLayer();
}

void ScreenMapControllers::Input( const DeviceInput& DeviceI, const InputEventType type, const GameInput &GameI, const MenuInput &MenuI, const StyleInput &StyleI )
{
	if( type != IET_FIRST_PRESS && type != IET_SLOW_REPEAT )
		return;	// ignore

	LOG->Trace( "ScreenMapControllers::Input():  device: %d, button: %d", 
		DeviceI.device, DeviceI.button );

	if( m_bWaitingForPress )	// we're going to map an input
	{	
		/* Don't allow function keys to be mapped. */
		if ( DeviceI.device == DEVICE_KEYBOARD && (DeviceI.button >= SDLK_F1 && DeviceI.button <= SDLK_F12) )
		{
			m_textError.SetText( "That key can not be mapped." );
			SOUNDMAN->PlayOnce( THEME->GetPathToS("Common invalid" ) );
			m_textError.StopTweening();
			m_textError.SetDiffuse( RageColor(0,1,0,1) );
			m_textError.BeginTweening( 3 );
			m_textError.BeginTweening( 1 );
			m_textError.SetDiffuse( RageColor(0,1,0,0) );
			return;
		}

		// ignore joystick D-Pad presses if the user has set their pref.
		if( PREFSMAN->m_bIgnoreJoyAxes  &&
			DEVICE_JOY1 <= DeviceI.device  &&  DeviceI.device <= DEVICE_JOY4  &&
			( DeviceI.button == JOY_LEFT ||
			  DeviceI.button == JOY_RIGHT || 
			  DeviceI.button == JOY_UP || 
			  DeviceI.button == JOY_DOWN ||
			  DeviceI.button == JOY_ROT_UP ||
			  DeviceI.button == JOY_ROT_DOWN ||
			  DeviceI.button == JOY_ROT_LEFT ||
			  DeviceI.button == JOY_ROT_RIGHT ||
			  DeviceI.button == JOY_ROT_Z_UP ||
			  DeviceI.button == JOY_ROT_Z_DOWN ) )
		{
			//m_textError.SetText( "Game option is set to ignore the Joystick D-Pad." );
			//m_fErrorDisplayCountdown = 5;	// show the error message
			m_textError.StopTweening();
			m_textError.SetDiffuse( RageColor(0,1,0,1) );
			m_textError.BeginTweening( 3 );
			m_textError.BeginTweening( 1 );
			m_textError.SetDiffuse( RageColor(0,1,0,0) );

			return;	// ignore this press
		}

		GameInput curGameI( (GameController)m_iCurController,
							(GameButton)m_iCurButton );

		INPUTMAPPER->SetInputMap( DeviceI, curGameI, m_iCurSlot );
		INPUTMAPPER->AddDefaultMappingsForCurrentGameIfUnmapped();
		// commit to disk so we don't lose the changes!
		INPUTMAPPER->SaveMappingsToDisk();

		m_bWaitingForPress = false;
	}
	else if( DeviceI.device == DEVICE_KEYBOARD )
	{
		switch( DeviceI.button )
		{
		/* We only advertise space as doing this, but most games
		 * use either backspace or delete, and I find them more
		 * intuitive, so allow them, too. -gm */
		case SDLK_SPACE:
		case SDLK_DELETE:
		case SDLK_BACKSPACE: /* Clear the selected input mapping. */
			{
				GameInput curGameI( (GameController)m_iCurController, (GameButton)m_iCurButton );
				INPUTMAPPER->ClearFromInputMap( curGameI, m_iCurSlot );
				INPUTMAPPER->AddDefaultMappingsForCurrentGameIfUnmapped();
				// commit to disk so we don't lose the changes!
				INPUTMAPPER->SaveMappingsToDisk();
			}
			break;
		case SDLK_LEFT: /* Move the selection left, wrapping up. */
			if( m_iCurSlot == 0 && m_iCurController == 0 )
				break;	// can't go left any more
			m_iCurSlot--;
			if( m_iCurSlot < 0 )
			{
				m_iCurSlot = NUM_CHANGABLE_SLOTS-1;
				m_iCurController--;
			}
			break;
		case SDLK_RIGHT:	/* Move the selection right, wrapping down. */
			if( m_iCurSlot == NUM_CHANGABLE_SLOTS-1 && m_iCurController == MAX_GAME_CONTROLLERS-1 )
				break;	// can't go right any more
			m_iCurSlot++;
			if( m_iCurSlot > NUM_CHANGABLE_SLOTS-1 )
			{
				m_iCurSlot = 0;
				m_iCurController++;
			}
			break;
		case SDLK_UP: /* Move the selection up. */
			if( m_iCurButton == 0 )
				break;	// can't go up any more
			m_iCurButton--;
			break;
		case SDLK_DOWN: /* Move the selection down. */
			if( m_iCurButton == GAMESTATE->GetCurrentGameDef()->m_iButtonsPerController-1 )
				break;	// can't go down any more
			m_iCurButton++;
			break;
		case SDLK_ESCAPE: /* Quit the screen. */
			if(!m_Menu.IsTransitioning())
			{
				SOUNDMAN->PlayOnce( THEME->GetPathToS("Common start") );
				m_Menu.StartTransitioning( SM_GoToNextScreen );		
			}
			break;
		case SDLK_RETURN: /* Change the selection. */
			m_bWaitingForPress = true;	
			break;
		}
	}

//	Screen::Input( DeviceI, type, GameI, MenuI, StyleI );	// default handler

	LOG->Trace( "m_iCurSlot: %d m_iCurController: %d m_iCurButton: %d", m_iCurSlot, m_iCurController, m_iCurButton );

	Refresh();
}

void ScreenMapControllers::HandleScreenMessage( const ScreenMessage SM )
{
	switch( SM )
	{
	case SM_GoToNextScreen:
		//SCREENMAN->SetNewScreen( "ScreenTitleMenu" );
		/* At request, moved this into the options/operator menu -- Miryokuteki */
		SCREENMAN->SetNewScreen( "ScreenOptionsMenu" );
		break;
	}
}

void ScreenMapControllers::Refresh()
{
	for( int p=0; p<MAX_GAME_CONTROLLERS; p++ ) 
	{			
		for( int b=0; b<GAMESTATE->GetCurrentGameDef()->m_iButtonsPerController; b++ ) 
		{
			for( int s=0; s<NUM_GAME_TO_DEVICE_SLOTS; s++ ) 
			{
				GameInput cur_gi( (GameController)p, (GameButton)b );
				DeviceInput di;
				if( INPUTMAPPER->GameToDevice( cur_gi, s, di ) )
					m_textMappedTo[p][b][s].SetText( di.GetDescription() );
				else
					m_textMappedTo[p][b][s].SetText( "-----------" );
				
				// highlight the currently selected pad button
				RageColor color;
				if( p == m_iCurController  &&  b == m_iCurButton  &&  s == m_iCurSlot ) 
				{
					if( m_bWaitingForPress )
						color = RageColor(1,0.5,0.5,1);	// red
					else
						color = RageColor(1,1,1,1);		// white
				} 
				else 
					color = RageColor(0.5,0.5,0.5,1);	// gray
				m_textMappedTo[p][b][s].SetDiffuse( color );
			}
		}
	}
}
