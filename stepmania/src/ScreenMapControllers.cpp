#include "global.h"
#include "ScreenMapControllers.h"
#include "ScreenManager.h"
#include "RageLog.h"
#include "RageInput.h"
#include "InputMapper.h"
#include "ThemeManager.h"
#include "ScreenDimensions.h"
#include "InputEventPlus.h"
#include "LocalizedString.h"

#define BUTTONS_TO_MAP			THEME->GetMetric ( m_sName, "ButtonsToMap" )
static LocalizedString INVALID_BUTTON   ( "ScreenMapControllers", "InvalidButton" );
#define MAPPED_TO_COMMAND(gc,slot)	THEME->GetMetricA( m_sName, ssprintf("MappedToP%iS%iCommand", gc+1, slot+1) )

static const float g_fSecondsToWaitForInput = 0.05f;

// reserve the 3rd slot for hard-coded keys
static const int NUM_CHANGABLE_SLOTS = NUM_SHOWN_GAME_TO_DEVICE_SLOTS-1;

REGISTER_SCREEN_CLASS( ScreenMapControllers );

ScreenMapControllers::ScreenMapControllers()
{
	this->SubscribeToMessage( Message_AutoJoyMappingApplied );
}

static LocalizedString PLAYER_SLOTS( "ScreenMapControllers", "%s slots" );
void ScreenMapControllers::Init()
{
	ScreenWithMenuElements::Init();

	m_soundChange.Load( THEME->GetPathS(m_sName,"change"), true );
	m_soundDelete.Load( THEME->GetPathS(m_sName,"delete"), true );


	m_textDevices.LoadFromFont( THEME->GetPathF("Common","normal") );
	m_textDevices.SetName( "Devices" );
	SET_XY_AND_ON_COMMAND( m_textDevices );
	this->AddChild( &m_textDevices );


	RString sButtons = BUTTONS_TO_MAP;
	if( sButtons.empty() )
	{
		/* Map all buttons for this game. */
		FOREACH_GameButtonInScheme( INPUTMAPPER->GetInputScheme(), gb )
		{
			KeyToMap k;
			k.m_GameButton = gb;
			m_KeysToMap.push_back( k );
		}
	}
	else
	{
		/* Map the specified buttons. */
		vector<RString> asBits;
		split( sButtons, ",", asBits );
		for( unsigned i=0; i<asBits.size(); ++i )
		{
			KeyToMap k;
			k.m_GameButton = StringToGameButton( INPUTMAPPER->GetInputScheme(), asBits[i] );
			m_KeysToMap.push_back( k );
		}
	}

	int iRow = 0;

	// header row
	{
		FOREACH_GameController( c )
		{			
			BitmapText &text = m_textLabel[c];
			text.LoadFromFont( THEME->GetPathF("Common","title") );
			PlayerNumber pn = (PlayerNumber)c;
			text.SetName( "Label"+PlayerNumberToString(pn) );
			RString sText = ssprintf(PLAYER_SLOTS.GetValue(), PlayerNumberToLocalizedString(pn).c_str());
			text.SetText( sText );
			ActorUtil::LoadAllCommands( text, m_sName );
			m_Line[iRow].AddChild( &m_textLabel[c] );
		}
		m_LineScroller.AddChild( &m_Line[iRow] );

		iRow++;
	}

	// normal rows
	for( unsigned b=0; b<m_KeysToMap.size(); b++ )
	{
		KeyToMap *pKey = &m_KeysToMap[b];

		{
			BitmapText *pName = new BitmapText;
			pName->SetName( "Primary" );
			pName->LoadFromFont( THEME->GetPathF("Common","title") );
			RString sText = GameButtonToLocalizedString( INPUTMAPPER->GetInputScheme(), pKey->m_GameButton );
			pName->SetText( sText );
			ActorUtil::LoadAllCommands( *pName, m_sName );
			m_Line[iRow].AddChild( pName );
		}
		{
			BitmapText *pSecondary = new BitmapText;
			pSecondary->SetName( "Secondary" );
			pSecondary->LoadFromFont( THEME->GetPathF("Common","title") );
			MenuButton mb = INPUTMAPPER->GetInputScheme()->GetMenuButtonSecondaryFunction( pKey->m_GameButton );
			RString sText;
			if( mb != MenuButton_Invalid )
				sText = GameButtonToLocalizedString( INPUTMAPPER->GetInputScheme(), mb );
			ActorUtil::LoadAllCommands( *pSecondary, m_sName );
			pSecondary->SetText( sText );
			m_Line[iRow].AddChild( pSecondary );
		}

		FOREACH_GameController( c )
		{			
			for( int s=0; s<NUM_SHOWN_GAME_TO_DEVICE_SLOTS; s++ ) 
			{
				pKey->m_textMappedTo[c][s] = new BitmapText;
				pKey->m_textMappedTo[c][s]->SetName( "MappedTo" );
				pKey->m_textMappedTo[c][s]->LoadFromFont( THEME->GetPathF(m_sName,"entry") );
				pKey->m_textMappedTo[c][s]->RunCommands( MAPPED_TO_COMMAND(c,s) );
				ActorUtil::LoadAllCommands( *pKey->m_textMappedTo[c][s], m_sName );
				m_Line[iRow].AddChild( pKey->m_textMappedTo[c][s] );
			}
		}
		m_Line[iRow].DeleteChildrenWhenDone();
		m_Line[iRow].SetName( "Line" );
		ActorUtil::LoadAllCommands( m_Line[iRow], m_sName );
		m_LineScroller.AddChild( &m_Line[iRow] );

		iRow++;
	}	

	// exit row
	{
		m_sprExit.Load( THEME->GetPathG(m_sName,"exit") );
		m_sprExit->SetName( "Exit" );
		ActorUtil::LoadAllCommands( *m_sprExit, m_sName );

		m_Line[iRow].AddChild( m_sprExit );
		m_LineScroller.AddChild( &m_Line[iRow] );

		iRow++;
	}

	m_LineScroller.SetName( "LineScroller" );
	ActorUtil::LoadAllCommands( m_LineScroller, m_sName );
	m_LineScroller.SetNumItemsToDraw( (float) m_LineScroller.GetNumChildren()*2 );
	m_LineScroller.Load2();
	this->AddChild( &m_LineScroller );
}

void ScreenMapControllers::BeginScreen()
{
	m_iCurController = 0;
	m_iCurButton = 0;
	m_iCurSlot = 0;

	ScreenWithMenuElements::BeginScreen();

	m_WaitingForPress.SetZero();

	Refresh();
	AfterChangeFocus();
}


void ScreenMapControllers::Update( float fDeltaTime )
{
	ScreenWithMenuElements::Update( fDeltaTime );

	
	//
	// Update devices text
	//
	m_textDevices.SetText( INPUTMAN->GetDisplayDevicesString() );


	if( !m_WaitingForPress.IsZero() && m_DeviceIToMap.IsValid() ) // we're going to map an input
	{	
		if( m_WaitingForPress.PeekDeltaTime() < g_fSecondsToWaitForInput )
			return; /* keep waiting */
		m_WaitingForPress.SetZero();

		ASSERT( m_iCurButton < (int) m_KeysToMap.size() );
		const KeyToMap *pKey = &m_KeysToMap[m_iCurButton];
		
		GameInput curGameI( (GameController)m_iCurController, pKey->m_GameButton );

		INPUTMAPPER->SetInputMap( m_DeviceIToMap, curGameI, m_iCurSlot );
		INPUTMAPPER->AddDefaultMappingsForCurrentGameIfUnmapped();

		// commit to disk after each change
		INPUTMAPPER->SaveMappingsToDisk();

		Refresh();

		BitmapText *pText = pKey->m_textMappedTo[m_iCurController][m_iCurSlot];
		pText->PlayCommand( "MappedInput" );
		SCREENMAN->PlayStartSound();
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
	if( input.type != IET_FIRST_PRESS && input.type != IET_REPEAT )
		return;	// ignore
	if( IsTransitioning() )
		return;	// ignore

	LOG->Trace( "ScreenMapControllers::Input():  device: %d, button: %d", 
		input.DeviceI.device, input.DeviceI.button );

	int button = input.DeviceI.button;

#ifdef _XBOX
	if( m_WaitingForPress.IsZero() && input.DeviceI.device == DEVICE_JOY1 )
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
	if( !m_WaitingForPress.IsZero() )
	{
		if( input.type != IET_FIRST_PRESS )
			return;

		/* Don't allow function keys to be mapped. */
		if( input.DeviceI.device == DEVICE_KEYBOARD && (input.DeviceI.button >= KEY_F1 && input.DeviceI.button <= KEY_F12) )
		{
			SCREENMAN->SystemMessage( INVALID_BUTTON );
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
			if( m_iCurButton == (int) m_KeysToMap.size() )
				break; // on exit

			{
				const KeyToMap *pKey = &m_KeysToMap[m_iCurButton];
				GameInput curGameI( (GameController)m_iCurController, pKey->m_GameButton );
				if( !INPUTMAPPER->ClearFromInputMap(curGameI, m_iCurSlot) )
					break;

				INPUTMAPPER->AddDefaultMappingsForCurrentGameIfUnmapped();

				m_soundDelete.Play();

				// commit to disk after each change
				INPUTMAPPER->SaveMappingsToDisk();
			}
			break;
		case KEY_LEFT: /* Move the selection left, wrapping up. */
			if( m_iCurButton == (int) m_KeysToMap.size() )
				break; // on exit
			if( m_iCurSlot == 0 && m_iCurController == 0 )
				break;	// can't go left any more
			BeforeChangeFocus();
			m_iCurSlot--;
			if( m_iCurSlot < 0 )
			{
				m_iCurSlot = NUM_CHANGABLE_SLOTS-1;
				m_iCurController--;
			}
			AfterChangeFocus();
			m_soundChange.Play();
			break;
		case KEY_RIGHT:	/* Move the selection right, wrapping down. */
			if( m_iCurButton == (int) m_KeysToMap.size() )
				break; // on exit
			if( m_iCurSlot == NUM_CHANGABLE_SLOTS-1 && m_iCurController == NUM_GameController-1 )
				break;	// can't go right any more
			BeforeChangeFocus();
			m_iCurSlot++;
			if( m_iCurSlot > NUM_CHANGABLE_SLOTS-1 )
			{
				m_iCurSlot = 0;
				m_iCurController++;
			}
			AfterChangeFocus();
			m_soundChange.Play();
			break;
		case KEY_UP: /* Move the selection up. */
			if( m_iCurButton == 0 )
				break;	// can't go up any more
			BeforeChangeFocus();
			m_iCurButton--;
			AfterChangeFocus();
			m_soundChange.Play();
			break;
		case KEY_DOWN: /* Move the selection down. */
			if( m_iCurButton == (int) m_KeysToMap.size() )
				break;	// can't go down any more
			BeforeChangeFocus();
			m_iCurButton++;
			AfterChangeFocus();
			m_soundChange.Play();
			break;
		case KEY_ESC: /* Quit the screen. */
			SCREENMAN->PlayStartSound();
			StartTransitioningScreen( SM_GoToNextScreen );		
			break;
		case KEY_ENTER: /* Change the selection. */
		case KEY_KP_ENTER:
			if( m_iCurButton == (int) m_KeysToMap.size() )
			{
				SCREENMAN->PlayStartSound();
				StartTransitioningScreen( SM_GoToNextScreen );		
				break;
			}

			{
				const KeyToMap *pKey = &m_KeysToMap[m_iCurButton];
				BitmapText *pText = pKey->m_textMappedTo[m_iCurController][m_iCurSlot];
				pText->PlayCommand( "Waiting" );
			}
			m_WaitingForPress.Touch();
			m_DeviceIToMap.MakeInvalid();
			SCREENMAN->PlayStartSound();
			break;
		}
	}

//	ScreenWithMenuElements::Input( input );	// default handler

	LOG->Trace( "m_iCurSlot: %d m_iCurController: %d m_iCurButton: %d", m_iCurSlot, m_iCurController, m_iCurButton );

	Refresh();
}

void ScreenMapControllers::TweenOffScreen()
{
	ScreenWithMenuElements::TweenOffScreen();

	OFF_COMMAND( m_LineScroller );
	OFF_COMMAND( m_textDevices );
}

Actor *ScreenMapControllers::GetActorWithFocus()
{
	if( m_iCurButton == (int) m_KeysToMap.size() )
		return m_sprExit;

	const KeyToMap *pKey = &m_KeysToMap[m_iCurButton];
	return pKey->m_textMappedTo[m_iCurController][m_iCurSlot];
}

void ScreenMapControllers::BeforeChangeFocus()
{
	Actor *pActor = GetActorWithFocus();
	pActor->PlayCommand( "LoseFocus" );
}

void ScreenMapControllers::AfterChangeFocus()
{
	Actor *pActor = GetActorWithFocus();
	pActor->PlayCommand( "GainFocus" );
}

void ScreenMapControllers::Refresh()
{
	FOREACH_GameController( p )
	{			
		for( unsigned b=0; b<m_KeysToMap.size(); b++ )
		{
			const KeyToMap *pKey = &m_KeysToMap[b];
			for( int s=0; s<NUM_SHOWN_GAME_TO_DEVICE_SLOTS; s++ ) 
			{
				BitmapText *pText = pKey->m_textMappedTo[p][s];
				GameInput cur_gi( p, pKey->m_GameButton );
				DeviceInput di;
				RString sText = "-----------";
				if( INPUTMAPPER->GameToDevice( cur_gi, s, di ) )
					sText = INPUTMAN->GetDeviceSpecificInputString( di );
				pText->SetText( sText );
			}
		}
	}

	m_LineScroller.SetDestinationItem( (float) m_iCurButton );
}

void ScreenMapControllers::HandleMessage( const Message &msg )
{
	if( msg == Message_AutoJoyMappingApplied )
	{
		Refresh();
	}

	ScreenWithMenuElements::HandleMessage( msg );
}


/*
 * (c) 2001-2005 Chris Danford, Glenn Maynard
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
