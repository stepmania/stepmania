#include "global.h"
#include "ScreenDebugOverlay.h"
#include "ScreenDimensions.h"
#include "PrefsManager.h"
#include "RageLog.h"
#include "GameState.h"
#include "PlayerState.h"
#include "StepMania.h"
#include "GameCommand.h"
#include "ScreenGameplay.h"

static bool g_bIsDisplayed = false;
static bool g_bIsSlow = false;
static bool g_bIsHalt = false;
static RageTimer g_HaltTimer(RageZeroTimer);

REGISTER_SCREEN_CLASS( ScreenDebugOverlay );
ScreenDebugOverlay::ScreenDebugOverlay( const CString &sName ) : Screen(sName)
{
}

ScreenDebugOverlay::~ScreenDebugOverlay()
{
}

struct MapDebugToDI
{
	DeviceInput holdForMenu;
	DeviceInput button[NUM_DEBUG_LINES];
	void Clear()
	{
		holdForMenu.MakeInvalid();
		FOREACH_DebugLine(i)
			button[i].MakeInvalid();
	}
};
static MapDebugToDI g_Mappings;

static CString GetDebugButtonName( DebugLine i )
{
	// TODO: Make arch appropriate.
	return g_Mappings.button[i].toString();
}

void ScreenDebugOverlay::Init()
{
	Screen::Init();
	
	// Init debug mappings
	// TODO: Arch-specific?
	{
		g_Mappings.Clear();

		g_Mappings.holdForMenu = DeviceInput(DEVICE_KEYBOARD, KEY_F3);
		g_Mappings.button[0] = DeviceInput(DEVICE_KEYBOARD, KEY_C1);
		g_Mappings.button[1] = DeviceInput(DEVICE_KEYBOARD, KEY_C2);
		g_Mappings.button[2] = DeviceInput(DEVICE_KEYBOARD, KEY_C3);
		g_Mappings.button[3] = DeviceInput(DEVICE_KEYBOARD, KEY_C4);
		g_Mappings.button[4] = DeviceInput(DEVICE_KEYBOARD, KEY_C5);
		g_Mappings.button[5] = DeviceInput(DEVICE_KEYBOARD, KEY_C6);
		g_Mappings.button[6] = DeviceInput(DEVICE_KEYBOARD, KEY_C7);
		g_Mappings.button[7] = DeviceInput(DEVICE_KEYBOARD, KEY_C8);
		g_Mappings.button[8] = DeviceInput(DEVICE_KEYBOARD, KEY_C9);
		g_Mappings.button[9] = DeviceInput(DEVICE_KEYBOARD, KEY_C0);
		g_Mappings.button[10] = DeviceInput(DEVICE_KEYBOARD, KEY_UNDERSCORE);
		g_Mappings.button[11] = DeviceInput(DEVICE_KEYBOARD, KEY_EQUAL);
	}


	m_Quad.StretchTo( RectF( 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT ) );
	m_Quad.SetDiffuse( RageColor(0, 0, 0, 0.5f) );
	this->AddChild( &m_Quad );
	
	m_Header.LoadFromFont( THEME->GetPathToF("Common normal") );
	m_Header.SetHorizAlign( Actor::align_left );
	m_Header.SetX( 20 );
	m_Header.SetY( SCREEN_TOP+20 );
	m_Header.SetZoom( 1.0f );
	m_Header.SetText( "Debug Menu" );
	this->AddChild( &m_Header );

	FOREACH_DebugLine( i )
	{
		BitmapText &txt = m_Text[i];
		txt.LoadFromFont( THEME->GetPathToF("Common normal") );
		txt.SetHorizAlign( Actor::align_left );
		this->AddChild( &txt );
	}
	
	Update( 0 );
}

static void SetSpeed()
{
    if( g_bIsHalt )
    {
		INPUTFILTER->ButtonPressed( DeviceInput(DEVICE_KEYBOARD, KEY_TAB), true );
		INPUTFILTER->ButtonPressed( DeviceInput(DEVICE_KEYBOARD, KEY_ACCENT), true );
    }
    else if( g_bIsSlow )
    {
		INPUTFILTER->ButtonPressed( DeviceInput(DEVICE_KEYBOARD, KEY_TAB), false );
		INPUTFILTER->ButtonPressed( DeviceInput(DEVICE_KEYBOARD, KEY_ACCENT), true );
    }
    else
    {
		INPUTFILTER->ButtonPressed( DeviceInput(DEVICE_KEYBOARD, KEY_TAB), false );
		INPUTFILTER->ButtonPressed( DeviceInput(DEVICE_KEYBOARD, KEY_ACCENT), false );
    }
	
    // PauseMusic( g_bIsHalt );
}

void ScreenDebugOverlay::Update( float fDeltaTime )
{
	Screen::Update(fDeltaTime);

    this->SetVisible( g_bIsDisplayed );
	if( !g_bIsDisplayed )
		return;

	/* Highlight options that aren't the default. */
	const RageColor off(0.6f, 0.6f, 0.6f, 1.0f);
	const RageColor on(1, 1, 1, 1);
	
	FOREACH_DebugLine( i )
	{
		BitmapText &txt = m_Text[i];
		txt.SetX( 100 );
		txt.SetY( SCALE(i, 0, NUM_DEBUG_LINES-1, SCREEN_TOP+60, SCREEN_BOTTOM-40) );
		txt.SetZoom( 0.8f );

		CString s;
		switch( i )
		{
		case DebugLine_Autoplay:			s="AutoPlay";			break;
		case DebugLine_CoinMode:		{ s="CoinMode: "+CoinModeToString(PREFSMAN->m_CoinMode); }	break;
		case DebugLine_Slow:				s="Slow";				break;
		case DebugLine_Halt:				s="Halt";				break;
		case DebugLine_LightsDebug:			s="Lights Debug";		break;
		case DebugLine_MonkeyInput:			s="MonkeyInput";		break;
		case DebugLine_Stats:				s="Stats";				break;
		case DebugLine_Vsync:				s="Vsync";				break;
		case DebugLine_ScreenTestMode:		s="Screen Test Mode";	break;
		case DebugLine_ClearMachineStats:	s="Clear Machine Stats";break;
		case DebugLine_FillMachineStats:	s="Fill Machine Stats";	break;
		case DebugLine_SendNotesEnded:		s="Send Notes Ended";	break;
		case DebugLine_CurrentScreen:	{ s="Screen: "; s+=SCREENMAN ? SCREENMAN->GetTopScreen()->m_sName : ""; }	break;
		case DebugLine_Uptime:			s="uptime: " + SecondsToMMSSMsMsMs(RageTimer::GetTimeSinceStart());	break;
		default:	ASSERT(0);
		}
		CString sButton = GetDebugButtonName(i);
		if( !sButton.empty() )
			sButton += ": ";
		txt.SetText( sButton + s );

		bool bOn = false;
		switch( i )
		{
		case DebugLine_Autoplay:			bOn=PREFSMAN->m_bAutoPlay;		break;
		case DebugLine_CoinMode:			bOn=true;						break;
		case DebugLine_Slow:				bOn=g_bIsSlow;					break;
		case DebugLine_Halt:				bOn=g_bIsHalt;					break;
		case DebugLine_LightsDebug:			bOn=false;						break;
		case DebugLine_MonkeyInput:			bOn=false;						break;
		case DebugLine_Stats:				bOn=PREFSMAN->m_bShowStats;		break;
		case DebugLine_Vsync:				bOn=PREFSMAN->m_bVsync;			break;
		case DebugLine_ScreenTestMode:		bOn=PREFSMAN->m_bScreenTestMode;break;
		case DebugLine_ClearMachineStats:	bOn=true;						break;
		case DebugLine_FillMachineStats:	bOn=true;						break;
		case DebugLine_SendNotesEnded:		bOn=true;						break;
		case DebugLine_CurrentScreen:		bOn=false;						break;
		case DebugLine_Uptime:				bOn=false;						break;
		default:	ASSERT(0);
		}

		txt.SetDiffuse( bOn ? on:off );
	}
	
    if( g_bIsHalt )
    {
		/* More than once I've paused the game accidentally and wasted time
		 * figuring out why, so warn. */
		if( g_HaltTimer.Ago() >= 5.0f )
		{
			g_HaltTimer.Touch();
			LOG->Warn( "Game halted" );
		}
    }
}

bool ScreenDebugOverlay::OverlayInput( const DeviceInput& DeviceI, const InputEventType type, const GameInput &GameI, const MenuInput &MenuI, const StyleInput &StyleI )
{
	if( DeviceI == g_Mappings.holdForMenu )
	{
		if( type == IET_FIRST_PRESS )
			g_bIsDisplayed = true;
		else if( type == IET_RELEASE )
			g_bIsDisplayed = false;
	}

	if( !g_bIsDisplayed )
		return false;

	if( type != IET_FIRST_PRESS )
		return false; /* eat the input but do nothing */

	FOREACH_DebugLine( i )
	{
		if( DeviceI == g_Mappings.button[i] )
		{
			BitmapText &txt = m_Text[i];

			txt.FinishTweening();
			float fZoom = txt.GetZoom();
			txt.SetZoom( fZoom * 1.2f );
			txt.BeginTweening( 0.2f, Actor::TWEEN_LINEAR );
			txt.SetZoom( fZoom );

			switch( i )
			{
			case DebugLine_Autoplay:
				PREFSMAN->m_bAutoPlay.Set( !PREFSMAN->m_bAutoPlay );
				FOREACH_HumanPlayer(pn)
					GAMESTATE->m_pPlayerState[pn]->m_PlayerController = PREFSMAN->m_bAutoPlay ? PC_AUTOPLAY : PC_HUMAN;
				break;
			case DebugLine_CoinMode:
				{
					int i = PREFSMAN->m_CoinMode.Get();
					i++;
					wrap( i, NUM_COIN_MODES );
					PREFSMAN->m_CoinMode.Set( (CoinMode)i );
				}
				break;
			case DebugLine_Slow:
				g_bIsSlow = !g_bIsSlow;
				SetSpeed();
				break;
			case DebugLine_Halt:
				g_bIsHalt = !g_bIsHalt;
				g_HaltTimer.Touch();
				SetSpeed();
				break;
			case DebugLine_LightsDebug:
				ASSERT(0);	// TODO
				break;
			case DebugLine_MonkeyInput:
				ASSERT(0);	// TODO
				break;
			case DebugLine_Stats:
				PREFSMAN->m_bShowStats.Set( !PREFSMAN->m_bShowStats );
				break;
			case DebugLine_Vsync:
				PREFSMAN->m_bVsync.Set( !PREFSMAN->m_bVsync );
				ApplyGraphicOptions();
				break;
			case DebugLine_ScreenTestMode:
				PREFSMAN->m_bScreenTestMode.Set( !PREFSMAN->m_bScreenTestMode );
				break;
			case DebugLine_ClearMachineStats:
				{
					GameCommand gc;
					gc.Load( 0, ParseCommands("ClearMachineStats") );
					gc.ApplyToAllPlayers();
				}
				break;
			case DebugLine_FillMachineStats:
				{
					GameCommand gc;
					gc.Load( 0, ParseCommands("FillMachineStats") );
					gc.ApplyToAllPlayers();
				}
				break;
			case DebugLine_SendNotesEnded:
				SCREENMAN->PostMessageToTopScreen( SM_NotesEnded, 0 );
				break;
			case DebugLine_CurrentScreen:
				break;
			case DebugLine_Uptime:
				break;
			default:	ASSERT(0);
			}
			break;
		}
	}

	return true;
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
