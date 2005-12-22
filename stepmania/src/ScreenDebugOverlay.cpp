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
#include "RageDisplay.h"
#include "InputEventPlus.h"
#include "LocalizedString.h"

static bool g_bIsDisplayed = false;
static bool g_bIsSlow = false;
static bool g_bIsHalt = false;
static RageTimer g_HaltTimer(RageZeroTimer);
static float g_fImageScaleCurrent = 1;
static float g_fImageScaleDestination = 1;

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
	virtual void Do( CString &sMessageOut )
	{
		CString s1 = GetDescription();
		CString s2 = GetValue();
		if( !s2.empty() )
			s1 += " - ";
		sMessageOut = s1 + s2;
	};
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

		int i=0;
		g_Mappings.gameplayButton[i++]	= DeviceInput(DEVICE_KEYBOARD, KEY_F8);
		g_Mappings.gameplayButton[i++]	= DeviceInput(DEVICE_KEYBOARD, KEY_F7);
		g_Mappings.gameplayButton[i++]	= DeviceInput(DEVICE_KEYBOARD, KEY_F6);
		g_Mappings.debugButton[i++] = DeviceInput(DEVICE_KEYBOARD, KEY_C1);
		g_Mappings.debugButton[i++] = DeviceInput(DEVICE_KEYBOARD, KEY_C2);
		g_Mappings.debugButton[i++] = DeviceInput(DEVICE_KEYBOARD, KEY_C3);
		g_Mappings.debugButton[i++] = DeviceInput(DEVICE_KEYBOARD, KEY_C4);
		g_Mappings.debugButton[i++] = DeviceInput(DEVICE_KEYBOARD, KEY_C5);
		g_Mappings.debugButton[i++] = DeviceInput(DEVICE_KEYBOARD, KEY_C6);
		g_Mappings.debugButton[i++] = DeviceInput(DEVICE_KEYBOARD, KEY_C7);
		g_Mappings.debugButton[i++] = DeviceInput(DEVICE_KEYBOARD, KEY_C8);
		g_Mappings.debugButton[i++] = DeviceInput(DEVICE_KEYBOARD, KEY_C9);
		g_Mappings.debugButton[i++] = DeviceInput(DEVICE_KEYBOARD, KEY_C0);
		g_Mappings.debugButton[i++] = DeviceInput(DEVICE_KEYBOARD, KEY_Cq);
		g_Mappings.debugButton[i++] = DeviceInput(DEVICE_KEYBOARD, KEY_Cw);
		g_Mappings.debugButton[i++] = DeviceInput(DEVICE_KEYBOARD, KEY_Ce);
		g_Mappings.debugButton[i++] = DeviceInput(DEVICE_KEYBOARD, KEY_Cr);
		g_Mappings.debugButton[i++] = DeviceInput(DEVICE_KEYBOARD, KEY_Ct);
		g_Mappings.debugButton[i++] = DeviceInput(DEVICE_KEYBOARD, KEY_Cy);
		g_Mappings.debugButton[i++] = DeviceInput(DEVICE_KEYBOARD, KEY_Cu);
		g_Mappings.debugButton[i++] = DeviceInput(DEVICE_KEYBOARD, KEY_Ci);
		g_Mappings.debugButton[i++] = DeviceInput(DEVICE_KEYBOARD, KEY_Co);
		g_Mappings.debugButton[i++] = DeviceInput(DEVICE_KEYBOARD, KEY_UP);
		g_Mappings.debugButton[i++] = DeviceInput(DEVICE_KEYBOARD, KEY_DOWN);

	}


	m_Quad.StretchTo( RectF( 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT ) );
	m_Quad.SetDiffuse( RageColor(0, 0, 0, 0.5f) );
	this->AddChild( &m_Quad );
	
	m_textHeader.LoadFromFont( THEME->GetPathF("Common", "normal") );
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
			pT1->LoadFromFont( THEME->GetPathF("Common", "normal") );
			pT1->SetHorizAlign( Actor::align_right );
			pT1->SetText( "blah" );
			pT1->SetShadowLength( 2 );
			m_vptextButton.push_back( pT1 );
			this->AddChild( pT1 );
		}
		{
			BitmapText *pT2 = new BitmapText;
			pT2->LoadFromFont( THEME->GetPathF("Common", "normal") );
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
	bool bCenteringNeedsUpdate = g_fImageScaleCurrent != g_fImageScaleDestination;
	fapproach( g_fImageScaleCurrent, g_fImageScaleDestination, fDeltaTime );
	if( bCenteringNeedsUpdate )
	{
		DISPLAY->ChangeCentering(
			PREFSMAN->m_iCenterImageTranslateX, 
			PREFSMAN->m_iCenterImageTranslateY,
			PREFSMAN->m_fCenterImageAddWidth - (int)SCREEN_WIDTH + (int)(g_fImageScaleCurrent*SCREEN_WIDTH),
			PREFSMAN->m_fCenterImageAddHeight - (int)SCREEN_HEIGHT + (int)(g_fImageScaleCurrent*SCREEN_HEIGHT) );
	}

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

bool ScreenDebugOverlay::OverlayInput( const InputEventPlus &input )
{
	if( input.DeviceI == g_Mappings.holdForDebug1 || 
		input.DeviceI == g_Mappings.holdForDebug2 )
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

		if( (g_bIsDisplayed && input.DeviceI == g_Mappings.debugButton[i]) ||
			(IsGameplay() && input.DeviceI == g_Mappings.gameplayButton[i]) )
		{
			if( input.type != IET_FIRST_PRESS )
				return true; /* eat the input but do nothing */

			BitmapText &txt1 = *m_vptextButton[i];
			txt1.FinishTweening();
			float fZoom = txt1.GetZoom();
			txt1.SetZoom( fZoom * 1.2f );
			txt1.BeginTweening( 0.2f, Actor::TWEEN_LINEAR );
			txt1.SetZoom( fZoom );

			CString sMessage;
			(*p)->Do( sMessage );
			if( !sMessage.empty() )
				SCREENMAN->SystemMessage( sMessage );

			UpdateText();
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
	SOUNDMAN->SetMixVolume( PREFSMAN->m_fSoundVolume );
}



//
// DebugLines
//
#define DECLARE_ONE( x ) static x g_##x


static LocalizedString AUTO_PLAY		( "ScreenDebugOverlay", "AutoPlay" );
static LocalizedString ASSIST_TICK		( "ScreenDebugOverlay", "AssistTick" );
static LocalizedString AUTOSYNC			( "ScreenDebugOverlay", "Autosync" );
static LocalizedString COIN_MODE		( "ScreenDebugOverlay", "CoinMode" );
static LocalizedString HALT				( "ScreenDebugOverlay", "Halt" );
static LocalizedString LIGHTS_DEBUG		( "ScreenDebugOverlay", "Lights Debug" );
static LocalizedString MONKEY_INPUT		( "ScreenDebugOverlay", "Monkey Input" );
static LocalizedString RENDERING_STATS	( "ScreenDebugOverlay", "Rendering Stats" );
static LocalizedString VSYNC			( "ScreenDebugOverlay", "Vsync" );
static LocalizedString MULTITEXTURE		( "ScreenDebugOverlay", "Multitexture" );
static LocalizedString SCREEN_TEST_MODE	( "ScreenDebugOverlay", "Screen Test Mode" );
static LocalizedString CLEAR_MACHINE_STATS	( "ScreenDebugOverlay", "Clear Machine Stats" );
static LocalizedString FILL_MACHINE_STATS	( "ScreenDebugOverlay", "Fill Machine Stats" );
static LocalizedString SEND_NOTES_ENDED		( "ScreenDebugOverlay", "Send Notes Ended" );
static LocalizedString RELOAD				( "ScreenDebugOverlay", "Reload" );
static LocalizedString RELOAD_THEME_AND_TEXTURES	( "ScreenDebugOverlay", "Reload Theme and Textures" );
static LocalizedString WRITE_PROFILES		( "ScreenDebugOverlay", "Write Profiles" );
static LocalizedString WRITE_PREFERENCES	( "ScreenDebugOverlay", "Write Preferences" );
static LocalizedString MENU_TIMER			( "ScreenDebugOverlay", "Menu Timer" );
static LocalizedString FLUSH_LOG			( "ScreenDebugOverlay", "Flush Log" );
static LocalizedString PULL_BACK_CAMERA		( "ScreenDebugOverlay", "Pull Back Camera" );
static LocalizedString VOLUME_UP			( "ScreenDebugOverlay", "Volume Up" );
static LocalizedString VOLUME_DOWN			( "ScreenDebugOverlay", "Volume Down" );
static LocalizedString UPTIME				( "ScreenDebugOverlay", "Uptime" );
static LocalizedString SLOW		( "ScreenDebugOverlay", "Slow" );
static LocalizedString ON		( "ScreenDebugOverlay", "on" );
static LocalizedString OFF		( "ScreenDebugOverlay", "off" );
static LocalizedString CPU		( "ScreenDebugOverlay", "CPU" );
static LocalizedString SONG		( "ScreenDebugOverlay", "Song" );
static LocalizedString MACHINE	( "ScreenDebugOverlay", "Machine" );


class DebugLineAutoplay : public IDebugLine
{
	virtual CString GetDescription() { return AUTO_PLAY; }
	virtual CString GetValue()
	{
		switch( PREFSMAN->m_AutoPlay )
		{
		case PC_HUMAN:		return OFF;	break;
		case PC_AUTOPLAY:	return ON;	break;
		case PC_CPU:		return CPU;	break;
		default:	ASSERT(0);	return NULL;
		}
	}
	virtual bool IsEnabled() { return PREFSMAN->m_AutoPlay.Get() != PC_HUMAN; }
	virtual void Do( CString &sMessageOut )
	{
		PlayerController pc = (PlayerController)(PREFSMAN->m_AutoPlay+1);
		wrap( (int&)pc, NUM_PLAYER_CONTROLLERS );
		PREFSMAN->m_AutoPlay.Set( pc );
		FOREACH_HumanPlayer(p)
			GAMESTATE->m_pPlayerState[p]->m_PlayerController = PREFSMAN->m_AutoPlay;
		FOREACH_MultiPlayer(p)
			GAMESTATE->m_pMultiPlayerState[p]->m_PlayerController = PREFSMAN->m_AutoPlay;
		IDebugLine::Do( sMessageOut );
	}
};

class DebugLineAssistTick : public IDebugLine
{
	virtual CString GetDescription() { return ASSIST_TICK; }
	virtual CString GetValue() { return IsEnabled() ? ON:OFF; }
	virtual bool IsEnabled() { return GAMESTATE->m_SongOptions.m_bAssistTick; }
	virtual void Do( CString &sMessageOut )
	{
		GAMESTATE->m_SongOptions.m_bAssistTick = !GAMESTATE->m_SongOptions.m_bAssistTick;
		// Store this change, so it sticks if we change songs
		GAMESTATE->m_StoredSongOptions.m_bAssistTick = GAMESTATE->m_SongOptions.m_bAssistTick;
		MESSAGEMAN->Broadcast( Message_AssistTickChanged );
		IDebugLine::Do( sMessageOut );
	}
};

class DebugLineAutosync : public IDebugLine
{
	virtual CString GetDescription() { return AUTOSYNC; }
	virtual CString GetValue()
	{ 
		switch( GAMESTATE->m_SongOptions.m_AutosyncType )
		{
		case SongOptions::AUTOSYNC_OFF:		return OFF;		break;
		case SongOptions::AUTOSYNC_SONG:	return SONG;		break;
		case SongOptions::AUTOSYNC_MACHINE:	return MACHINE;	break;
		default:	ASSERT(0);
		}
	}
	virtual bool IsEnabled() { return GAMESTATE->m_SongOptions.m_AutosyncType!=SongOptions::AUTOSYNC_OFF; }
	virtual void Do( CString &sMessageOut )
	{
		SongOptions::AutosyncType as = (SongOptions::AutosyncType)(GAMESTATE->m_SongOptions.m_AutosyncType+1);
		wrap( (int&)as, SongOptions::NUM_AUTOSYNC_TYPES );
		GAMESTATE->m_SongOptions.m_AutosyncType = as;
		MESSAGEMAN->Broadcast( Message_AutosyncChanged );
		IDebugLine::Do( sMessageOut );
	}
};

class DebugLineCoinMode : public IDebugLine
{
	virtual CString GetDescription() { return COIN_MODE; }
	virtual CString GetValue() { return CoinModeToString(PREFSMAN->m_CoinMode); }
	virtual bool IsEnabled() { return true; }
	virtual void Do( CString &sMessageOut )
	{
		CoinMode cm = (CoinMode)(PREFSMAN->m_CoinMode+1);
		wrap( (int&)cm, NUM_COIN_MODES );
		PREFSMAN->m_CoinMode.Set( cm );
		SCREENMAN->RefreshCreditsMessages();
		IDebugLine::Do( sMessageOut );
	}
};

class DebugLineSlow : public IDebugLine
{
	virtual CString GetDescription() { return SLOW; }
	virtual CString GetValue() { return IsEnabled() ? ON:OFF; }
	virtual bool IsEnabled() { return g_bIsSlow; }
	virtual void Do( CString &sMessageOut )
	{
		g_bIsSlow = !g_bIsSlow;
		SetSpeed();
		IDebugLine::Do( sMessageOut );
	}
};

class DebugLineHalt : public IDebugLine
{
	virtual CString GetDescription() { return HALT; }
	virtual CString GetValue() { return IsEnabled() ? ON:OFF; }
	virtual bool IsEnabled() { return g_bIsHalt; }
	virtual void Do( CString &sMessageOut )
	{
		g_bIsHalt = !g_bIsHalt;
		g_HaltTimer.Touch();
		SetSpeed();
		IDebugLine::Do( sMessageOut );
	}
};

class DebugLineLightsDebug : public IDebugLine
{
	virtual CString GetDescription() { return LIGHTS_DEBUG; }
	virtual CString GetValue() { return IsEnabled() ? ON:OFF; }
	virtual bool IsEnabled() { return PREFSMAN->m_bDebugLights.Get(); }
	virtual void Do( CString &sMessageOut )
	{
		PREFSMAN->m_bDebugLights.Set( !PREFSMAN->m_bDebugLights );
		IDebugLine::Do( sMessageOut );
	}
};

class DebugLineMonkeyInput : public IDebugLine
{
	virtual CString GetDescription() { return MONKEY_INPUT; }
	virtual CString GetValue() { return IsEnabled() ? ON:OFF; }
	virtual bool IsEnabled() { return PREFSMAN->m_bMonkeyInput.Get(); }
	virtual void Do( CString &sMessageOut )
	{
		PREFSMAN->m_bMonkeyInput.Set( !PREFSMAN->m_bMonkeyInput );
		IDebugLine::Do( sMessageOut );
	}
};

class DebugLineStats : public IDebugLine
{
	virtual CString GetDescription() { return RENDERING_STATS; }
	virtual CString GetValue() { return IsEnabled() ? ON:OFF; }
	virtual bool IsEnabled() { return PREFSMAN->m_bShowStats.Get(); }
	virtual void Do( CString &sMessageOut )
	{
		PREFSMAN->m_bShowStats.Set( !PREFSMAN->m_bShowStats );
		IDebugLine::Do( sMessageOut );
	}
};

class DebugLineVsync : public IDebugLine
{
	virtual CString GetDescription() { return VSYNC; }
	virtual CString GetValue() { return IsEnabled() ? ON:OFF; }
	virtual bool IsEnabled() { return PREFSMAN->m_bVsync.Get(); }
	virtual void Do( CString &sMessageOut )
	{
		PREFSMAN->m_bVsync.Set( !PREFSMAN->m_bVsync );
		StepMania::ApplyGraphicOptions();
		IDebugLine::Do( sMessageOut );
	}
};

class DebugLineAllowMultitexture : public IDebugLine
{
	virtual CString GetDescription() { return MULTITEXTURE; }
	virtual CString GetValue() { return IsEnabled() ? ON:OFF; }
	virtual bool IsEnabled() { return PREFSMAN->m_bAllowMultitexture.Get(); }
	virtual void Do( CString &sMessageOut )
	{
		PREFSMAN->m_bAllowMultitexture.Set( !PREFSMAN->m_bAllowMultitexture );
		IDebugLine::Do( sMessageOut );
	}
};

class DebugLineScreenTestMode : public IDebugLine
{
	virtual CString GetDescription() { return SCREEN_TEST_MODE; }
	virtual CString GetValue() { return IsEnabled() ? ON:OFF; }
	virtual bool IsEnabled() { return PREFSMAN->m_bScreenTestMode.Get(); }
	virtual void Do( CString &sMessageOut )
	{
		PREFSMAN->m_bScreenTestMode.Set( !PREFSMAN->m_bScreenTestMode );
		IDebugLine::Do( sMessageOut );
	}
};

class DebugLineClearMachineStats : public IDebugLine
{
	virtual CString GetDescription() { return CLEAR_MACHINE_STATS; }
	virtual CString GetValue() { return NULL; }
	virtual bool IsEnabled() { return true; }
	virtual void Do( CString &sMessageOut )
	{
		GameCommand gc;
		gc.Load( 0, ParseCommands("ClearMachineStats") );
		gc.ApplyToAllPlayers();
		IDebugLine::Do( sMessageOut );
	}
};

class DebugLineFillMachineStats : public IDebugLine
{
	virtual CString GetDescription() { return FILL_MACHINE_STATS; }
	virtual CString GetValue() { return NULL; }
	virtual bool IsEnabled() { return true; }
	virtual void Do( CString &sMessageOut )
	{
		GameCommand gc;
		gc.Load( 0, ParseCommands("FillMachineStats") );
		gc.ApplyToAllPlayers();
		IDebugLine::Do( sMessageOut );
	}
};

class DebugLineSendNotesEnded : public IDebugLine
{
	virtual CString GetDescription() { return SEND_NOTES_ENDED; }
	virtual CString GetValue() { return NULL; }
	virtual bool IsEnabled() { return true; }
	virtual void Do( CString &sMessageOut )
	{
		SCREENMAN->PostMessageToTopScreen( SM_NotesEnded, 0 );
		IDebugLine::Do( sMessageOut );
	}
};

class DebugLineReloadCurrentScreen : public IDebugLine
{
	virtual CString GetDescription() { return RELOAD; }
	virtual CString GetValue() { return SCREENMAN ? SCREENMAN->GetTopScreen()->GetName() : NULL; }
	virtual bool IsEnabled() { return true; }
	virtual void Do( CString &sMessageOut )
	{
		SOUND->StopMusic();
		StepMania::ResetGame();
		SCREENMAN->SetNewScreen( SCREENMAN->GetTopScreen()->GetName() );
		IDebugLine::Do( sMessageOut );
		sMessageOut = "";
	}
};

class DebugLineReloadTheme : public IDebugLine
{
	virtual CString GetDescription() { return RELOAD_THEME_AND_TEXTURES; }
	virtual CString GetValue() { return NULL; }
	virtual bool IsEnabled() { return true; }
	virtual void Do( CString &sMessageOut )
	{
		THEME->ReloadMetrics();
		TEXTUREMAN->ReloadAll();
		NOTESKIN->RefreshNoteSkinData( GAMESTATE->m_pCurGame );
		CodeDetector::RefreshCacheItems();
		// HACK: Don't update text below.  Return immediately because this screen.
		// was just destroyed as part of the them reload.
		IDebugLine::Do( sMessageOut );
	}
};

class DebugLineWriteProfiles : public IDebugLine
{
	virtual CString GetDescription() { return WRITE_PROFILES; }
	virtual CString GetValue() { return NULL; }
	virtual bool IsEnabled() { return true; }
	virtual void Do( CString &sMessageOut )
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
		IDebugLine::Do( sMessageOut );
	}
};

class DebugLineWritePreferences : public IDebugLine
{
	virtual CString GetDescription() { return "Write Preferences"; }
	virtual CString GetValue() { return NULL; }
	virtual bool IsEnabled() { return true; }
	virtual void Do( CString &sMessageOut )
	{
		PREFSMAN->SavePrefsToDisk();
		IDebugLine::Do( sMessageOut );
	}
};

class DebugLineMenuTimer : public IDebugLine
{
	virtual CString GetDescription() { return MENU_TIMER; }
	virtual CString GetValue() { return NULL; }
	virtual bool IsEnabled() { return PREFSMAN->m_bMenuTimer.Get(); }
	virtual void Do( CString &sMessageOut )
	{
		PREFSMAN->m_bMenuTimer.Set( !PREFSMAN->m_bMenuTimer );
		IDebugLine::Do( sMessageOut );
	}
};

class DebugLineFlushLog : public IDebugLine
{
	virtual CString GetDescription() { return FLUSH_LOG; }
	virtual CString GetValue() { return NULL; }
	virtual bool IsEnabled() { return true; }
	virtual void Do( CString &sMessageOut )
	{
		LOG->Flush();
		IDebugLine::Do( sMessageOut );
	}
};

class DebugLinePullBackCamera : public IDebugLine
{
	virtual CString GetDescription() { return PULL_BACK_CAMERA; }
	virtual CString GetValue() { return NULL; }
	virtual bool IsEnabled() { return g_fImageScaleDestination != 1; }
	virtual void Do( CString &sMessageOut )
	{
		if( g_fImageScaleDestination == 1 )
			g_fImageScaleDestination = 0.5f;
		else
			g_fImageScaleDestination = 1;
		IDebugLine::Do( sMessageOut );
	}
};

class DebugLineVolumeUp : public IDebugLine
{
	virtual CString GetDescription() { return VOLUME_UP; }
	virtual CString GetValue() { return ssprintf("%.0f%%",PREFSMAN->m_fSoundVolume.Get()*100); }
	virtual bool IsEnabled() { return true; }
	virtual void Do( CString &sMessageOut )
	{
		ChangeVolume( +0.1f );
		IDebugLine::Do( sMessageOut );
	}
};

class DebugLineVolumeDown : public IDebugLine
{
	virtual CString GetDescription() { return VOLUME_DOWN; }
	virtual CString GetValue() { return NULL; }
	virtual bool IsEnabled() { return true; }
	virtual void Do( CString &sMessageOut )
	{
		ChangeVolume( -0.1f );
		IDebugLine::Do( sMessageOut );
	}
};

class DebugLineUptime : public IDebugLine
{
	virtual CString GetDescription() { return UPTIME; }
	virtual CString GetValue() { return SecondsToMMSSMsMsMs(RageTimer::GetTimeSinceStart()); }
	virtual bool IsEnabled() { return false; }
	virtual void Do( CString &sMessageOut ) {}
};

/* #ifdef out the lines below if you don't want them to appear on certain
 * platforms.  This is easier than #ifdefing the whole DebugLine definitions
 * that can span pages.
 */

DECLARE_ONE( DebugLineAutoplay );
DECLARE_ONE( DebugLineAssistTick );
DECLARE_ONE( DebugLineAutosync );
DECLARE_ONE( DebugLineCoinMode );
DECLARE_ONE( DebugLineSlow );
DECLARE_ONE( DebugLineHalt );
DECLARE_ONE( DebugLineLightsDebug );
DECLARE_ONE( DebugLineMonkeyInput );
DECLARE_ONE( DebugLineStats );
DECLARE_ONE( DebugLineVsync );
DECLARE_ONE( DebugLineAllowMultitexture );
DECLARE_ONE( DebugLineScreenTestMode );
DECLARE_ONE( DebugLineClearMachineStats );
DECLARE_ONE( DebugLineFillMachineStats );
DECLARE_ONE( DebugLineSendNotesEnded );
DECLARE_ONE( DebugLineReloadCurrentScreen );
DECLARE_ONE( DebugLineReloadTheme );
DECLARE_ONE( DebugLineWriteProfiles );
DECLARE_ONE( DebugLineWritePreferences );
DECLARE_ONE( DebugLineMenuTimer );
DECLARE_ONE( DebugLineFlushLog );
DECLARE_ONE( DebugLinePullBackCamera );
DECLARE_ONE( DebugLineVolumeUp );
DECLARE_ONE( DebugLineVolumeDown );
DECLARE_ONE( DebugLineUptime );


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
