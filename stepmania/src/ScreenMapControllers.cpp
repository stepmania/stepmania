#include "global.h"
#include "ScreenMapControllers.h"
#include "PrefsManager.h"
#include "ScreenManager.h"
#include "GameConstantsAndTypes.h"
#include "PrefsManager.h"
#include "RageLog.h"
#include "InputMapper.h"
#include "GameManager.h"
#include "GameState.h"
#include "GameSoundManager.h"
#include "ThemeManager.h"
#include "RageDisplay.h"
#include "Game.h"
#include "HelpDisplay.h"


#define EVEN_LINE_IN		THEME->GetMetric("ScreenMapControllers","EvenLineIn")
#define EVEN_LINE_OUT		THEME->GetMetric("ScreenMapControllers","EvenLineOut")
#define ODD_LINE_IN			THEME->GetMetric("ScreenMapControllers","OddLineIn")
#define ODD_LINE_OUT		THEME->GetMetric("ScreenMapControllers","OddLineOut")

const int FramesToWaitForInput = 2;

// reserve the 3rd slot for hard-coded keys
const int NUM_CHANGABLE_SLOTS = NUM_GAME_TO_DEVICE_SLOTS-1;


const float LINE_START_Y	=	64;
const float LINE_GAP_Y		=	28;
const float BUTTON_COLUMN_X[NUM_GAME_TO_DEVICE_SLOTS*MAX_GAME_CONTROLLERS] =
{
	50, 125, 200, 440, 515, 590 
};


ScreenMapControllers::ScreenMapControllers( CString sClassName ) : ScreenWithMenuElements( sClassName )
{
	LOG->Trace( "ScreenMapControllers::ScreenMapControllers()" );

#ifdef _XBOX
	CStringArray strArray;
	CString text("Use joypad to navigate, START to assign, A, B, X or Y to clear, BACK when done.");
	strArray.push_back(text);
	m_textHelp->SetTips(strArray);
#endif

	for( int b=0; b<GAMESTATE->GetCurrentGame()->m_iButtonsPerController; b++ )
	{
		CString sName = GAMESTATE->GetCurrentGame()->m_szButtonNames[b];
		CString sSecondary = GAMESTATE->GetCurrentGame()->m_szSecondaryFunction[b];

		m_textName[b].LoadFromFont( THEME->GetPathToF("Common title") );
		m_textName[b].SetXY( CENTER_X, -6 );
		m_textName[b].SetText( sName );
		m_textName[b].SetZoom( 0.7f );
		m_textName[b].SetShadowLength( 2 );
		m_Line[b].AddChild( &m_textName[b] );

		m_textName2[b].LoadFromFont( THEME->GetPathToF("Common title") );
		m_textName2[b].SetXY( CENTER_X, +6 );
		m_textName2[b].SetText( sSecondary );
		m_textName2[b].SetZoom( 0.5f );
		m_textName2[b].SetShadowLength( 2 );
		m_Line[b].AddChild( &m_textName2[b] );

		for( int p=0; p<MAX_GAME_CONTROLLERS; p++ ) 
		{			
			for( int s=0; s<NUM_GAME_TO_DEVICE_SLOTS; s++ ) 
			{
				m_textMappedTo[p][b][s].LoadFromFont( THEME->GetPathToF("ScreenMapControllers entry") );
				m_textMappedTo[p][b][s].SetXY( BUTTON_COLUMN_X[p*NUM_GAME_TO_DEVICE_SLOTS+s], 0 );
				m_textMappedTo[p][b][s].SetZoom( 0.5f );
				m_textMappedTo[p][b][s].SetShadowLength( 0 );
				m_Line[b].AddChild( &m_textMappedTo[p][b][s] );
			}
		}
		m_Line[b].SetY( LINE_START_Y + b*LINE_GAP_Y );
		this->AddChild( &m_Line[b] );

		m_Line[b].Command( (b%2)? ODD_LINE_IN:EVEN_LINE_IN );
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

	m_iWaitingForPress = 0;

	SOUND->PlayMusic( THEME->GetPathToS("ScreenMapControllers music") );

	Refresh();
}



ScreenMapControllers::~ScreenMapControllers()
{
	LOG->Trace( "ScreenMapControllers::~ScreenMapControllers()" );
}


void ScreenMapControllers::Update( float fDeltaTime )
{
	Screen::Update( fDeltaTime );

	
	if( m_iWaitingForPress  &&  m_DeviceIToMap.IsValid() )	// we're going to map an input
	{	
		--m_iWaitingForPress;
		if( m_iWaitingForPress )
			return; /* keep waiting */

		GameInput curGameI( (GameController)m_iCurController,
							(GameButton)m_iCurButton );

		INPUTMAPPER->SetInputMap( m_DeviceIToMap, curGameI, m_iCurSlot );
		INPUTMAPPER->AddDefaultMappingsForCurrentGameIfUnmapped();
		// commit to disk so we don't lose the changes!
		INPUTMAPPER->SaveMappingsToDisk();

		Refresh();
	}
}


void ScreenMapControllers::DrawPrimitives()
{
	Screen::DrawPrimitives();
}

static bool IsAxis( const DeviceInput& DeviceI )
{
	if( !DeviceI.IsJoystick() )
		return false;

	static int axes[] = 
	{
		JOY_LEFT, JOY_RIGHT, JOY_UP, JOY_DOWN,
		JOY_LEFT_2, JOY_RIGHT_2, JOY_UP_2, JOY_DOWN_2,
		JOY_Z_UP, JOY_Z_DOWN,
		JOY_ROT_UP, JOY_ROT_DOWN, JOY_ROT_LEFT, JOY_ROT_RIGHT, JOY_ROT_Z_UP, JOY_ROT_Z_DOWN,
		JOY_HAT_LEFT, JOY_HAT_RIGHT, JOY_HAT_UP, JOY_HAT_DOWN, 
		JOY_AUX_1, JOY_AUX_2, JOY_AUX_3, JOY_AUX_4,
		-1
	};

	for( int ax = 0; axes[ax] != -1; ++ax )
		if( DeviceI.button == axes[ax] )
			return true;

	return false;
}

void ScreenMapControllers::Input( const DeviceInput& DeviceI, const InputEventType type, const GameInput &GameI, const MenuInput &MenuI, const StyleInput &StyleI )
{
	if( type != IET_FIRST_PRESS && type != IET_SLOW_REPEAT )
		return;	// ignore

	LOG->Trace( "ScreenMapControllers::Input():  device: %d, button: %d", 
		DeviceI.device, DeviceI.button );

	int button = DeviceI.button;

#ifdef _XBOX
	if(!m_iWaitingForPress && DeviceI.device == DEVICE_JOY1)
	{
		// map the xbox controller buttons to the keyboard equivalents
		if(DeviceI.button == JOY_HAT_LEFT)
			button = KEY_LEFT;
		else if(DeviceI.button == JOY_HAT_RIGHT)
			button = KEY_RIGHT;
		else if(DeviceI.button == JOY_HAT_UP)
			button = KEY_UP;
		else if(DeviceI.button == JOY_HAT_DOWN)
			button = KEY_DOWN;
		else if(DeviceI.button == JOY_9)
			button = KEY_ENTER;
		else if(DeviceI.button == JOY_10)
			button = KEY_ESC;
		else if(DeviceI.button == JOY_1 || DeviceI.button == JOY_2 ||
				DeviceI.button == JOY_3 || DeviceI.button == JOY_4)
			button = KEY_DEL;
	}
#endif

	//
	// TRICKY:  This eliminates the need for a separate "ignore joy axes"
	// preference.  Some adapters map the PlayStation digital d-pad to
	// both axes and buttons.  We want buttons to be used for 
	// any mappings where possible because presses of buttons aren't mutually 
	// exclusive and presses of axes are (e.g. can't read presses of both Left and 
	// Right simultaneously).  So, when the user presses a button, we'll wait
	// until the next Update before adding a mapping so that we get a chance 
	// to see all input events the user's press of a panel.  This screen will be
	// receive input events for joystick axes presses first, then the input events 
	// for button presses.  We'll use the last input event received in the same 
	// Update so that a button presses are favored for mapping over axis presses.
	//

	/* We can't do that: it assumes that button presses are always received after
	 * corresponding axis events.  We need to check and explicitly prefer non-axis events
	 * over axis events. */
	if( m_iWaitingForPress )
	{
		/* Don't allow function keys to be mapped. */
		if ( DeviceI.device == DEVICE_KEYBOARD && (DeviceI.button >= KEY_F1 && DeviceI.button <= KEY_F12) )
		{
			m_textError.SetText( "That key can not be mapped." );
			SCREENMAN->PlayInvalidSound();
			m_textError.StopTweening();
			m_textError.SetDiffuse( RageColor(0,1,0,1) );
			m_textError.BeginTweening( 3 );
			m_textError.BeginTweening( 1 );
			m_textError.SetDiffuse( RageColor(0,1,0,0) );
		}
		else
		{
			if( m_DeviceIToMap.IsValid() &&
				!IsAxis(m_DeviceIToMap) &&
				IsAxis(DeviceI) )
			{
				LOG->Trace("Ignored input; non-axis event already received");
				return;	// ignore this press
			}

			m_DeviceIToMap = DeviceI;
		}
	}
#ifdef _XBOX
	else if( DeviceI.device == DEVICE_JOY1 )
#else
	else if( DeviceI.device == DEVICE_KEYBOARD )
#endif
	{
		switch( button )
		{
		/* We only advertise space as doing this, but most games
		 * use either backspace or delete, and I find them more
		 * intuitive, so allow them, too. -gm */

		/* XXX: For some reason that eludes me, this function gets sent an
		 * KEY_SPACE button press every time the JOY_HAT_UP button is pressed.
		 * Had to put this in to prevent mappings being erased everytime the user
		 * pressed up on the joypad. */

		case KEY_DEL:
#ifndef _XBOX
		case KEY_SPACE:
		case KEY_BACK: /* Clear the selected input mapping. */
#endif
			{
				GameInput curGameI( (GameController)m_iCurController, (GameButton)m_iCurButton );
				INPUTMAPPER->ClearFromInputMap( curGameI, m_iCurSlot );
				INPUTMAPPER->AddDefaultMappingsForCurrentGameIfUnmapped();
				// commit to disk so we don't lose the changes!
				INPUTMAPPER->SaveMappingsToDisk();
			}
			break;
		case KEY_LEFT: /* Move the selection left, wrapping up. */
			if( m_iCurSlot == 0 && m_iCurController == 0 )
				break;	// can't go left any more
			m_iCurSlot--;
			if( m_iCurSlot < 0 )
			{
				m_iCurSlot = NUM_CHANGABLE_SLOTS-1;
				m_iCurController--;
			}

			break;
		case KEY_RIGHT:	/* Move the selection right, wrapping down. */
			if( m_iCurSlot == NUM_CHANGABLE_SLOTS-1 && m_iCurController == MAX_GAME_CONTROLLERS-1 )
				break;	// can't go right any more
			m_iCurSlot++;
			if( m_iCurSlot > NUM_CHANGABLE_SLOTS-1 )
			{
				m_iCurSlot = 0;
				m_iCurController++;
			}
			break;
		case KEY_UP: /* Move the selection up. */
			if( m_iCurButton == 0 )
				break;	// can't go up any more
			m_iCurButton--;
			break;
		case KEY_DOWN: /* Move the selection down. */
			if( m_iCurButton == GAMESTATE->GetCurrentGame()->m_iButtonsPerController-1 )
				break;	// can't go down any more
			m_iCurButton++;
			break;
		case KEY_ESC: /* Quit the screen. */
			if(!IsTransitioning())
			{
				SCREENMAN->PlayStartSound();
				StartTransitioning( SM_GoToNextScreen );		
				for( int b=0; b<GAMESTATE->GetCurrentGame()->m_iButtonsPerController; b++ )
					m_Line[b].Command( (b%2)? ODD_LINE_OUT:EVEN_LINE_OUT );
			}
			break;
		case KEY_ENTER: /* Change the selection. */
		case KEY_KP_ENTER:
			m_iWaitingForPress = FramesToWaitForInput;
			m_DeviceIToMap.MakeInvalid();
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
		SCREENMAN->SetNewScreen( "ScreenOptionsMenu" );
		break;
	}
}

void ScreenMapControllers::Refresh()
{
	for( int p=0; p<MAX_GAME_CONTROLLERS; p++ ) 
	{			
		for( int b=0; b<GAMESTATE->GetCurrentGame()->m_iButtonsPerController; b++ ) 
		{
			for( int s=0; s<NUM_GAME_TO_DEVICE_SLOTS; s++ ) 
			{
				bool bSelected = p == m_iCurController  &&  b == m_iCurButton  &&  s == m_iCurSlot; 

				GameInput cur_gi( (GameController)p, (GameButton)b );
				DeviceInput di;
				if( INPUTMAPPER->GameToDevice( cur_gi, s, di ) )
					m_textMappedTo[p][b][s].SetText( di.GetDescription() );
				else
					m_textMappedTo[p][b][s].SetText( "-----------" );
				
				// highlight the currently selected pad button
				RageColor color;
				bool bPulse;
				if( bSelected ) 
				{
					if( m_iWaitingForPress )
					{
						color = RageColor(1,0.5,0.5,1);	// red
						bPulse = true;
					}
					else
					{
						color = RageColor(1,1,1,1);		// white
						bPulse = false;
					}
				} 
				else 
				{
					color = RageColor(0.5,0.5,0.5,1);	// gray
					bPulse = false;
				}
				m_textMappedTo[p][b][s].SetDiffuse( color );
				if( bPulse )
					m_textMappedTo[p][b][s].SetEffectPulse( .5f, .5f, .6f );
				else
					m_textMappedTo[p][b][s].SetEffectNone();
			}
		}
	}
}

/*
 * (c) 2001-2004 Chris Danford
 * All rights reserved.
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, and/or sell copies of the Software, and to permit persons to
 * whom the Software is furnished to do so, provided that the above
 * copyright notice(s) and this permission notice appear in all copies of
 * the Software and that both the above copyright notice(s) and this
 * permission notice appear in supporting documentation.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT OF
 * THIRD PARTY RIGHTS. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR HOLDERS
 * INCLUDED IN THIS NOTICE BE LIABLE FOR ANY CLAIM, OR ANY SPECIAL INDIRECT
 * OR CONSEQUENTIAL DAMAGES, OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS
 * OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR
 * OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 */
