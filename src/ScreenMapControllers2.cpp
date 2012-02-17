#include "global.h"
#include "ScreenMapControllers2.h"
#include "ScreenManager.h"
#include "RageLog.h"
#include "GameState.h"
#include "RageInput.h"
#include "InputMapper.h"
#include "ThemeManager.h"
#include "ScreenDimensions.h"
#include "InputEventPlus.h"
#include "LocalizedString.h"
#include "Foreach.h"
#include "LuaBinding.h"

static const float g_fSecondsToWaitForInput = 0.05f;

static const char *MapControlsStateNames[] = {
	"SelectingButton",
	"EditingMappings"
};
XToString( MapControlsState );
LuaXType( MapControlsState );

REGISTER_SCREEN_CLASS( ScreenMapControllers2 );

ScreenMapControllers2::ScreenMapControllers2()
{
	this->SubscribeToMessage( Message_AutoJoyMappingApplied );
	m_MapControlsState = MCS_Button;
	m_EditingButton = GAME_BUTTON_MENULEFT;
	ZERO( m_iMappingChoice );
	ZERO( m_iNumMappingRows );
}

static LocalizedString ADD_MAPPING( "ScreenMapControllers2", "Add Mapping" );
void ScreenMapControllers2::Init()
{
	ScreenWithMenuElements::Init();

	// load sounds
	m_soundChange.Load( THEME->GetPathS(m_sName,"change"), true );
	m_soundDelete.Load( THEME->GetPathS(m_sName,"delete"), true );

	// set devices text
	m_textDevices.LoadFromFont( THEME->GetPathF(m_sName,"devices") );
	m_textDevices.SetName( "Devices" );
	LOAD_ALL_COMMANDS_AND_SET_XY( m_textDevices );
	this->AddChild( &m_textDevices );

	// button input scroller
	WRAP_INPUT_SCROLLER.Load( m_sName, "WrapInputScroller" );
	LOOP_INPUT_SCROLLER.Load( m_sName, "LoopInputScroller" );
	INPUT_SCROLLER_SECONDS_PER_ITEM.Load( m_sName, "InputScrollerSecondsPerItem" );
	INPUT_SCROLLER_NUM_ITEMS_TO_DRAW.Load( m_sName, "InputScrollerNumItemsToDraw" );
	INPUT_SCROLLER_TRANSFORM.Load( m_sName, "InputScrollerTransform" );
	INPUT_SCROLLER_SUBDIVISIONS.Load( m_sName, "InputScrollerSubdivisions" );

	// map all buttons for this game. (todo: re-add BUTTONS_TO_MAP support?)
	FOREACH_GameButtonInScheme( INPUTMAPPER->GetInputScheme(), gb )
	{
		KeyToMap k;
		k.m_GameButton = gb;
		m_KeysToMap.push_back( k );
	}

	// input button names
	int iRow = 0;
	for( unsigned b=0; b<m_KeysToMap.size(); b++ )
	{
		KeyToMap *pKey = &m_KeysToMap[b];
		{
			BitmapText *pName = new BitmapText;
			pName->SetName( "ButtonName" );
			pName->LoadFromFont( THEME->GetPathF(m_sName,"ButtonName") );
			RString sText = GameButtonToLocalizedString( INPUTMAPPER->GetInputScheme(), pKey->m_GameButton );
			pName->SetText( sText );
			ActorUtil::LoadAllCommands( *pName, m_sName );
			pName->PlayCommand( b == (unsigned)m_iInputChoice ? "GainFocus" : "LoseFocus" );
			m_InputLine[iRow].AddChild( pName );
		}

		m_InputLine[iRow].DeleteChildrenWhenDone();
		m_InputLine[iRow].SetName( "InputLine" );
		ActorUtil::LoadAllCommands( m_InputLine[iRow], m_sName );
		m_InputScroller.AddChild( &m_InputLine[iRow] );

		iRow++;
	}

	m_InputScroller.SetName( "InputScroller" );
	LOAD_ALL_COMMANDS_AND_SET_XY( m_InputScroller );
	m_InputScroller.SetLoop( LOOP_INPUT_SCROLLER );
	m_InputScroller.SetNumItemsToDraw( INPUT_SCROLLER_NUM_ITEMS_TO_DRAW );
	m_InputScroller.Load2();
	m_InputScroller.SetTransformFromReference( INPUT_SCROLLER_TRANSFORM );
	m_InputScroller.SetSecondsPerItem( INPUT_SCROLLER_SECONDS_PER_ITEM );
	m_InputScroller.SetNumSubdivisions( INPUT_SCROLLER_SUBDIVISIONS );
	this->AddChild( &m_InputScroller );

	// set player scrollers:
	vector<PlayerNumber> vpns;
	FOREACH_PlayerNumber( p )
		vpns.push_back( p );

	// per-player mappings
	WRAP_MAPPING_SCROLLER.Load( m_sName, "WrapMappingScroller" );
	LOOP_MAPPING_SCROLLER.Load( m_sName, "LoopMappingScroller" );
	MAPPING_SCROLLER_SECONDS_PER_ITEM.Load( m_sName, "MappingScrollerSecondsPerItem" );
	MAPPING_SCROLLER_NUM_ITEMS_TO_DRAW.Load( m_sName, "MappingScrollerNumItemsToDraw" );
	MAPPING_SCROLLER_TRANSFORM.Load( m_sName, "MappingScrollerTransform" );
	MAPPING_SCROLLER_SUBDIVISIONS.Load( m_sName, "MappingScrollerSubdivisions" );

	FOREACH( PlayerNumber, vpns, p )
	{
		// mapping scroller
		m_MappingScroller[*p].SetLoop( LOOP_MAPPING_SCROLLER );
		m_MappingScroller[*p].SetNumItemsToDraw( MAPPING_SCROLLER_NUM_ITEMS_TO_DRAW );
		m_MappingScroller[*p].Load2();
		m_MappingScroller[*p].SetTransformFromReference( MAPPING_SCROLLER_TRANSFORM );
		m_MappingScroller[*p].SetSecondsPerItem( MAPPING_SCROLLER_SECONDS_PER_ITEM );
		m_MappingScroller[*p].SetNumSubdivisions( MAPPING_SCROLLER_SUBDIVISIONS );
		m_MappingScroller[*p].SetName( "MappingScroller"+ssprintf("P%d",(*p)+1) );
		LOAD_ALL_COMMANDS_AND_SET_XY( m_MappingScroller[*p] );
		this->AddChild( &m_MappingScroller[*p] );
	}
}

void ScreenMapControllers2::BeginScreen()
{
	m_iInputChoice = 0;
	m_ActivePlayerMapping = PLAYER_1;

	ScreenWithMenuElements::BeginScreen();

	m_WaitingForPress.SetZero();

	Refresh();
}

void ScreenMapControllers2::Update( float fDeltaTime )
{
	ScreenWithMenuElements::Update( fDeltaTime );

	// Update devices text
	m_textDevices.SetText( INPUTMAN->GetDisplayDevicesString() );

	if( !m_WaitingForPress.IsZero() && m_DeviceIToMap.IsValid() )
	{
		// we're going to map an input
		if( m_WaitingForPress.PeekDeltaTime() < g_fSecondsToWaitForInput )
			return; // keep waiting
		m_WaitingForPress.SetZero();

		/* do stuff here */
		//ASSERT( m_iCurButton < (int) m_KeysToMap.size() );
		//const KeyToMap *pKey = &m_KeysToMap[m_iCurButton];
		//GameInput curGameI( (GameController)m_iCurController, pKey->m_GameButton );
		//INPUTMAPPER->SetInputMap( m_DeviceIToMap, curGameI, m_iCurSlot );
		//INPUTMAPPER->AddDefaultMappingsForCurrentGameIfUnmapped();

		// commit to disk after each change
		//INPUTMAPPER->SaveMappingsToDisk();

		Refresh();

		/*
		BitmapText *pText = pKey->m_textMappedTo[m_iCurController][m_iCurSlot];
		pText->PlayCommand( "MappedInput" );
		*/
		SCREENMAN->PlayStartSound();
	}
}


/* Note that this isn't necessarily correct. For example, JOY_LEFT might
 * actually be a D-pad and not an axis. All this is actually doing is giving
 * priority to some inputs over others; this function is unsuitable for other use. */
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

void ScreenMapControllers2::ChangeActivePlayer()
{
	m_ActivePlayerMapping = (m_ActivePlayerMapping == PLAYER_1) ? PLAYER_2 : PLAYER_1;
	Message msg("ActivePlayerChanged");
	msg.SetParam("NewActivePlayer",m_ActivePlayerMapping);
	MESSAGEMAN->Broadcast(msg);
}

void ScreenMapControllers2::Input( const InputEventPlus &input )
{
	if( input.type != IET_FIRST_PRESS && input.type != IET_REPEAT )
		return;	// ignore
	if( IsTransitioning() )
		return;	// ignore

	LOG->Trace( "ScreenMapControllers2::Input():  device: %d, button: %d", 
		input.DeviceI.device, input.DeviceI.button );

	int button = input.DeviceI.button;

	/* TRICKY: Some adapters map the PlayStation digital d-pad to both axes and
	 * buttons. We want buttons to be used for any mappings where possible
	 * because presses of buttons aren't mutually exclusive and presses of axes
	 * are (e.g. can't read presses of both Left and Right simultaneously). So,
	 * when the user presses a button, we'll wait until the next Update before
	 * adding a mapping so that we get a chance to see all input events the 
	 * user's press of a panel. Prefer non-axis events over axis events. */
	if( !m_WaitingForPress.IsZero() )
	{
		if( input.type != IET_FIRST_PRESS )
			return;

		// Don't allow function keys to be mapped.
		if( input.DeviceI.device == DEVICE_KEYBOARD && (input.DeviceI.button >= KEY_F1 && input.DeviceI.button <= KEY_F12) )
		{
			SCREENMAN->SystemMessage( "INVALID_BUTTON" );
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
	else if( input.DeviceI.device == DEVICE_KEYBOARD )
	{
		switch( button )
		{
			// todo: m_MapControlsState checks
			case KEY_UP: // mapping list up
				{
					if( m_MapControlsState == MCS_EditMappings )
					{
						if( m_iMappingChoice[m_ActivePlayerMapping] == 0 )
							break;
						m_iMappingChoice[m_ActivePlayerMapping]--;
					}
				}
				break;
			case KEY_DOWN: // mapping list down
				{
					if( m_MapControlsState == MCS_EditMappings )
					{
						if( m_iMappingChoice[m_ActivePlayerMapping] == m_iNumMappingRows[m_ActivePlayerMapping] )
							break;
						m_iMappingChoice[m_ActivePlayerMapping]++;
					}
				}
				break;
			case KEY_LEFT: // on P2, changing to P1
				{
					if( m_MapControlsState == MCS_EditMappings )
					{
						if(m_ActivePlayerMapping == PLAYER_2)
							ChangeActivePlayer();
					}
				}
				break;
			case KEY_RIGHT: // on P1, changing to P2
				{
					if( m_MapControlsState == MCS_EditMappings )
					{
						if(m_ActivePlayerMapping == PLAYER_1)
							ChangeActivePlayer();
					}
				}
				break;
			case KEY_DEL:
			case KEY_BACK:
				if( m_MapControlsState == MCS_EditMappings )
				{
					// clear mapping
					/*
					if( m_iMappingChoice[m_ActivePlayerMapping] == m_iNumMappingRows[m_ActivePlayerMapping] )
						break; // on "add"

					{
						// stuff

						m_soundDelete.Play();

						// commit to disk after each change
						INPUTMAPPER->SaveMappingsToDisk();
					}
					*/
				}
				break;
			case KEY_ESC:
				{
					if(m_MapControlsState == MCS_EditMappings)
					{
						// xxx: make sure it's not getting a button.
						m_MapControlsState = MCS_Button;
						SCREENMAN->PlayCancelSound();
					}
					else
					{
						// Quit the screen.
						SCREENMAN->PlayStartSound();
						StartTransitioningScreen( SM_GoToNextScreen );
					}
				}
				break;
			case KEY_ENTER:
			case KEY_KP_ENTER:
				{
					if( m_MapControlsState == MCS_EditMappings )
					{
						// map button
						LOG->Trace("got an enter press...");
						LOG->Trace( ssprintf("current choice: %i",m_iMappingChoice[m_ActivePlayerMapping]) );
						if( m_iMappingChoice[m_ActivePlayerMapping] == m_iNumMappingRows[m_ActivePlayerMapping] )
						{
							// add
							LOG->Trace("Adding a new mapping");
						}
						else
						{
							// replace
							LOG->Trace("Replacing an existing mapping");
						}
						/*
						m_WaitingForPress.Touch();
						m_DeviceIToMap.MakeInvalid();
						SCREENMAN->PlayStartSound();
						*/
					}
				}
				break;
		}
	}

	ScreenWithMenuElements::Input( input );	// default handler

	Refresh();
}

void ScreenMapControllers2::Refresh()
{
	// update mapping scrollers
	UpdateMappingScrollers();
}

void ScreenMapControllers2::HandleMessage( const Message &msg )
{
	if( msg == Message_AutoJoyMappingApplied )
	{
		Refresh();
	}

	ScreenWithMenuElements::HandleMessage( msg );
}

void ScreenMapControllers2::MenuStart( const InputEventPlus &input )
{
	// MCS_EditMappings is handled in Input() above.
	if( m_MapControlsState == MCS_Button )
	{
		m_MapControlsState = MCS_EditMappings;
	}
}

void ScreenMapControllers2::UpdateMappingScrollers()
{
	FOREACH_ENUM( GameController,  c )
	{
		PlayerNumber pn = (PlayerNumber)c;
		m_MappingScroller[pn].RemoveAllChildren();

		int iLast = 0;
		for(unsigned s = 0; s < NUM_SHOWN_GAME_TO_DEVICE_SLOTS; s++)
		{
			GameInput cur_gi( c, m_EditingButton );
			DeviceInput di;
			RString sInput = "";
			if( INPUTMAPPER->GameToDevice( cur_gi, s, di ) )
				sInput = INPUTMAN->GetDeviceSpecificInputString(di);

			if( sInput != "" )
			{
				m_MappingLine[iLast].RemoveAllChildren();
				// we have an input, add to the scroller
				BitmapText *pText = new BitmapText;
				pText->SetName( "MappedTo"+PlayerNumberToString(pn) );
				pText->LoadFromFont( THEME->GetPathF(m_sName,"Mapping") );
				ActorUtil::LoadAllCommands(pText,m_sName);
				pText->PlayCommand( ((unsigned)m_iMappingChoice[pn] == s) ? "GainFocus" : "LoseFocus" );
				pText->SetText( sInput );

				m_MappingLine[iLast].AddChild( pText );
				m_MappingLine[iLast].DeleteChildrenWhenDone();
				m_MappingLine[iLast].SetName("MappingLine");
				ActorUtil::LoadAllCommands(m_MappingLine[iLast],m_sName);
				m_MappingScroller[pn].AddChild( &m_MappingLine[iLast] );
				iLast++;
			}
		}

		BitmapText &text = m_textAddMapping[pn];
		text.LoadFromFont( THEME->GetPathF(m_sName,"AddMapping") );
		text.SetName( "AddMapping"+PlayerNumberToString(pn) );
		ActorUtil::LoadAllCommands( text, m_sName );
		text.PlayCommand( (m_iMappingChoice[pn] == m_iNumMappingRows[pn]) ? "GainFocus" : "LoseFocus" );
		text.SetText( ADD_MAPPING );
		m_MappingScroller[pn].AddChild( &m_textAddMapping[c] );
		iLast++;

		m_iNumMappingRows[pn] = iLast;

		m_MappingScroller[pn].Load2();
		m_MappingScroller[pn].SetTransformFromReference( MAPPING_SCROLLER_TRANSFORM );
		m_MappingScroller[pn].SetCurrentAndDestinationItem( (float)m_iMappingChoice[pn] );
	}
}

// change game/system button scroller selection
void ScreenMapControllers2::ChangeInputSelection(int iDir)
{
	m_InputLine[m_iInputChoice].PlayCommand("LoseFocus");
	m_iInputChoice += iDir;
	m_soundChange.Play();
	m_InputLine[m_iInputChoice].PlayCommand("GainFocus");

	// set scroller
	m_InputScroller.SetCurrentAndDestinationItem( (float)m_iInputChoice );

	// set current game button
	KeyToMap *pKey = &m_KeysToMap[m_iInputChoice];
	m_EditingButton = pKey->m_GameButton;
}

void ScreenMapControllers2::MenuUp( const InputEventPlus &input )
{
	if( m_MapControlsState == MCS_Button )
	{
		if( m_iInputChoice == 0 ) // limit
			return;
		ChangeInputSelection(-1);
	}
}

void ScreenMapControllers2::MenuDown( const InputEventPlus &input )
{
	if( m_MapControlsState == MCS_Button )
	{
		if( m_iInputChoice == (int)m_KeysToMap.size()-1 ) // limit
			return;
		ChangeInputSelection(1);
	}
}

void ScreenMapControllers2::TweenOnScreen()
{
	ScreenWithMenuElements::TweenOnScreen();

	m_InputScroller.SetCurrentAndDestinationItem( (float)m_iInputChoice );
}

// lua start
#include "LuaBinding.h"

/** @brief Allow Lua to have access to ScreenMapControllers2. */ 
class LunaScreenMapControllers2: public Luna<ScreenMapControllers2>
{
public:
	//static int MapControlsState( T* p, lua_State *L ) //
	//static int GetEditingGameButton( T* p, lua_State *L ) // GetEditingGameButton()
	DEFINE_METHOD( GetInputIndex, GetInputIndex() );
	DEFINE_METHOD( GetNumInputs, GetNumInputs() );
	LunaScreenMapControllers2()
	{
		//ADD_METHOD( GetEditingGameButton );
		ADD_METHOD( GetInputIndex );
		ADD_METHOD( GetNumInputs );
		//ADD_METHOD( MapControlsState );
	}
};

LUA_REGISTER_DERIVED_CLASS( ScreenMapControllers2, ScreenWithMenuElements )


/*
 * (c) 2011 AJ Kelly
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
