#include "global.h"
#include "ScreenMapControllers.h"
#include "GameConstantsAndTypes.h"
#include "ScreenManager.h"
#include "RageLog.h"
#include "InputMapper.h"
#include "GameManager.h"
#include "GameState.h"
#include "ThemeManager.h"
#include "Game.h"
#include "ScreenDimensions.h"
#include "Command.h"
#include "InputEventPlus.h"

static const ThemeMetric<apActorCommands> EVEN_LINE_IN	("ScreenMapControllers","EvenLineIn");
static const ThemeMetric<apActorCommands> EVEN_LINE_OUT	("ScreenMapControllers","EvenLineOut");
static const ThemeMetric<apActorCommands> ODD_LINE_IN		("ScreenMapControllers","OddLineIn");
static const ThemeMetric<apActorCommands> ODD_LINE_OUT	("ScreenMapControllers","OddLineOut");

static const int FramesToWaitForInput = 2;

// reserve the 3rd slot for hard-coded keys
static const int NUM_CHANGABLE_SLOTS = NUM_SHOWN_GAME_TO_DEVICE_SLOTS-1;


static const float LINE_START_Y	=	64;
static const float LINE_GAP_Y		=	28;
static const float BUTTON_COLUMN_X[NUM_SHOWN_GAME_TO_DEVICE_SLOTS*MAX_GAME_CONTROLLERS] =
{
	50, 125, 200, 440, 515, 590 
};


REGISTER_SCREEN_CLASS( ScreenMapControllers );
ScreenMapControllers::ScreenMapControllers( CString sClassName ) : ScreenWithMenuElements( sClassName )
{
	LOG->Trace( "ScreenMapControllers::ScreenMapControllers()" );
}

void ScreenMapControllers::Init()
{
	ScreenWithMenuElements::Init();

	for( int b=0; b<GAMESTATE->GetCurrentGame()->m_iButtonsPerController; b++ )
	{
		KeyToMap k;
		k.m_sName = GAMESTATE->GetCurrentGame()->m_szButtonNames[b];
		k.m_sSecondary = GAMEMAN->GetMenuButtonSecondaryFunction( GAMESTATE->GetCurrentGame(), b );
		k.m_GameButton = (GameButton) b;
		m_KeysToMap.push_back( k );
	}

	for( unsigned b=0; b<m_KeysToMap.size(); b++ )
	{
		const KeyToMap *pKey = &m_KeysToMap[b];

		m_textName[b].SetName( "Title" );
		m_textName[b].LoadFromFont( THEME->GetPathF("Common","title") );
		m_textName[b].SetXY( SCREEN_CENTER_X, -6 );
		m_textName[b].SetText( pKey->m_sName );
		ON_COMMAND( m_textName[b] );
		m_Line[b].AddChild( &m_textName[b] );

		m_textName2[b].SetName( "Secondary" );
		m_textName2[b].LoadFromFont( THEME->GetPathF("Common","title") );
		m_textName2[b].SetXY( SCREEN_CENTER_X, +6 );
		m_textName2[b].SetText( pKey->m_sSecondary );
		ON_COMMAND( m_textName2[b] );
		m_Line[b].AddChild( &m_textName2[b] );

		for( int p=0; p<MAX_GAME_CONTROLLERS; p++ ) 
		{			
			for( int s=0; s<NUM_SHOWN_GAME_TO_DEVICE_SLOTS; s++ ) 
			{
				m_textMappedTo[b][p][s].SetName( "MappedTo" );
				m_textMappedTo[b][p][s].LoadFromFont( THEME->GetPathF("ScreenMapControllers","entry") );
				m_textMappedTo[b][p][s].SetXY( BUTTON_COLUMN_X[p*NUM_SHOWN_GAME_TO_DEVICE_SLOTS+s], 0 );
				ON_COMMAND( m_textMappedTo[b][p][s] );
				m_Line[b].AddChild( &m_textMappedTo[b][p][s] );
			}
		}
		m_Line[b].SetY( LINE_START_Y + b*LINE_GAP_Y );
		this->AddChild( &m_Line[b] );

		m_Line[b].RunCommands( (b%2)? ODD_LINE_IN : EVEN_LINE_IN );
	}	

	m_iCurController = 0;
	m_iCurButton = 0;
	m_iCurSlot = 0;

	m_iWaitingForPress = 0;

	Refresh();
}


void ScreenMapControllers::Update( float fDeltaTime )
{
	ScreenWithMenuElements::Update( fDeltaTime );

	
	if( m_iWaitingForPress  &&  m_DeviceIToMap.IsValid() )	// we're going to map an input
	{	
		--m_iWaitingForPress;
		if( m_iWaitingForPress )
			return; /* keep waiting */

		const KeyToMap *pKey = &m_KeysToMap[m_iCurButton];
		
		GameInput curGameI( (GameController)m_iCurController, pKey->m_GameButton );

		INPUTMAPPER->SetInputMap( m_DeviceIToMap, curGameI, m_iCurSlot );
		INPUTMAPPER->AddDefaultMappingsForCurrentGameIfUnmapped();

		// commit to disk after each change
		INPUTMAPPER->SaveMappingsToDisk();

		Refresh();
	}
}


/* Note that this isn't necessarily correct.  For example, JOY_LEFT might actually be
 * a D-pad and not an axis.  All this is actually doing is giving priority to some
 * inputs over others; this function is unsuitable for other use. */
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

void ScreenMapControllers::Input( const InputEventPlus &input )
{
	if( input.type != IET_FIRST_PRESS && input.type != IET_SLOW_REPEAT && input.type != IET_FAST_REPEAT )
		return;	// ignore

	LOG->Trace( "ScreenMapControllers::Input():  device: %d, button: %d", 
		input.DeviceI.device, input.DeviceI.button );

	int button = input.DeviceI.button;

#ifdef _XBOX
	if( !m_iWaitingForPress && input.DeviceI.device == DEVICE_JOY1 )
	{
		// map the xbox controller buttons to the keyboard equivalents
		if( input.DeviceI.button == JOY_HAT_LEFT )
			button = KEY_LEFT;
		else if( input.DeviceI.button == JOY_HAT_RIGHT )
			button = KEY_RIGHT;
		else if( input.DeviceI.button == JOY_HAT_UP )
			button = KEY_UP;
		else if( input.DeviceI.button == JOY_HAT_DOWN )
			button = KEY_DOWN;
		else if( input.DeviceI.button == JOY_AUX_1 )
			button = KEY_ENTER;
		else if( input.DeviceI.button == JOY_AUX_2 )
			button = KEY_ESC;
		else if( input.DeviceI.button == JOY_BUTTON_1 || input.DeviceI.button == JOY_BUTTON_2 ||
				input.DeviceI.button == JOY_BUTTON_3 || input.DeviceI.button == JOY_BUTTON_4 )
			button = KEY_DEL;
	}
#endif

	//
	// TRICKY:  Some adapters map the PlayStation digital d-pad to both axes and
	// buttons.  We want buttons to be used for any mappings where possible because
	// presses of buttons aren't mutually exclusive and presses of axes are (e.g.
	// can't read presses of both Left and Right simultaneously).  So, when the user
	// presses a button, we'll wait until the next Update before adding a mapping so
	// that we get a chance to see all input events the user's press of a panel.
	// Prefer non-axis events over axis events. 
	//
	if( m_iWaitingForPress )
	{
		/* Don't allow function keys to be mapped. */
		if( input.DeviceI.device == DEVICE_KEYBOARD && (input.DeviceI.button >= KEY_F1 && input.DeviceI.button <= KEY_F12) )
		{
			SCREENMAN->SystemMessage( "That key can not be mapped." );
			SCREENMAN->PlayInvalidSound();
		}
		else
		{
			if( m_DeviceIToMap.IsValid() &&
				!IsAxis(m_DeviceIToMap) &&
				IsAxis(input.DeviceI) )
			{
				LOG->Trace("Ignored input; non-axis event already received");
				return;	// ignore this press
			}

			m_DeviceIToMap = input.DeviceI;
		}
	}
#ifdef _XBOX
	else if( input.DeviceI.device == DEVICE_JOY1 )
#else
	else if( input.DeviceI.device == DEVICE_KEYBOARD )
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
				const KeyToMap *pKey = &m_KeysToMap[m_iCurButton];
				GameInput curGameI( (GameController)m_iCurController, pKey->m_GameButton );
				INPUTMAPPER->ClearFromInputMap( curGameI, m_iCurSlot );
				INPUTMAPPER->AddDefaultMappingsForCurrentGameIfUnmapped();
		
				// commit to disk after each change
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
			if( m_iCurButton == (int) m_KeysToMap.size()-1 )
				break;	// can't go down any more
			m_iCurButton++;
			break;
		case KEY_ESC: /* Quit the screen. */
			if( !IsTransitioning() )
			{
				SCREENMAN->PlayStartSound();

				INPUTMAPPER->SaveMappingsToDisk();	// save changes

				StartTransitioningScreen( SM_GoToNextScreen );		
			}
			break;
		case KEY_ENTER: /* Change the selection. */
		case KEY_KP_ENTER:
			m_iWaitingForPress = FramesToWaitForInput;
			m_DeviceIToMap.MakeInvalid();
			break;
		}
	}

//	ScreenWithMenuElements::Input( input );	// default handler

	LOG->Trace( "m_iCurSlot: %d m_iCurController: %d m_iCurButton: %d", m_iCurSlot, m_iCurController, m_iCurButton );

	Refresh();
}

void ScreenMapControllers::TweenOffScreen()
{
	for( unsigned b=0; b<m_KeysToMap.size(); b++ )
		m_Line[b].RunCommands( (b%2)? ODD_LINE_OUT:EVEN_LINE_OUT );
}

void ScreenMapControllers::Refresh()
{
	for( int p=0; p<MAX_GAME_CONTROLLERS; p++ ) 
	{			
		for( unsigned b=0; b<m_KeysToMap.size(); b++ )
		{
			for( int s=0; s<NUM_SHOWN_GAME_TO_DEVICE_SLOTS; s++ ) 
			{
				bool bSelected = p == m_iCurController  &&  (int) b == m_iCurButton  &&  s == m_iCurSlot; 

				BitmapText *pText = &m_textMappedTo[b][p][s];
				GameInput cur_gi( (GameController)p, (GameButton)b );
				DeviceInput di;
				if( INPUTMAPPER->GameToDevice( cur_gi, s, di ) )
					pText->SetText( di.toString() );
				else
					pText->SetText( "-----------" );
				
				if( bSelected ) 
				{
					if( m_iWaitingForPress )
						pText->PlayCommand( "Waiting" );
					else
						pText->PlayCommand( "Selected" );
				} 
				else 
					pText->PlayCommand( "Unselected" );
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
