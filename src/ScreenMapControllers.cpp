#include "global.h"
#include "ScreenMapControllers.h"
#include "ScreenManager.h"
#include "ScreenPrompt.h"
#include "RageLog.h"
#include "RageInput.h"
#include "InputMapper.h"
#include "ThemeManager.h"
#include "ScreenDimensions.h"
#include "InputEventPlus.h"
#include "LocalizedString.h"

#include <cstddef>
#include <vector>

AutoScreenMessage(SM_DoSaveAndExit);
#define BUTTONS_TO_MAP			THEME->GetMetric ( m_sName, "ButtonsToMap" )
static LocalizedString INVALID_BUTTON   ( "ScreenMapControllers", "InvalidButton" );
static LocalizedString SAVE_PROMPT("ScreenMapControllers", "SavePrompt");
#define MAPPED_TO_COMMAND(gc,slot)	THEME->GetMetricA( m_sName, ssprintf("MappedToP%iS%iCommand", gc+1, slot+1) )

static const float g_fSecondsToWaitForInput = 0.05f;

// reserve the 3rd slot for hard-coded keys
static const int NUM_CHANGABLE_SLOTS = NUM_SHOWN_GAME_TO_DEVICE_SLOTS-1;

REGISTER_SCREEN_CLASS( ScreenMapControllers );

ScreenMapControllers::ScreenMapControllers()
{
	m_InSetListMode= false;
	m_ChangeOccurred= false;
	this->SubscribeToMessage( Message_AutoJoyMappingApplied );
}

ScreenMapControllers::~ScreenMapControllers()
{
	for(std::size_t i= 0; i < m_Line.size(); ++i)
	{
		SAFE_DELETE(m_Line[i]);
	}
}

static LocalizedString PLAYER_SLOTS( "ScreenMapControllers", "%s slots" );
static LocalizedString SLOT_NAMES[3]= {
	LocalizedString("ScreenMapControllers", "Primary"),
	LocalizedString("ScreenMapControllers", "Secondary"),
	LocalizedString("ScreenMapControllers", "Default")
};
static LocalizedString KEYNAME("ScreenMapControllers", "KeyName");
void ScreenMapControllers::Init()
{
	ScreenWithMenuElements::Init();

	m_soundChange.Load( THEME->GetPathS(m_sName,"change"), true );
	m_soundDelete.Load( THEME->GetPathS(m_sName,"delete"), true );


	m_textDevices.LoadFromFont( THEME->GetPathF(m_sName,"devices") );
	m_textDevices.SetName( "Devices" );
	LOAD_ALL_COMMANDS_AND_SET_XY( m_textDevices );
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
		std::vector<RString> asBits;
		split( sButtons, ",", asBits );
		for( unsigned i=0; i<asBits.size(); ++i )
		{
			KeyToMap k;
			k.m_GameButton = StringToGameButton( INPUTMAPPER->GetInputScheme(), asBits[i] );
			m_KeysToMap.push_back( k );
		}
	}

	// slot names row
	{
		m_Line.push_back(new ActorFrame);
		FOREACH_ENUM( GameController,  c )
		{
			BitmapText &text = m_textLabel[c];
			text.LoadFromFont( THEME->GetPathF(m_sName,"title") );
			PlayerNumber pn = (PlayerNumber)c;
			text.SetName( "Label"+PlayerNumberToString(pn) );
			RString sText = ssprintf(PLAYER_SLOTS.GetValue(), PlayerNumberToLocalizedString(pn).c_str());
			text.SetText( sText );
			ActorUtil::LoadAllCommands( text, m_sName );
			m_Line.back()->AddChild( &m_textLabel[c] );
		}
		m_LineScroller.AddChild(m_Line.back());
	}
	// header row
	{
		m_Line.push_back(new ActorFrame);
		m_ListHeaderCenter.LoadFromFont(THEME->GetPathF(m_sName, "title"));
		m_ListHeaderCenter.SetName("ListHeaderCenter");
		m_ListHeaderCenter.SetText(KEYNAME);
		ActorUtil::LoadAllCommands(m_ListHeaderCenter, m_sName);
		m_Line.back()->AddChild(&m_ListHeaderCenter);
		FOREACH_ENUM(GameController,  c)
		{
			for(int s= 0; s < NUM_SHOWN_GAME_TO_DEVICE_SLOTS; ++s)
			{
				BitmapText& text= m_ListHeaderLabels[c][s];
				text.LoadFromFont(THEME->GetPathF(m_sName, "title"));
				text.SetName("ListHeader");
				text.SetText(SLOT_NAMES[s]);
				text.RunCommands(THEME->GetMetricA(
						m_sName, ssprintf("ListHeaderP%iS%iCommand", c+1, s+1)));
				ActorUtil::LoadAllCommands(text, m_sName);
				m_Line.back()->AddChild(&text);
			}
		}
		m_LineScroller.AddChild(m_Line.back());
	}

	// normal rows
	for( unsigned b=0; b<m_KeysToMap.size(); b++ )
	{
		m_Line.push_back(new ActorFrame);
		KeyToMap *pKey = &m_KeysToMap[b];

		{
			BitmapText *pName = new BitmapText;
			pName->SetName( "Primary" );
			pName->LoadFromFont( THEME->GetPathF(m_sName,"title") );
			RString sText = GameButtonToLocalizedString( INPUTMAPPER->GetInputScheme(), pKey->m_GameButton );
			pName->SetText( sText );
			ActorUtil::LoadAllCommands( *pName, m_sName );
			m_Line.back()->AddChild( pName );
		}
		{
			BitmapText *pSecondary = new BitmapText;
			pSecondary->SetName( "Secondary" );
			pSecondary->LoadFromFont( THEME->GetPathF(m_sName,"title") );
			GameButton mb = INPUTMAPPER->GetInputScheme()->GameButtonToMenuButton( pKey->m_GameButton );
			RString sText;
			if( mb != GameButton_Invalid && mb != pKey->m_GameButton )
				sText = GameButtonToLocalizedString( INPUTMAPPER->GetInputScheme(), mb );
			ActorUtil::LoadAllCommands( *pSecondary, m_sName );
			pSecondary->SetText( sText );
			m_Line.back()->AddChild( pSecondary );
		}

		FOREACH_ENUM( GameController,  c )
		{
			for( int s=0; s<NUM_SHOWN_GAME_TO_DEVICE_SLOTS; s++ )
			{
				pKey->m_textMappedTo[c][s] = new BitmapText;
				pKey->m_textMappedTo[c][s]->SetName( "MappedTo" );
				pKey->m_textMappedTo[c][s]->LoadFromFont( THEME->GetPathF(m_sName,"entry") );
				pKey->m_textMappedTo[c][s]->RunCommands( MAPPED_TO_COMMAND(c,s) );
				ActorUtil::LoadAllCommands( *pKey->m_textMappedTo[c][s], m_sName );
				m_Line.back()->AddChild( pKey->m_textMappedTo[c][s] );
			}
		}
		m_Line.back()->DeleteChildrenWhenDone();
		m_Line.back()->SetName( "Line" );
		ActorUtil::LoadAllCommands(m_Line.back(), m_sName);
		m_LineScroller.AddChild(m_Line.back());
	}

#define ADD_ACTION(index, a, b, c, d, e) \
	m_Line.push_back(new ActorFrame);	\
	m_Actions[index].Load(a, b, c, d, e);

	m_Actions.resize(5);
	ADD_ACTION(0, m_sName, "Clear", &ScreenMapControllers::ClearToDefault,
		m_Line.back(), &m_LineScroller);
	ADD_ACTION(1, m_sName, "Reload", &ScreenMapControllers::ReloadFromDisk,
		m_Line.back(), &m_LineScroller);
	ADD_ACTION(2, m_sName, "Save", &ScreenMapControllers::SaveToDisk,
		m_Line.back(), &m_LineScroller);
	ADD_ACTION(3, m_sName, "SetList", &ScreenMapControllers::SetListMode,
		m_Line.back(), &m_LineScroller);
	ADD_ACTION(4, m_sName, "Exit", &ScreenMapControllers::ExitAction,
		m_Line.back(), &m_LineScroller);
#undef ADD_ACTION

	m_MaxDestItem= (1 + m_KeysToMap.size() + m_Actions.size()) -
		THEME->GetMetricI("ScreenMapControllers", "LinesVisible");

	m_LineScroller.SetName( "LineScroller" );
	ActorUtil::LoadAllCommands( m_LineScroller, m_sName );
	m_LineScroller.SetNumItemsToDraw( (float) m_LineScroller.GetNumChildren()*2 );
	m_LineScroller.Load2();
	this->AddChild( &m_LineScroller );

	m_NoSetListPrompt.Load(THEME->GetPathG(m_sName, "nosetlistprompt"));
	m_NoSetListPrompt->SetName("NoSetListPrompt");
	this->AddChild(m_NoSetListPrompt);

	m_SanityMessage.Load(THEME->GetPathG(m_sName, "sanitymessage"));
	m_SanityMessage->SetName("SanityMessage");
	this->AddChild(m_SanityMessage);

	m_Warning.Load(THEME->GetPathG(m_sName, "warning"));
	m_Warning->SetName("Warning");
	m_Warning->SetDrawOrder(DRAW_ORDER_TRANSITIONS);
	this->AddChild(m_Warning);
	LOAD_ALL_COMMANDS(m_Warning);
}

void ScreenMapControllers::BeginScreen()
{
	m_CurController = 0;
	m_CurButton = 0;
	m_CurSlot = 0;

	ScreenWithMenuElements::BeginScreen();

	m_WaitingForPress.SetZero();

	Refresh();
	AfterChangeFocus();
	m_fLockInputSecs= THEME->GetMetricF(m_sName, "LockInputSecs");
	m_AutoDismissWarningSecs= THEME->GetMetricF(m_sName, "AutoDismissWarningSecs");
	m_AutoDismissNoSetListPromptSecs= 0.0f;
	m_AutoDismissSanitySecs= 0.0f;
	if(m_AutoDismissWarningSecs > 0.25)
	{
		m_Warning->PlayCommand("TweenOn");
	}
	else
	{
		if(m_Warning->HasCommand("NeverShow"))
		{
			m_Warning->PlayCommand("NeverShow");
		}
		else
		{
			m_Warning->SetHibernate(16777216.0f);
		}
	}
}


void ScreenMapControllers::Update( float fDeltaTime )
{
	ScreenWithMenuElements::Update( fDeltaTime );

	if(m_fLockInputSecs <= 0.0f)
	{
		bool was_above= m_AutoDismissWarningSecs > 0.0f;
		m_AutoDismissWarningSecs-= fDeltaTime;
		if(was_above && m_AutoDismissWarningSecs <= 0.0f)
		{
			DismissWarning();
		}
	}
	if(m_AutoDismissNoSetListPromptSecs > 0.0f)
	{
		m_AutoDismissNoSetListPromptSecs-= fDeltaTime;
		if(m_AutoDismissNoSetListPromptSecs <= 0.0f)
		{
			m_NoSetListPrompt->PlayCommand("TweenOff");
		}
	}
	if(m_AutoDismissSanitySecs > 0.0f)
	{
		m_AutoDismissSanitySecs-= fDeltaTime;
		if(m_AutoDismissSanitySecs <= 0.0f)
		{
			m_SanityMessage->PlayCommand("TweenOff");
		}
	}

	//
	// Update devices text
	//
	m_textDevices.SetText( INPUTMAN->GetDisplayDevicesString() );

	if( !m_WaitingForPress.IsZero() && m_DeviceIToMap.IsValid() ) // we're going to map an input
	{
		if( m_WaitingForPress.PeekDeltaTime() < g_fSecondsToWaitForInput )
			return; /* keep waiting */
		m_WaitingForPress.SetZero();

		ASSERT(CursorOnKey());
		const KeyToMap *pKey = &m_KeysToMap[CurKeyIndex()];

		GameInput curGameI( (GameController)m_CurController, pKey->m_GameButton );

		INPUTMAPPER->SetInputMap( m_DeviceIToMap, curGameI, m_CurSlot );
		INPUTMAPPER->AddDefaultMappingsForCurrentGameIfUnmapped();
		m_ChangeOccurred= true;

		BitmapText *pText = pKey->m_textMappedTo[m_CurController][m_CurSlot];
		pText->PlayCommand("MappedInput");
		if(m_InSetListMode)
		{
			pText->PlayCommand("LoseMark");
			++m_SetListCurrent;
			if(m_SetListCurrent == m_SetList.end())
			{
				m_InSetListMode= false;
				m_SetList.clear();
			}
			else
			{
				BeforeChangeFocus();
				SetCursorFromSetListCurrent();
				AfterChangeFocus();
				StartWaitingForPress();
			}
		}
		Refresh();
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

bool ScreenMapControllers::Input( const InputEventPlus &input )
{
	if(m_fLockInputSecs > 0.0f)
	{
		return false;
	}

	if(m_AutoDismissWarningSecs > 0.0f)
	{
		if(input.type == IET_FIRST_PRESS &&
			input.DeviceI.device == DEVICE_KEYBOARD &&
			input.DeviceI.button == KEY_ENTER)
		{
			m_AutoDismissWarningSecs = 0.0f;
			DismissWarning();
		}
		return false;
	}

	if(m_AutoDismissNoSetListPromptSecs > 0.0f)
	{
		if(input.type == IET_FIRST_PRESS &&
			input.DeviceI.device == DEVICE_KEYBOARD &&
			input.DeviceI.button == KEY_ENTER)
		{
			m_AutoDismissNoSetListPromptSecs = 0.0f;
			m_NoSetListPrompt->PlayCommand("TweenOff");
		}
		return false;
	}

	if(m_AutoDismissSanitySecs > 0.0f)
	{
		if(input.type == IET_FIRST_PRESS &&
			input.DeviceI.device == DEVICE_KEYBOARD &&
			input.DeviceI.button == KEY_ENTER)
		{
			m_AutoDismissSanitySecs = 0.0f;
			m_SanityMessage->PlayCommand("TweenOff");
		}
		return false;
	}

	if( input.type != IET_FIRST_PRESS && input.type != IET_REPEAT )
	{
		return false;	// ignore
	}
	if( IsTransitioning() )
	{
		return false;	// ignore
	}

	// Whoever wants it can uncomment this log spew, I don't think it's necessary. -Kyz
	// LOG->Trace( "ScreenMapControllers::Input():  device: %d, button: %d",
	// 	input.DeviceI.device, input.DeviceI.button );

	int button = input.DeviceI.button;
	bool bHandled = false;

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
			return false;

		// Don't allow function keys to be mapped.
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
				return false;	// ignore this press
			}

			m_DeviceIToMap = input.DeviceI;
		}
	}
	else if( input.DeviceI.device == DEVICE_KEYBOARD )
	{
		switch( button )
		{
		/* We only advertise space as doing this, but most games use either delete
		 * or backspace, and I find them more intuitive, so allow them, too. -gm */

		/* XXX: For some reason that eludes me, this function gets sent an
		 * KEY_SPACE button press every time the JOY_HAT_UP button is pressed.
		 * Had to put this in to prevent mappings being erased everytime the user
		 * pressed up on the joypad. */

		case KEY_DEL:
		case KEY_SPACE:
		case KEY_BACK: // Clear the selected input mapping.
			if(!CursorOnKey())
			{
				break;
			}
			{
				const KeyToMap *pKey = &m_KeysToMap[CurKeyIndex()];
				GameInput curGameI( (GameController)m_CurController, pKey->m_GameButton );
				if( !INPUTMAPPER->ClearFromInputMap(curGameI, m_CurSlot) )
					break;

				INPUTMAPPER->AddDefaultMappingsForCurrentGameIfUnmapped();

				m_soundDelete.Play(true);
				bHandled = true;
			}
			break;
		case KEY_LEFT: // Move the selection left, wrapping up.
			if(!CursorCanGoLeft())
			{
				break;
			}
			BeforeChangeFocus();
			if(m_CurSlot == 0)
			{
				m_CurSlot = NUM_CHANGABLE_SLOTS-1;
				--m_CurController;
			}
			else
			{
				--m_CurSlot;
			}
			AfterChangeFocus();
			m_soundChange.Play(true);
			bHandled = true;
			break;
		case KEY_RIGHT:	// Move the selection right, wrapping down.
			if(!CursorCanGoRight())
			{
				break;
			}
			BeforeChangeFocus();
			m_CurSlot++;
			if( m_CurSlot > NUM_CHANGABLE_SLOTS-1 )
			{
				m_CurSlot = 0;
				m_CurController++;
			}
			AfterChangeFocus();
			m_soundChange.Play(true);
			bHandled = true;
			break;
		case KEY_UP: // Move the selection up.
			if(!CursorCanGoUp())
			{
				break;
			}
			BeforeChangeFocus();
			m_CurButton--;
			AfterChangeFocus();
			m_soundChange.Play(true);
			bHandled = true;
			break;
		case KEY_DOWN: // Move the selection down.
			if(!CursorCanGoDown())
			{
				break;
			}
			BeforeChangeFocus();
			m_CurButton++;
			AfterChangeFocus();
			m_soundChange.Play(true);
			bHandled = true;
			break;
		case KEY_ESC: // Quit the screen.
			ExitAction();
			bHandled = true;
			break;
		case KEY_Cm:
			if(CursorOnKey())
			{
				SetListEntry to_add(SetListEntry(CurKeyIndex(), m_CurController, m_CurSlot));
				std::set<SetListEntry>::iterator found= m_SetList.find(to_add);
				if(found == m_SetList.end())
				{
					m_SetList.insert(to_add);
					GetActorWithFocus()->PlayCommand("GainMark");
				}
				else
				{
					m_SetList.erase(found);
					GetActorWithFocus()->PlayCommand("LoseMark");
				}
			}
			break;
		case KEY_ENTER: // Change the selection.
		case KEY_KP_ENTER:
			bHandled = true;
			if(CursorOnAction())
			{
				(this->*m_Actions[CurActionIndex()].m_action)();
				SCREENMAN->PlayStartSound();
				break;
			}
			if(CursorOnHeader())
			{
				break;
			}
			StartWaitingForPress();
			SCREENMAN->PlayStartSound();
			break;
		}
	}

//	ScreenWithMenuElements::Input( input );	// default handler

	// This trace is also useless log spew.  Create a preference or something
	// configurable if you disagree. -Kyz
	// LOG->Trace( "m_CurSlot: %d m_CurController: %d m_CurButton: %d", m_CurSlot, m_CurController, m_CurButton );

	Refresh();
	return bHandled;
}

Actor *ScreenMapControllers::GetActorWithFocus()
{
	if(CursorOnAction())
	{
		return m_Actions[CurActionIndex()].m_actor;
	}
	if(CursorOnHeader())
	{
		return &(m_ListHeaderLabels[m_CurController][m_CurSlot]);
	}

	const KeyToMap *pKey = &m_KeysToMap[CurKeyIndex()];
	return pKey->m_textMappedTo[m_CurController][m_CurSlot];
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
	FOREACH_ENUM( GameController,  p )
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

	m_LineScroller.SetDestinationItem(
		static_cast<float>(std::min(m_CurButton, m_MaxDestItem)));
}

void ScreenMapControllers::DismissWarning()
{
	m_Warning->PlayCommand("TweenOff");
}

bool ScreenMapControllers::CursorOnAction()
{
	// We have a header row, the rows for the keys, and the action rows.
	// So every row after m_KeysToMap.size is an action row.
	return m_CurButton >= m_KeysToMap.size()+1;
}

bool ScreenMapControllers::CursorOnHeader()
{
	// We have a header row before all others.
	// So the header is at 0.
	return m_CurButton == 0;
}

bool ScreenMapControllers::CursorOnKey()
{
	return !(CursorOnHeader() || CursorOnAction());
}

bool ScreenMapControllers::CursorCanGoUp()
{
	return m_CurButton > 0;
}

bool ScreenMapControllers::CursorCanGoDown()
{
	return m_CurButton < m_KeysToMap.size() + m_Actions.size();
}

bool ScreenMapControllers::CursorCanGoLeft()
{
	return !CursorOnAction() && (m_CurSlot > 0 || m_CurController > 0);
}

bool ScreenMapControllers::CursorCanGoRight()
{
	return !CursorOnAction() && (m_CurSlot < NUM_CHANGABLE_SLOTS-1 ||
		m_CurController < NUM_GameController-1);
}

int ScreenMapControllers::CurKeyIndex()
{
	// The header row is at 0, so subtract 1 from m_CurButton.
	return m_CurButton - 1;
}

int ScreenMapControllers::CurActionIndex()
{
	// Subtract the header row and the keys.
	return m_CurButton - 1 - m_KeysToMap.size();
}

void ScreenMapControllers::SetCursorFromSetListCurrent()
{
	m_CurButton= m_SetListCurrent->m_button + 1;
	m_CurController= m_SetListCurrent->m_controller;
	m_CurSlot= m_SetListCurrent->m_slot;
}

void ScreenMapControllers::StartWaitingForPress()
{
	const KeyToMap *pKey = &m_KeysToMap[CurKeyIndex()];
	BitmapText *pText = pKey->m_textMappedTo[m_CurController][m_CurSlot];
	pText->PlayCommand( "Waiting" );
	m_WaitingForPress.Touch();
	m_DeviceIToMap.MakeInvalid();
}

void ScreenMapControllers::HandleScreenMessage(const ScreenMessage SM)
{
	if(SM == SM_DoSaveAndExit)
	{
		switch(ScreenPrompt::s_LastAnswer)
		{
			DEFAULT_FAIL(ScreenPrompt::s_LastAnswer);
			case ANSWER_YES:
				SaveToDisk();
				StartTransitioningScreen(SM_GoToNextScreen);
				break;
			case ANSWER_NO:
				ReloadFromDisk();
				StartTransitioningScreen(SM_GoToNextScreen);
				break;
			case ANSWER_CANCEL:
				break;
		}
	}
	else
	{
		ScreenWithMenuElements::HandleScreenMessage(SM);
	}
}

void ScreenMapControllers::HandleMessage( const Message &msg )
{
	if( msg == Message_AutoJoyMappingApplied )
	{
		Refresh();
	}

	ScreenWithMenuElements::HandleMessage( msg );
}

void ScreenMapControllers::ClearToDefault()
{
	INPUTMAPPER->ResetMappingsToDefault();
}

void ScreenMapControllers::ReloadFromDisk()
{
	INPUTMAPPER->ReadMappingsFromDisk();
}

void ScreenMapControllers::SaveToDisk()
{
	if(SanityCheckWrapper())
	{
		INPUTMAPPER->SaveMappingsToDisk();
		m_ChangeOccurred= false;
	}
}

void ScreenMapControllers::SetListMode()
{
	if(m_SetList.size() < 1)
	{
		m_NoSetListPrompt->PlayCommand("TweenOn");
		m_AutoDismissNoSetListPromptSecs= THEME->GetMetricF(m_sName, "AutoDismissNoSetListPromptSecs");
	}
	else
	{
		m_SetListCurrent= m_SetList.begin();
		m_InSetListMode= true;
		BeforeChangeFocus();
		SetCursorFromSetListCurrent();
		AfterChangeFocus();
		StartWaitingForPress();
	}
}

void ScreenMapControllers::ExitAction()
{
	if(m_ChangeOccurred)
	{
		// If the current mapping doesn't pass the sanity check, then the user
		// can't navigate the prompt screen to pick a choice. -Kyz
		if(SanityCheckWrapper())
		{
			ScreenPrompt::Prompt(SM_DoSaveAndExit, SAVE_PROMPT,
				PROMPT_YES_NO_CANCEL, ANSWER_YES);
		}
	}
	else
	{
		SCREENMAN->PlayStartSound();
		StartTransitioningScreen(SM_GoToNextScreen);
	}
}

bool ScreenMapControllers::SanityCheckWrapper()
{
	std::vector<RString> reasons_not_sane;
	INPUTMAPPER->SanityCheckMappings(reasons_not_sane);
	if(reasons_not_sane.empty())
	{
		return true;
	}
	else
	{
		for (RString &reason : reasons_not_sane)
		{
			reason= THEME->GetString("ScreenMapControllers", reason);
		}
		RString joined_reasons= join("\n", reasons_not_sane);
		joined_reasons= THEME->GetString("ScreenMapControllers", "VitalButtons") + "\n" + joined_reasons;
		Message msg("SetText");
		msg.SetParam("Text", joined_reasons);
		m_SanityMessage->HandleMessage(msg);
		m_AutoDismissSanitySecs= THEME->GetMetricF(m_sName, "AutoDismissSanitySecs");
		return false;
	}
}

void ScreenMapControllers::ActionRow::Load(RString const& scr_name,
	RString const& name, ScreenMapControllers::action_fun_t action,
	ActorFrame* line, ActorScroller* scroller)
{
	m_action= action;
	RString lower_name= name;
	lower_name.MakeLower();
	// Make the specific actor optional, use a fallback if it doesn't exist.
	RString path= THEME->GetPathG(scr_name, lower_name, true);
	if(path.empty())
	{
		path= THEME->GetPathG(scr_name, "action");
	}
	m_actor.Load(path);
	m_actor->SetName(name);
	ActorUtil::LoadAllCommands(*m_actor, scr_name);
	line->AddChild(m_actor);
	scroller->AddChild(line);
}


/*
 * (c) 2001-2005 Chris Danford, Glenn Maynard
 * 2014 Eric Reese
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
