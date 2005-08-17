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
#include "RageInput.h"

static bool g_bIsDisplayed = false;
static bool g_bIsSlow = false;
static bool g_bIsHalt = false;
static RageTimer g_HaltTimer(RageZeroTimer);


//
// self-registering debug lines
//
class IDebugLine;
static vector<IDebugLine*> *g_pvpSubscribers = NULL;
class IDebugLine
{
public:
	IDebugLine()
	{ 
		if( g_pvpSubscribers == NULL )
			g_pvpSubscribers = new vector<IDebugLine*>;
		g_pvpSubscribers->push_back( this );
	}
	virtual CString GetDescription() = 0;
	virtual CString GetValue() = 0;
	virtual bool IsEnabled() = 0;
	virtual void Do() = 0;
};


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
	this->RemoveAllChildren();

	FOREACH( BitmapText*, m_vptextButton, p )
		SAFE_DELETE( *p );
	m_vptextButton.clear();
	FOREACH( BitmapText*, m_vptextFunction, p )
		SAFE_DELETE( *p );
	m_vptextFunction.clear();
}

const int MAX_DEBUG_LINES = 30;
struct MapDebugToDI
{
	DeviceInput holdForDebug1;
	DeviceInput holdForDebug2;
	DeviceInput debugButton[MAX_DEBUG_LINES];
	DeviceInput gameplayButton[MAX_DEBUG_LINES];
	void Clear()
	{
		holdForDebug1.MakeInvalid();
		holdForDebug2.MakeInvalid();
		for( int i=0; i<MAX_DEBUG_LINES; i++ )
		{
			debugButton[i].MakeInvalid();
			gameplayButton[i].MakeInvalid();
		}
	}
};
static MapDebugToDI g_Mappings;

static CString GetDebugButtonName( int i )
{
	vector<CString> v;
	if( g_Mappings.debugButton[i].IsValid() )
		v.push_back( INPUTMAN->GetDeviceSpecificInputString(g_Mappings.debugButton[i]) );
	if( g_Mappings.gameplayButton[i].IsValid() )
		v.push_back( INPUTMAN->GetDeviceSpecificInputString(g_Mappings.gameplayButton[i])+" in gameplay" );
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
		g_Mappings.debugButton[19] = DeviceInput(DEVICE_KEYBOARD, KEY_Cu);
		g_Mappings.debugButton[20] = DeviceInput(DEVICE_KEYBOARD, KEY_UP);
		g_Mappings.debugButton[21] = DeviceInput(DEVICE_KEYBOARD, KEY_DOWN);

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

	FOREACH_CONST( IDebugLine*, *g_pvpSubscribers, p )
	{
		{
			BitmapText *pT1 = new BitmapText;
			pT1->LoadFromFont( THEME->GetPathToF("Common normal") );
			pT1->SetHorizAlign( Actor::align_right );
			pT1->SetText( "blah" );
			pT1->SetShadowLength( 2 );
			m_vptextButton.push_back( pT1 );
			this->AddChild( pT1 );
		}
		{
			BitmapText *pT2 = new BitmapText;
			pT2->LoadFromFont( THEME->GetPathToF("Common normal") );
			pT2->SetHorizAlign( Actor::align_left );
			pT2->SetText( "blah" );
			pT2->SetShadowLength( 2 );
			m_vptextFunction.push_back( pT2 );
			this->AddChild( pT2 );
		}
	}

	Update( 0 );
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
	
	const unsigned NUM_DEBUG_LINES = g_pvpSubscribers->size();
	FOREACH_CONST( IDebugLine*, *g_pvpSubscribers, p )
	{
		int i = p-g_pvpSubscribers->begin();

		BitmapText &txt1 = *m_vptextButton[i];
		txt1.SetX( SCREEN_CENTER_X-50 );
		txt1.SetY( SCALE(i, 0, NUM_DEBUG_LINES-1, SCREEN_TOP+60, SCREEN_BOTTOM-40) );
		txt1.SetZoom( 0.7f );

		BitmapText &txt2 = *m_vptextFunction[i];
		txt2.SetX( SCREEN_CENTER_X-30 );
		txt2.SetY( SCALE(i, 0, NUM_DEBUG_LINES-1, SCREEN_TOP+60, SCREEN_BOTTOM-40) );
		txt2.SetZoom( 0.7f );

		CString s1 = (*p)->GetDescription();
		CString s2 = (*p)->GetValue();

		bool bOn = (*p)->IsEnabled();

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

	FOREACH_CONST( IDebugLine*, *g_pvpSubscribers, p )
	{
		int i = p-g_pvpSubscribers->begin();

		if( (g_bIsDisplayed && DeviceI == g_Mappings.debugButton[i]) ||
			(IsGameplay() && DeviceI == g_Mappings.gameplayButton[i]) )
		{
			if( type != IET_FIRST_PRESS )
				return true; /* eat the input but do nothing */

			BitmapText &txt1 = *m_vptextButton[i];
			txt1.FinishTweening();
			float fZoom = txt1.GetZoom();
			txt1.SetZoom( fZoom * 1.2f );
			txt1.BeginTweening( 0.2f, Actor::TWEEN_LINEAR );
			txt1.SetZoom( fZoom );

			BitmapText &txt2 = *m_vptextFunction[i];

			(*p)->Do();

			UpdateText();

			SCREENMAN->SystemMessage( txt2.GetText() );

			return true;
		}
	}

	return false;
}


//
// DebugLine helpers
//
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

void ChangeVolume( float fDelta )
{
	float fVol = PREFSMAN->m_fSoundVolume;
	fVol += fDelta;
	CLAMP( fVol, 0.0f, 1.0f );
	PREFSMAN->m_fSoundVolume.Set( fVol );
	SOUNDMAN->SetPrefs( PREFSMAN->m_fSoundVolume );
}



//
// DebugLines
//
class DebugLineAutoplay : public IDebugLine
{
	virtual CString GetDescription() { return "AutoPlay"; }
	virtual CString GetValue()
	{
		switch( PREFSMAN->m_AutoPlay )
		{
		case PC_HUMAN:		return "off";	break;
		case PC_AUTOPLAY:	return "on";	break;
		case PC_CPU:		return "CPU";	break;
		default:	ASSERT(0);	return NULL;
		}
	}
	virtual bool IsEnabled() { return PREFSMAN->m_AutoPlay.Get() != PC_HUMAN; }
	virtual void Do()
	{
		PlayerController pc = (PlayerController)(PREFSMAN->m_AutoPlay+1);
		wrap( (int&)pc, NUM_PLAYER_CONTROLLERS );
		PREFSMAN->m_AutoPlay.Set( pc );
		FOREACH_HumanPlayer(pn)
			GAMESTATE->m_pPlayerState[pn]->m_PlayerController = PREFSMAN->m_AutoPlay;
	}
};
static DebugLineAutoplay g_DebugLineAutoplay;

class DebugLineAssistTick : public IDebugLine
{
	virtual CString GetDescription() { return "AssistTick"; }
	virtual CString GetValue() { return IsEnabled() ? "on":"off"; }
	virtual bool IsEnabled() { return GAMESTATE->m_SongOptions.m_bAssistTick; }
	virtual void Do()
	{
		GAMESTATE->m_SongOptions.m_bAssistTick = !GAMESTATE->m_SongOptions.m_bAssistTick;
		// Store this change, so it sticks if we change songs
		GAMESTATE->m_StoredSongOptions.m_bAssistTick = GAMESTATE->m_SongOptions.m_bAssistTick;
		MESSAGEMAN->Broadcast( Message_AssistTickChanged );
	}
};
static DebugLineAssistTick g_DebugLineAssistTick;

class DebugLineAutosync : public IDebugLine
{
	virtual CString GetDescription() { return "Autosync"; }
	virtual CString GetValue()
	{ 
		switch( GAMESTATE->m_SongOptions.m_AutosyncType )
		{
		case SongOptions::AUTOSYNC_OFF:		return "off";		break;
		case SongOptions::AUTOSYNC_SONG:	return "Song";		break;
		case SongOptions::AUTOSYNC_MACHINE:	return "Machine";	break;
		default:	ASSERT(0);
		}
	}
	virtual bool IsEnabled() { return GAMESTATE->m_SongOptions.m_AutosyncType!=SongOptions::AUTOSYNC_OFF; }
	virtual void Do()
	{
	}
};
static DebugLineAutosync g_DebugLineAutosync;

class DebugLineCoinMode : public IDebugLine
{
	virtual CString GetDescription() { return "CoinMode"; }
	virtual CString GetValue() { return CoinModeToString(PREFSMAN->m_CoinMode); }
	virtual bool IsEnabled() { return true; }
	virtual void Do()
	{
		CoinMode cm = (CoinMode)(PREFSMAN->m_CoinMode+1);
		wrap( (int&)cm, NUM_COIN_MODES );
		PREFSMAN->m_CoinMode.Set( cm );
		SCREENMAN->RefreshCreditsMessages();
	}
};
static DebugLineCoinMode g_DebugLineCoinMode;

class DebugLineSlow : public IDebugLine
{
	virtual CString GetDescription() { return "Slow"; }
	virtual CString GetValue() { return IsEnabled() ? "on":"off"; }
	virtual bool IsEnabled() { return g_bIsSlow; }
	virtual void Do()
	{
		g_bIsSlow = !g_bIsSlow;
		SetSpeed();
	}
};
static DebugLineSlow g_DebugLineSlow;

class DebugLineHalt : public IDebugLine
{
	virtual CString GetDescription() { return "Halt"; }
	virtual CString GetValue() { return IsEnabled() ? "on":"off"; }
	virtual bool IsEnabled() { return g_bIsHalt; }
	virtual void Do()
	{
		g_bIsHalt = !g_bIsHalt;
		g_HaltTimer.Touch();
		SetSpeed();
	}
};
static DebugLineHalt g_DebugLineHalt;

class DebugLineLightsDebug : public IDebugLine
{
	virtual CString GetDescription() { return "Lights Debug"; }
	virtual CString GetValue() { return IsEnabled() ? "on":"off"; }
	virtual bool IsEnabled() { return PREFSMAN->m_bDebugLights.Get(); }
	virtual void Do()
	{
		PREFSMAN->m_bDebugLights.Set( !PREFSMAN->m_bDebugLights );
	}
};
static DebugLineLightsDebug g_DebugLineLightsDebug;

class DebugLineMonkeyInput : public IDebugLine
{
	virtual CString GetDescription() { return "MonkeyInput"; }
	virtual CString GetValue() { return IsEnabled() ? "on":"off"; }
	virtual bool IsEnabled() { return PREFSMAN->m_bMonkeyInput.Get(); }
	virtual void Do()
	{
		PREFSMAN->m_bMonkeyInput.Set( !PREFSMAN->m_bMonkeyInput );
	}
};
static DebugLineMonkeyInput g_DebugLineMonkeyInput;

class DebugLineStats : public IDebugLine
{
	virtual CString GetDescription() { return "Rendering Stats"; }
	virtual CString GetValue() { return IsEnabled() ? "on":"off"; }
	virtual bool IsEnabled() { return PREFSMAN->m_bShowStats.Get(); }
	virtual void Do()
	{
		PREFSMAN->m_bShowStats.Set( !PREFSMAN->m_bShowStats );
	}
};
static DebugLineStats g_DebugLineStats;

class DebugLineVsync : public IDebugLine
{
	virtual CString GetDescription() { return "Vsync"; }
	virtual CString GetValue() { return IsEnabled() ? "on":"off"; }
	virtual bool IsEnabled() { return PREFSMAN->m_bVsync.Get(); }
	virtual void Do()
	{
		PREFSMAN->m_bVsync.Set( !PREFSMAN->m_bVsync );
		ApplyGraphicOptions();
	}
};
static DebugLineVsync g_DebugLineVsync;

class DebugLineScreenTestMode : public IDebugLine
{
	virtual CString GetDescription() { return "Screen Test Mode"; }
	virtual CString GetValue() { return IsEnabled() ? "on":"off"; }
	virtual bool IsEnabled() { return PREFSMAN->m_bScreenTestMode.Get(); }
	virtual void Do()
	{
		PREFSMAN->m_bScreenTestMode.Set( !PREFSMAN->m_bScreenTestMode );
	}
};
static DebugLineScreenTestMode g_DebugLineScreenTestMode;

class DebugLineClearMachineStats : public IDebugLine
{
	virtual CString GetDescription() { return "Clear Machine Stats"; }
	virtual CString GetValue() { return NULL; }
	virtual bool IsEnabled() { return true; }
	virtual void Do()
	{
		GameCommand gc;
		gc.Load( 0, ParseCommands("ClearMachineStats") );
		gc.ApplyToAllPlayers();
	}
};
static DebugLineClearMachineStats g_DebugClearMachineStats;

class DebugLineFillMachineStats : public IDebugLine
{
	virtual CString GetDescription() { return "Fill Machine Stats"; }
	virtual CString GetValue() { return NULL; }
	virtual bool IsEnabled() { return true; }
	virtual void Do()
	{
		GameCommand gc;
		gc.Load( 0, ParseCommands("FillMachineStats") );
		gc.ApplyToAllPlayers();
	}
};
static DebugLineFillMachineStats g_DebugLineFillMachineStats;

class DebugLineSendNotesEnded : public IDebugLine
{
	virtual CString GetDescription() { return "Send Notes Ended"; }
	virtual CString GetValue() { return NULL; }
	virtual bool IsEnabled() { return true; }
	virtual void Do()
	{
		SCREENMAN->PostMessageToTopScreen( SM_NotesEnded, 0 );
	}
};
static DebugLineSendNotesEnded g_DebugLineSendNotesEnded;

class DebugLineReloadCurrentScreen : public IDebugLine
{
	virtual CString GetDescription() { return "Reload"; }
	virtual CString GetValue() { return SCREENMAN ? SCREENMAN->GetTopScreen()->GetName() : NULL; }
	virtual bool IsEnabled() { return true; }
	virtual void Do()
	{
		SOUND->StopMusic();
		ResetGame();
		SCREENMAN->SetNewScreen( SCREENMAN->GetTopScreen()->GetName() );
	}
};
static DebugLineStats g_DebugLineReloadCurrentScreen;

class DebugLineReloadTheme : public IDebugLine
{
	virtual CString GetDescription() { return "Reload Theme and Textures"; }
	virtual CString GetValue() { return NULL; }
	virtual bool IsEnabled() { return true; }
	virtual void Do()
	{
		THEME->ReloadMetrics();
		TEXTUREMAN->ReloadAll();
		NOTESKIN->RefreshNoteSkinData( GAMESTATE->m_pCurGame );
		CodeDetector::RefreshCacheItems();
		SCREENMAN->SystemMessage( "Theme reloaded." );
		// HACK: Don't update text below.  Return immediately because this screen.
		// was just destroyed as part of the them reload.
	}
};
static DebugLineReloadTheme g_DebugLineReloadTheme;

class DebugLineWriteProfiles : public IDebugLine
{
	virtual CString GetDescription() { return "Write Profiles"; }
	virtual CString GetValue() { return NULL; }
	virtual bool IsEnabled() { return true; }
	virtual void Do()
	{
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
	}
};
static DebugLineWriteProfiles g_DebugLineWriteProfiles;

class DebugLineWritePreferences : public IDebugLine
{
	virtual CString GetDescription() { return "Write Preferences"; }
	virtual CString GetValue() { return NULL; }
	virtual bool IsEnabled() { return true; }
	virtual void Do()
	{
		PREFSMAN->SaveGlobalPrefsToDisk();
	}
};
static DebugLineStats g_DebugLineWritePreferences;

class DebugLineMenuTimer : public IDebugLine
{
	virtual CString GetDescription() { return "Menu Timer"; }
	virtual CString GetValue() { return NULL; }
	virtual bool IsEnabled() { return PREFSMAN->m_bMenuTimer.Get(); }
	virtual void Do()
	{
		PREFSMAN->m_bMenuTimer.Set( !PREFSMAN->m_bMenuTimer );
	}
};
static DebugLineMenuTimer g_DebugLineMenuTimer;

class DebugLineFlushLog : public IDebugLine
{
	virtual CString GetDescription() { return "Flush Log"; }
	virtual CString GetValue() { return NULL; }
	virtual bool IsEnabled() { return true; }
	virtual void Do()
	{
		LOG->Flush();
	}
};
static DebugLineFlushLog g_DebugLineFlushLog;

class DebugLineVolumeUp : public IDebugLine
{
	virtual CString GetDescription() { return "Volume Up"; }
	virtual CString GetValue() { return ssprintf("%.0f%%",PREFSMAN->m_fSoundVolume.Get()*100); }
	virtual bool IsEnabled() { return true; }
	virtual void Do()
	{
		ChangeVolume( +0.1f );
	}
};
static DebugLineVolumeUp g_DebugLineVolumeUp;

class DebugLineVolumeDown : public IDebugLine
{
	virtual CString GetDescription() { return "Volume Down"; }
	virtual CString GetValue() { return NULL; }
	virtual bool IsEnabled() { return true; }
	virtual void Do()
	{
		ChangeVolume( -0.1f );
	}
};
static DebugLineVolumeDown g_DebugLineVolumeDown;

class DebugLineUptime : public IDebugLine
{
	virtual CString GetDescription() { return "Uptime"; }
	virtual CString GetValue() { return SecondsToMMSSMsMsMs(RageTimer::GetTimeSinceStart()); }
	virtual bool IsEnabled() { return false; }
	virtual void Do() {}
};
static DebugLineUptime g_DebugLineUptime;


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
