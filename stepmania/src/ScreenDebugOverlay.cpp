#include "global.h"
#include "ScreenDebugOverlay.h"
#include "ScreenDimensions.h"
#include "ScreenManager.h"
#include "PrefsManager.h"
#include "RageLog.h"
#include "GameState.h"
#include "PlayerState.h"
#include "StepMania.h"
#include "GameCommand.h"
#include "ScreenGameplay.h"
#include "RageSoundManager.h"
#include "GameSoundManager.h"
#include "RageTextureManager.h"
#include "MemoryCardManager.h"
#include "NoteSkinManager.h"
#include "Bookkeeper.h"
#include "ProfileManager.h"
#include "CodeDetector.h"

static bool g_bIsDisplayed = false;
static bool g_bIsSlow = false;
static bool g_bIsHalt = false;
static RageTimer g_HaltTimer(RageZeroTimer);

static bool IsGameplay()
{
	return SCREENMAN && SCREENMAN->GetTopScreen() && SCREENMAN->GetTopScreen()->GetScreenType() == gameplay;
}

REGISTER_SCREEN_CLASS( ScreenDebugOverlay );
ScreenDebugOverlay::ScreenDebugOverlay( const CString &sName ) : Screen(sName)
{
}

ScreenDebugOverlay::~ScreenDebugOverlay()
{
}

struct MapDebugToDI
{
	DeviceInput holdForDebug1;
	DeviceInput holdForDebug2;
	DeviceInput debugButton[NUM_DEBUG_LINES];
	DeviceInput gameplayButton[NUM_DEBUG_LINES];
	void Clear()
	{
		holdForDebug1.MakeInvalid();
		holdForDebug2.MakeInvalid();
		FOREACH_DebugLine(i)
		{
			debugButton[i].MakeInvalid();
			gameplayButton[i].MakeInvalid();
		}
	}
};
static MapDebugToDI g_Mappings;

static CString GetDebugButtonName( DebugLine i )
{
	// TODO: Make arch appropriate.
	vector<CString> v;
	if( g_Mappings.debugButton[i].IsValid() )
		v.push_back( g_Mappings.debugButton[i].toString() );
	if( g_Mappings.gameplayButton[i].IsValid() )
		v.push_back( g_Mappings.gameplayButton[i].toString()+" in gameplay" );
	return join( " or ", v );
}

void ScreenDebugOverlay::Init()
{
	Screen::Init();
	
	// Init debug mappings
	// TODO: Arch-specific?
	{
		g_Mappings.Clear();

		g_Mappings.holdForDebug1 = DeviceInput(DEVICE_KEYBOARD, KEY_F3);
		g_Mappings.holdForDebug2.MakeInvalid();

		g_Mappings.gameplayButton[0]	= DeviceInput(DEVICE_KEYBOARD, KEY_F8);
		g_Mappings.gameplayButton[1]	= DeviceInput(DEVICE_KEYBOARD, KEY_F7);
		g_Mappings.gameplayButton[2]	= DeviceInput(DEVICE_KEYBOARD, KEY_F6);
		g_Mappings.debugButton[3]  = DeviceInput(DEVICE_KEYBOARD, KEY_C1);
		g_Mappings.debugButton[4]  = DeviceInput(DEVICE_KEYBOARD, KEY_C2);
		g_Mappings.debugButton[5]  = DeviceInput(DEVICE_KEYBOARD, KEY_C3);
		g_Mappings.debugButton[6]  = DeviceInput(DEVICE_KEYBOARD, KEY_C4);
		g_Mappings.debugButton[7]  = DeviceInput(DEVICE_KEYBOARD, KEY_C5);
		g_Mappings.debugButton[8]  = DeviceInput(DEVICE_KEYBOARD, KEY_C6);
		g_Mappings.debugButton[9]  = DeviceInput(DEVICE_KEYBOARD, KEY_C7);
		g_Mappings.debugButton[10] = DeviceInput(DEVICE_KEYBOARD, KEY_C8);
		g_Mappings.debugButton[11] = DeviceInput(DEVICE_KEYBOARD, KEY_C9);
		g_Mappings.debugButton[12] = DeviceInput(DEVICE_KEYBOARD, KEY_C0);
		g_Mappings.debugButton[13] = DeviceInput(DEVICE_KEYBOARD, KEY_Cq);
		g_Mappings.debugButton[14] = DeviceInput(DEVICE_KEYBOARD, KEY_Cw);
		g_Mappings.debugButton[15] = DeviceInput(DEVICE_KEYBOARD, KEY_Ce);
		g_Mappings.debugButton[16] = DeviceInput(DEVICE_KEYBOARD, KEY_Cr);
		g_Mappings.debugButton[17] = DeviceInput(DEVICE_KEYBOARD, KEY_Ct);
		g_Mappings.debugButton[18] = DeviceInput(DEVICE_KEYBOARD, KEY_Cy);
		g_Mappings.debugButton[19] = DeviceInput(DEVICE_KEYBOARD, KEY_UP);
		g_Mappings.debugButton[20] = DeviceInput(DEVICE_KEYBOARD, KEY_DOWN);

	}


	m_Quad.StretchTo( RectF( 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT ) );
	m_Quad.SetDiffuse( RageColor(0, 0, 0, 0.5f) );
	this->AddChild( &m_Quad );
	
	m_textHeader.LoadFromFont( THEME->GetPathToF("Common normal") );
	m_textHeader.SetHorizAlign( Actor::align_left );
	m_textHeader.SetX( SCREEN_LEFT+20 );
	m_textHeader.SetY( SCREEN_TOP+20 );
	m_textHeader.SetZoom( 1.0f );
	m_textHeader.SetText( "Debug Menu" );
	this->AddChild( &m_textHeader );

	FOREACH_DebugLine( i )
	{
		BitmapText &txt1 = m_textButton[i];
		txt1.LoadFromFont( THEME->GetPathToF("Common normal") );
		txt1.SetHorizAlign( Actor::align_right );
		txt1.SetShadowLength( 2 );
		this->AddChild( &txt1 );

		BitmapText &txt2 = m_textFunction[i];
		txt2.LoadFromFont( THEME->GetPathToF("Common normal") );
		txt2.SetHorizAlign( Actor::align_left );
		txt2.SetShadowLength( 2 );
		this->AddChild( &txt2 );
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

	UpdateText();
}

void ScreenDebugOverlay::UpdateText()
{
	/* Highlight options that aren't the default. */
	const RageColor off(0.6f, 0.6f, 0.6f, 1.0f);
	const RageColor on(1, 1, 1, 1);
	
	FOREACH_DebugLine( i )
	{
		BitmapText &txt1 = m_textButton[i];
		txt1.SetX( SCREEN_CENTER_X-50 );
		txt1.SetY( SCALE(i, 0, NUM_DEBUG_LINES-1, SCREEN_TOP+60, SCREEN_BOTTOM-40) );
		txt1.SetZoom( 0.7f );

		BitmapText &txt2 = m_textFunction[i];
		txt2.SetX( SCREEN_CENTER_X-30 );
		txt2.SetY( SCALE(i, 0, NUM_DEBUG_LINES-1, SCREEN_TOP+60, SCREEN_BOTTOM-40) );
		txt2.SetZoom( 0.7f );

		CString s1;
		switch( i )
		{
		case DebugLine_Autoplay:			s1="AutoPlay";				break;
		case DebugLine_AssistTick:			s1="AssistTick";			break;
		case DebugLine_Autosync:			s1="AutoSync";				break;
		case DebugLine_CoinMode:			s1="CoinMode";				break;
		case DebugLine_Slow:				s1="Slow";					break;
		case DebugLine_Halt:				s1="Halt";					break;
		case DebugLine_LightsDebug:			s1="Lights Debug";			break;
		case DebugLine_MonkeyInput:			s1="MonkeyInput";			break;
		case DebugLine_Stats:				s1="Rendering Stats";		break;
		case DebugLine_Vsync:				s1="Vsync";					break;
		case DebugLine_ScreenTestMode:		s1="Screen Test Mode";		break;
		case DebugLine_ClearMachineStats:	s1="Clear Machine Stats";	break;
		case DebugLine_FillMachineStats:	s1="Fill Machine Stats";	break;
		case DebugLine_SendNotesEnded:		s1="Send Notes Ended";		break;
		case DebugLine_ReloadCurrentScreen:	s1="Reload";				break;
		case DebugLine_ReloadTheme:			s1="Reload Theme and Textures";	break;
		case DebugLine_WriteProfiles:		s1="Write Profiles";		break;
		case DebugLine_WritePreferences:	s1="Write Preferences";		break;
		case DebugLine_MenuTimer:			s1="Menu Timer";			break;
		case DebugLine_VolumeUp:			s1="Volume Up";				break;
		case DebugLine_VolumeDown:			s1="Volume Down";			break;
		case DebugLine_Uptime:				s1="Uptime";				break;
		default:	ASSERT(0);
		}

		bool bOn = false;
		switch( i )
		{
		case DebugLine_Autoplay:			bOn=PREFSMAN->m_AutoPlay.Get() != PC_HUMAN;		break;
		case DebugLine_AssistTick:			bOn=GAMESTATE->m_SongOptions.m_bAssistTick;		break;
		case DebugLine_Autosync:			bOn=GAMESTATE->m_SongOptions.m_AutosyncType!=SongOptions::AUTOSYNC_OFF;		break;
		case DebugLine_CoinMode:			bOn=true;								break;
		case DebugLine_Slow:				bOn=g_bIsSlow;							break;
		case DebugLine_Halt:				bOn=g_bIsHalt;							break;
		case DebugLine_LightsDebug:			bOn=PREFSMAN->m_bDebugLights.Get();		break;
		case DebugLine_MonkeyInput:			bOn=PREFSMAN->m_bMonkeyInput.Get();		break;
		case DebugLine_Stats:				bOn=PREFSMAN->m_bShowStats.Get();		break;
		case DebugLine_Vsync:				bOn=PREFSMAN->m_bVsync.Get();			break;
		case DebugLine_ScreenTestMode:		bOn=PREFSMAN->m_bScreenTestMode.Get();	break;
		case DebugLine_ClearMachineStats:	bOn=true;								break;
		case DebugLine_FillMachineStats:	bOn=true;								break;
		case DebugLine_SendNotesEnded:		bOn=true;								break;
		case DebugLine_ReloadCurrentScreen:	bOn=true;								break;
		case DebugLine_ReloadTheme:			bOn=true;								break;
		case DebugLine_WriteProfiles:		bOn=true;								break;
		case DebugLine_WritePreferences:	bOn=true;								break;
		case DebugLine_MenuTimer:			bOn=PREFSMAN->m_bMenuTimer.Get();		break;
		case DebugLine_VolumeUp:			bOn=true;								break;
		case DebugLine_VolumeDown:			bOn=true;								break;
		case DebugLine_Uptime:				bOn=false;								break;
		default:	ASSERT(0);
		}

		CString s2;
		switch( i )
		{
		case DebugLine_Autoplay:
			switch( PREFSMAN->m_AutoPlay )
			{
			case PC_HUMAN:		s2="off";	break;
			case PC_AUTOPLAY:	s2="on";	break;
			case PC_CPU:		s2="CPU";	break;
			default:	ASSERT(0);
			}
			break;
		case DebugLine_AssistTick:		s2=bOn ? "on":"off";	break;
		case DebugLine_Autosync:
			switch( GAMESTATE->m_SongOptions.m_AutosyncType )
			{
			case SongOptions::AUTOSYNC_OFF:		s2="off";		break;
			case SongOptions::AUTOSYNC_SONG:	s2="Song";		break;
			case SongOptions::AUTOSYNC_MACHINE:	s2="Machine";	break;
			default:	ASSERT(0);
			}
			break;
		case DebugLine_CoinMode:			s2=CoinModeToString(PREFSMAN->m_CoinMode);	break;
		case DebugLine_Slow:				s2=bOn ? "on":"off";	break;
		case DebugLine_Halt:				s2=bOn ? "on":"off";	break;
		case DebugLine_LightsDebug:			s2=bOn ? "on":"off";	break;
		case DebugLine_MonkeyInput:			s2=bOn ? "on":"off";	break;
		case DebugLine_Stats:				s2=bOn ? "on":"off";	break;
		case DebugLine_Vsync:				s2=bOn ? "on":"off";	break;
		case DebugLine_ScreenTestMode:		s2=bOn ? "on":"off";	break;
		case DebugLine_ClearMachineStats:	s2="";					break;
		case DebugLine_FillMachineStats:	s2="";					break;
		case DebugLine_SendNotesEnded:		s2="";					break;
		case DebugLine_ReloadCurrentScreen:	s2=SCREENMAN ? SCREENMAN->GetTopScreen()->GetName():"";	break;
		case DebugLine_ReloadTheme:			s2="";					break;
		case DebugLine_WriteProfiles:		s2="";					break;
		case DebugLine_WritePreferences:	s2="";					break;
		case DebugLine_MenuTimer:			s2="";					break;
		case DebugLine_VolumeUp:			s2=ssprintf("%.0f%%",PREFSMAN->m_fSoundVolume.Get()*100);	break;
		case DebugLine_VolumeDown:			s2="";					break;
		case DebugLine_Uptime:				s2=SecondsToMMSSMsMsMs(RageTimer::GetTimeSinceStart());	break;
		default:	ASSERT(0);
		}

		txt1.SetDiffuse( bOn ? on:off );
		txt2.SetDiffuse( bOn ? on:off );

		CString sButton = GetDebugButtonName(i);
		if( !sButton.empty() )
			sButton += ": ";
		txt1.SetText( sButton );
		if( !s2.empty() )
			s1 += " - ";
		txt2.SetText( s1 + s2 );
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
	if( DeviceI == g_Mappings.holdForDebug1 || 
		DeviceI == g_Mappings.holdForDebug2 )
	{
		bool bHoldingBoth =
			(!g_Mappings.holdForDebug1.IsValid() || INPUTFILTER->IsBeingPressed(g_Mappings.holdForDebug1)) &&
			(!g_Mappings.holdForDebug2.IsValid() || INPUTFILTER->IsBeingPressed(g_Mappings.holdForDebug2));
			
		if( bHoldingBoth )
			g_bIsDisplayed = true;
		else
			g_bIsDisplayed = false;
	}

	FOREACH_DebugLine( i )
	{
		if( (g_bIsDisplayed && DeviceI == g_Mappings.debugButton[i]) ||
			(IsGameplay() && DeviceI == g_Mappings.gameplayButton[i]) )
		{
			if( type != IET_FIRST_PRESS )
				return true; /* eat the input but do nothing */

			BitmapText &txt1 = m_textButton[i];
			txt1.FinishTweening();
			float fZoom = txt1.GetZoom();
			txt1.SetZoom( fZoom * 1.2f );
			txt1.BeginTweening( 0.2f, Actor::TWEEN_LINEAR );
			txt1.SetZoom( fZoom );

			BitmapText &txt2 = m_textFunction[i];

			switch( i )
			{
			case DebugLine_Autoplay:
				{
					PlayerController pc = (PlayerController)(PREFSMAN->m_AutoPlay+1);
					wrap( (int&)pc, NUM_PLAYER_CONTROLLERS );
					PREFSMAN->m_AutoPlay.Set( pc );
					FOREACH_HumanPlayer(pn)
						GAMESTATE->m_pPlayerState[pn]->m_PlayerController = PREFSMAN->m_AutoPlay;
				}
				break;
			case DebugLine_AssistTick:
				{
					if( type != IET_FIRST_PRESS )
						return true; /* eat the input but do nothing */
					GAMESTATE->m_SongOptions.m_bAssistTick = !GAMESTATE->m_SongOptions.m_bAssistTick;
					/* Store this change, so it sticks if we change songs: */
					GAMESTATE->m_StoredSongOptions.m_bAssistTick = GAMESTATE->m_SongOptions.m_bAssistTick;
					MESSAGEMAN->Broadcast( Message_AssistTickChanged );
				}
				break;
			case DebugLine_Autosync:
				{
					if( type != IET_FIRST_PRESS )
						return true; /* eat the input but do nothing */
					SongOptions::AutosyncType as = (SongOptions::AutosyncType)(GAMESTATE->m_SongOptions.m_AutosyncType+1);
					wrap( (int&)as, SongOptions::NUM_AUTOSYNC_TYPES );
					GAMESTATE->m_SongOptions.m_AutosyncType = as;
					MESSAGEMAN->Broadcast( Message_AutosyncChanged );
				}
				break;
			case DebugLine_CoinMode:
				{
					CoinMode cm = (CoinMode)(PREFSMAN->m_CoinMode+1);
					wrap( (int&)cm, NUM_COIN_MODES );
					PREFSMAN->m_CoinMode.Set( cm );
					SCREENMAN->RefreshCreditsMessages();
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
				PREFSMAN->m_bDebugLights.Set( !PREFSMAN->m_bDebugLights );
				break;
			case DebugLine_MonkeyInput:
				PREFSMAN->m_bMonkeyInput.Set( !PREFSMAN->m_bMonkeyInput );
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
			case DebugLine_ReloadCurrentScreen:
				SOUND->StopMusic();
				ResetGame();
				SCREENMAN->SetNewScreen( SCREENMAN->GetTopScreen()->GetName() );
				break;
			case DebugLine_ReloadTheme:
				THEME->ReloadMetrics();
				TEXTUREMAN->ReloadAll();
				NOTESKIN->RefreshNoteSkinData( GAMESTATE->m_pCurGame );
				CodeDetector::RefreshCacheItems();
				SCREENMAN->SystemMessage( "Theme reloaded." );
				// HACK: Don't update text below.  Return immediately because this screen.
				// was just destroyed as part of the them reload.
				return true;
			case DebugLine_WriteProfiles:
				// Also save bookkeeping and profile info for debugging
				// so we don't have to play through a whole song to get new output.
				BOOKKEEPER->WriteToDisk();
				PROFILEMAN->SaveMachineProfile();
				FOREACH_PlayerNumber( p )
				{
					if( !PROFILEMAN->IsPersistentProfile(p) )
						continue;

					bool bWasMemoryCard = PROFILEMAN->ProfileWasLoadedFromMemoryCard(p);
					if( bWasMemoryCard )
						MEMCARDMAN->MountCard( p );
					PROFILEMAN->SaveProfile( p );
					if( bWasMemoryCard )
						MEMCARDMAN->UnmountCard( p );
				}
				break;
			case DebugLine_WritePreferences:
				PREFSMAN->SaveGlobalPrefsToDisk();
				break;
			case DebugLine_MenuTimer:
				PREFSMAN->m_bMenuTimer.Set( !PREFSMAN->m_bMenuTimer );
				break;
			case DebugLine_VolumeUp:
			case DebugLine_VolumeDown:
				{
					float fVol = PREFSMAN->m_fSoundVolume;
					switch( i )
					{
					default:	ASSERT(0);
					case DebugLine_VolumeUp:	fVol += 0.1f;	break;
					case DebugLine_VolumeDown:	fVol -= 0.1f;	break;
					}
					CLAMP( fVol, 0.0f, 1.0f );
					PREFSMAN->m_fSoundVolume.Set( fVol );
					SOUNDMAN->SetPrefs( PREFSMAN->m_fSoundVolume );
				}
				break;
			case DebugLine_Uptime:
				break;
			default:	ASSERT(0);
			}
			
			UpdateText();

			SCREENMAN->SystemMessage( txt2.GetText() );

			return true;
		}
	}

	return false;
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
