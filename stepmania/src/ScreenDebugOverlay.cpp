#include "global.h"
#include "ScreenDebugOverlay.h"
#include "ScreenDimensions.h"
#include "ScreenManager.h"
#include "PrefsManager.h"
#include "GamePreferences.h"
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
#include "Profile.h"
#include "SongManager.h"
#include "GameLoop.h"
#include "ScreenServiceAction.h"
#include "song.h"

static bool g_bIsDisplayed = false;
static bool g_bIsSlow = false;
static bool g_bIsHalt = false;
static RageTimer g_HaltTimer(RageZeroTimer);
static float g_fImageScaleCurrent = 1;
static float g_fImageScaleDestination = 1;

//
// self-registering debug lines
// We don't use SubscriptionManager, because we want to keep the line order.
//
static LocalizedString ON			( "ScreenDebugOverlay", "on" );
static LocalizedString OFF			( "ScreenDebugOverlay", "off" );

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
	virtual ~IDebugLine() { }
	enum Type { all_screens, gameplay_only };
	virtual Type GetType() const { return all_screens; }
	virtual RString GetDescription() = 0;
	virtual RString GetValue() { return IsEnabled() ? ON.GetValue():OFF.GetValue(); }
	virtual bool IsEnabled() = 0;
	virtual void Do( RString &sMessageOut )
	{
		RString s1 = GetDescription();
		RString s2 = GetValue();
		if( !s2.empty() )
			s1 += " - ";
		sMessageOut = s1 + s2;
	};

	DeviceInput m_Button;
};

static bool IsGameplay()
{
	return SCREENMAN && SCREENMAN->GetTopScreen() && SCREENMAN->GetTopScreen()->GetScreenType() == gameplay;
}


REGISTER_SCREEN_CLASS( ScreenDebugOverlay );

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
	DeviceInput holdForSlow;
	DeviceInput holdForFast;
	DeviceInput debugButton[MAX_DEBUG_LINES];
	DeviceInput gameplayButton[MAX_DEBUG_LINES];
	void Clear()
	{
		holdForDebug1.MakeInvalid();
		holdForDebug2.MakeInvalid();
		holdForSlow.MakeInvalid();
		holdForFast.MakeInvalid();
		for( int i=0; i<MAX_DEBUG_LINES; i++ )
		{
			debugButton[i].MakeInvalid();
			gameplayButton[i].MakeInvalid();
		}
	}
};
static MapDebugToDI g_Mappings;

static LocalizedString IN_GAMEPLAY( "ScreenDebugOverlay", "%s in gameplay" );
static LocalizedString OR( "ScreenDebugOverlay", "or" );
static RString GetDebugButtonName( const IDebugLine *pLine )
{
	RString s = INPUTMAN->GetDeviceSpecificInputString(pLine->m_Button);
	switch( pLine->GetType() )
	{
	case IDebugLine::all_screens:
		return s;
	case IDebugLine::gameplay_only:
		return ssprintf( IN_GAMEPLAY.GetValue(), s.c_str() );
	default:
		ASSERT(0);
	}
}

static LocalizedString DEBUG_MENU( "ScreenDebugOverlay", "Debug Menu" );
void ScreenDebugOverlay::Init()
{
	Screen::Init();
	
	// Init debug mappings
	// TODO: Arch-specific?
	{
		g_Mappings.Clear();

		g_Mappings.holdForDebug1 = DeviceInput(DEVICE_KEYBOARD, KEY_F3);
		g_Mappings.holdForDebug2.MakeInvalid();
		g_Mappings.holdForSlow = DeviceInput(DEVICE_KEYBOARD, KEY_ACCENT);
		g_Mappings.holdForFast = DeviceInput(DEVICE_KEYBOARD, KEY_TAB);


		int i=0;
		g_Mappings.gameplayButton[i++]	= DeviceInput(DEVICE_KEYBOARD, KEY_F8);
		g_Mappings.gameplayButton[i++]	= DeviceInput(DEVICE_KEYBOARD, KEY_F7);
		g_Mappings.gameplayButton[i++]	= DeviceInput(DEVICE_KEYBOARD, KEY_F6);
		i=0;
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
		g_Mappings.debugButton[i++] = DeviceInput(DEVICE_KEYBOARD, KEY_Cp);
		g_Mappings.debugButton[i++] = DeviceInput(DEVICE_KEYBOARD, KEY_UP);
		g_Mappings.debugButton[i++] = DeviceInput(DEVICE_KEYBOARD, KEY_DOWN);
		g_Mappings.debugButton[i++] = DeviceInput(DEVICE_KEYBOARD, KEY_BACK);
	}


	int iNextGameplayButton = 0;
	int iNextDebugButton = 0;
	FOREACH( IDebugLine*, *g_pvpSubscribers, p )
	{
		switch( (*p)->GetType() )
		{
		case IDebugLine::all_screens:
			(*p)->m_Button = g_Mappings.debugButton[iNextDebugButton++];
			break;
		case IDebugLine::gameplay_only:
			(*p)->m_Button = g_Mappings.gameplayButton[iNextGameplayButton++];
			break;
		}
	}

	m_Quad.StretchTo( RectF( 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT ) );
	m_Quad.SetDiffuse( RageColor(0, 0, 0, 0.5f) );
	this->AddChild( &m_Quad );
	
	m_textHeader.LoadFromFont( THEME->GetPathF("Common", "normal") );
	m_textHeader.SetHorizAlign( align_left );
	m_textHeader.SetX( SCREEN_LEFT+20 );
	m_textHeader.SetY( SCREEN_TOP+20 );
	m_textHeader.SetZoom( 1.0f );
	m_textHeader.SetText( DEBUG_MENU );
	this->AddChild( &m_textHeader );

	FOREACH_CONST( IDebugLine*, *g_pvpSubscribers, p )
	{
		{
			BitmapText *pT1 = new BitmapText;
			pT1->LoadFromFont( THEME->GetPathF("Common", "normal") );
			pT1->SetHorizAlign( align_right );
			pT1->SetText( "blah" );
			pT1->SetShadowLength( 2 );
			m_vptextButton.push_back( pT1 );
			this->AddChild( pT1 );
		}
		{
			BitmapText *pT2 = new BitmapText;
			pT2->LoadFromFont( THEME->GetPathF("Common", "normal") );
			pT2->SetHorizAlign( align_left );
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
	{
		float fRate = 1;
		if( INPUTFILTER->IsBeingPressed(g_Mappings.holdForFast) )
		{
			if( INPUTFILTER->IsBeingPressed(g_Mappings.holdForSlow) )
				fRate = 0; /* both; stop time */
			else
				fRate *= 4;
		}
		else if( INPUTFILTER->IsBeingPressed(g_Mappings.holdForSlow) )
		{
			fRate /= 4;
		}

		if( g_bIsHalt )
			fRate = 0;
		else if( g_bIsSlow )
			fRate /= 4;

		GameLoop::SetUpdateRate( fRate );
	}

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
		float fOffsetFromCenterIndex = i - (NUM_DEBUG_LINES-1)/2.0f; 
		float fY = SCREEN_CENTER_Y+10 + fOffsetFromCenterIndex * 16;

		BitmapText &txt1 = *m_vptextButton[i];
		txt1.SetX( SCREEN_CENTER_X-50 );
		txt1.SetY( fY );
		txt1.SetZoom( 0.7f );

		BitmapText &txt2 = *m_vptextFunction[i];
		txt2.SetX( SCREEN_CENTER_X-30 );
		txt2.SetY( fY );
		txt2.SetZoom( 0.7f );

		RString s1 = (*p)->GetDescription();
		RString s2 = (*p)->GetValue();

		bool bOn = (*p)->IsEnabled();

		txt1.SetDiffuse( bOn ? on:off );
		txt2.SetDiffuse( bOn ? on:off );

		RString sButton = GetDebugButtonName( *p );
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

		// Gameplay buttons are available only in gameplay.  Non-gameplay buttons are
		// only available when the screen is displayed.
		switch( (*p)->GetType() )
		{
		case IDebugLine::all_screens:
			if( !g_bIsDisplayed )
				continue;
			break;
		case IDebugLine::gameplay_only:
			if( !IsGameplay() )
				continue;
			break;
		default:
			ASSERT(0);
		}

		if( input.DeviceI == (*p)->m_Button )
		{
			if( input.type != IET_FIRST_PRESS )
				return true; /* eat the input but do nothing */

			BitmapText &txt1 = *m_vptextButton[i];
			txt1.FinishTweening();
			float fZoom = txt1.GetZoom();
			txt1.SetZoom( fZoom * 1.2f );
			txt1.BeginTweening( 0.2f, TWEEN_LINEAR );
			txt1.SetZoom( fZoom );

			RString sMessage;
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
static LocalizedString ASSIST			( "ScreenDebugOverlay", "Assist" );
static LocalizedString AUTOSYNC			( "ScreenDebugOverlay", "Autosync" );
static LocalizedString COIN_MODE		( "ScreenDebugOverlay", "CoinMode" );
static LocalizedString HALT			( "ScreenDebugOverlay", "Halt" );
static LocalizedString LIGHTS_DEBUG		( "ScreenDebugOverlay", "Lights Debug" );
static LocalizedString MONKEY_INPUT		( "ScreenDebugOverlay", "Monkey Input" );
static LocalizedString RENDERING_STATS		( "ScreenDebugOverlay", "Rendering Stats" );
static LocalizedString VSYNC			( "ScreenDebugOverlay", "Vsync" );
static LocalizedString MULTITEXTURE		( "ScreenDebugOverlay", "Multitexture" );
static LocalizedString SCREEN_TEST_MODE		( "ScreenDebugOverlay", "Screen Test Mode" );
static LocalizedString CLEAR_MACHINE_STATS	( "ScreenDebugOverlay", "Clear Machine Stats" );
static LocalizedString FILL_MACHINE_STATS	( "ScreenDebugOverlay", "Fill Machine Stats" );
static LocalizedString SEND_NOTES_ENDED		( "ScreenDebugOverlay", "Send Notes Ended" );
static LocalizedString RELOAD			( "ScreenDebugOverlay", "Reload" );
static LocalizedString RESTART			( "ScreenDebugOverlay", "Restart" );
static LocalizedString RELOAD_THEME_AND_TEXTURES( "ScreenDebugOverlay", "Reload Theme and Textures" );
static LocalizedString WRITE_PROFILES		( "ScreenDebugOverlay", "Write Profiles" );
static LocalizedString WRITE_PREFERENCES	( "ScreenDebugOverlay", "Write Preferences" );
static LocalizedString MENU_TIMER		( "ScreenDebugOverlay", "Menu Timer" );
static LocalizedString FLUSH_LOG		( "ScreenDebugOverlay", "Flush Log" );
static LocalizedString PULL_BACK_CAMERA		( "ScreenDebugOverlay", "Pull Back Camera" );
static LocalizedString VOLUME_UP		( "ScreenDebugOverlay", "Volume Up" );
static LocalizedString VOLUME_DOWN		( "ScreenDebugOverlay", "Volume Down" );
static LocalizedString UPTIME			( "ScreenDebugOverlay", "Uptime" );
static LocalizedString FORCE_CRASH		( "ScreenDebugOverlay", "Force Crash" );
static LocalizedString SLOW			( "ScreenDebugOverlay", "Slow" );
static LocalizedString CPU			( "ScreenDebugOverlay", "CPU" );
static LocalizedString SONG			( "ScreenDebugOverlay", "Song" );
static LocalizedString MACHINE			( "ScreenDebugOverlay", "Machine" );
static LocalizedString SYNC_TEMPO   ( "ScreenDebugOverlay", "Tempo" );


class DebugLineAutoplay : public IDebugLine
{
	virtual RString GetDescription() { return AUTO_PLAY.GetValue(); }
	virtual RString GetValue()
	{
		switch( GamePreferences::m_AutoPlay.Get() )
		{
		case PC_HUMAN:		return OFF.GetValue();	break;
		case PC_AUTOPLAY:	return ON.GetValue();	break;
		case PC_CPU:		return CPU.GetValue();	break;
		default:	ASSERT(0);	return RString();
		}
	}
	virtual Type GetType() const { return IDebugLine::gameplay_only; }
	virtual bool IsEnabled() { return GamePreferences::m_AutoPlay.Get() != PC_HUMAN; }
	virtual void Do( RString &sMessageOut )
	{
		ASSERT( GAMESTATE->m_MasterPlayerNumber != PLAYER_INVALID );
		PlayerController pc = GAMESTATE->m_pPlayerState[GAMESTATE->m_MasterPlayerNumber]->m_PlayerController;
		bool bHoldingShift = INPUTFILTER->IsBeingPressed( DeviceInput(DEVICE_KEYBOARD, KEY_LSHIFT) );
		if( bHoldingShift )
			pc = (pc==PC_CPU) ? PC_HUMAN : PC_CPU;
		else
			pc = (pc==PC_AUTOPLAY) ? PC_HUMAN : PC_AUTOPLAY;
		GamePreferences::m_AutoPlay.Set( pc );
		FOREACH_HumanPlayer(p)
			GAMESTATE->m_pPlayerState[p]->m_PlayerController = GamePreferences::m_AutoPlay;
		FOREACH_MultiPlayer(p)
			GAMESTATE->m_pMultiPlayerState[p]->m_PlayerController = GamePreferences::m_AutoPlay;
		IDebugLine::Do( sMessageOut );
	}
};

class DebugLineAssist : public IDebugLine
{
	virtual RString GetDescription() { return ASSIST.GetValue(); }
	virtual Type GetType() const { return gameplay_only; }
	virtual RString GetValue() { 
		SongOptions so;
		so.m_bAssistClap = GAMESTATE->m_SongOptions.GetSong().m_bAssistClap;
		so.m_bAssistMetronome = GAMESTATE->m_SongOptions.GetSong().m_bAssistMetronome;
		if( so.m_bAssistClap || so.m_bAssistMetronome )
			return so.GetLocalizedString();
		else
			return OFF.GetValue();
	}
	virtual bool IsEnabled() { return GAMESTATE->m_SongOptions.GetSong().m_bAssistClap || GAMESTATE->m_SongOptions.GetSong().m_bAssistMetronome; }
	virtual void Do( RString &sMessageOut )
	{
		ASSERT( GAMESTATE->m_MasterPlayerNumber != PLAYER_INVALID );
		bool bHoldingShift = INPUTFILTER->IsBeingPressed( DeviceInput(DEVICE_KEYBOARD, KEY_LSHIFT) );
		bool b;
		if( bHoldingShift )
			b = !GAMESTATE->m_SongOptions.GetSong().m_bAssistMetronome;
		else
			b = !GAMESTATE->m_SongOptions.GetSong().m_bAssistClap;
		if( bHoldingShift )
			SO_GROUP_ASSIGN( GAMESTATE->m_SongOptions, ModsLevel_Song, m_bAssistMetronome, b );
		else
			SO_GROUP_ASSIGN( GAMESTATE->m_SongOptions, ModsLevel_Song, m_bAssistClap, b );

		IDebugLine::Do( sMessageOut );
	}
};

class DebugLineAutosync : public IDebugLine
{
	virtual RString GetDescription() { return AUTOSYNC.GetValue(); }
	virtual RString GetValue()
	{ 
		switch( GAMESTATE->m_SongOptions.GetSong().m_AutosyncType )
		{
		case SongOptions::AUTOSYNC_OFF: 	return OFF.GetValue();  		break;
		case SongOptions::AUTOSYNC_SONG:	return SONG.GetValue(); 		break;
		case SongOptions::AUTOSYNC_MACHINE:	return MACHINE.GetValue(); 		break;
		case SongOptions::AUTOSYNC_TEMPO:	return SYNC_TEMPO.GetValue();		break;
		default:	ASSERT(0);
		}
	}
	virtual Type GetType() const { return IDebugLine::gameplay_only; }
	virtual bool IsEnabled() { return GAMESTATE->m_SongOptions.GetSong().m_AutosyncType!=SongOptions::AUTOSYNC_OFF; }
	virtual void Do( RString &sMessageOut )
	{
		int as = GAMESTATE->m_SongOptions.GetSong().m_AutosyncType + 1;
		bool bAllowSongAutosync = !GAMESTATE->IsCourseMode();
		if( !bAllowSongAutosync  && 
		  ( as == SongOptions::AUTOSYNC_SONG || as == SongOptions::AUTOSYNC_TEMPO ) )
			as = SongOptions::AUTOSYNC_MACHINE;
		wrap( as, SongOptions::NUM_AUTOSYNC_TYPES );
		SO_GROUP_ASSIGN( GAMESTATE->m_SongOptions, ModsLevel_Song, m_AutosyncType, SongOptions::AutosyncType(as) );
		MESSAGEMAN->Broadcast( Message_AutosyncChanged );
		IDebugLine::Do( sMessageOut );
	}
};

class DebugLineCoinMode : public IDebugLine
{
	virtual RString GetDescription() { return COIN_MODE.GetValue(); }
	virtual RString GetValue() { return CoinModeToString(GamePreferences::m_CoinMode); }
	virtual bool IsEnabled() { return true; }
	virtual void Do( RString &sMessageOut )
	{
		int cm = GamePreferences::m_CoinMode+1;
		wrap( cm, NUM_CoinMode );
		GamePreferences::m_CoinMode.Set( CoinMode(cm) );
		SCREENMAN->RefreshCreditsMessages();
		IDebugLine::Do( sMessageOut );
	}
};

class DebugLineSlow : public IDebugLine
{
	virtual RString GetDescription() { return SLOW.GetValue(); }
	virtual bool IsEnabled() { return g_bIsSlow; }
	virtual void Do( RString &sMessageOut )
	{
		g_bIsSlow = !g_bIsSlow;
		SetSpeed();
		IDebugLine::Do( sMessageOut );
	}
};

class DebugLineHalt : public IDebugLine
{
	virtual RString GetDescription() { return HALT.GetValue(); }
	virtual bool IsEnabled() { return g_bIsHalt; }
	virtual void Do( RString &sMessageOut )
	{
		g_bIsHalt = !g_bIsHalt;
		g_HaltTimer.Touch();
		SetSpeed();
		IDebugLine::Do( sMessageOut );
	}
};

class DebugLineLightsDebug : public IDebugLine
{
	virtual RString GetDescription() { return LIGHTS_DEBUG.GetValue(); }
	virtual bool IsEnabled() { return PREFSMAN->m_bDebugLights.Get(); }
	virtual void Do( RString &sMessageOut )
	{
		PREFSMAN->m_bDebugLights.Set( !PREFSMAN->m_bDebugLights );
		IDebugLine::Do( sMessageOut );
	}
};

class DebugLineMonkeyInput : public IDebugLine
{
	virtual RString GetDescription() { return MONKEY_INPUT.GetValue(); }
	virtual bool IsEnabled() { return PREFSMAN->m_bMonkeyInput.Get(); }
	virtual void Do( RString &sMessageOut )
	{
		PREFSMAN->m_bMonkeyInput.Set( !PREFSMAN->m_bMonkeyInput );
		IDebugLine::Do( sMessageOut );
	}
};

class DebugLineStats : public IDebugLine
{
	virtual RString GetDescription() { return RENDERING_STATS.GetValue(); }
	virtual bool IsEnabled() { return PREFSMAN->m_bShowStats.Get(); }
	virtual void Do( RString &sMessageOut )
	{
		PREFSMAN->m_bShowStats.Set( !PREFSMAN->m_bShowStats );
		IDebugLine::Do( sMessageOut );
	}
};

class DebugLineVsync : public IDebugLine
{
	virtual RString GetDescription() { return VSYNC.GetValue(); }
	virtual bool IsEnabled() { return PREFSMAN->m_bVsync.Get(); }
	virtual void Do( RString &sMessageOut )
	{
		PREFSMAN->m_bVsync.Set( !PREFSMAN->m_bVsync );
		StepMania::ApplyGraphicOptions();
		IDebugLine::Do( sMessageOut );
	}
};

class DebugLineAllowMultitexture : public IDebugLine
{
	virtual RString GetDescription() { return MULTITEXTURE.GetValue(); }
	virtual bool IsEnabled() { return PREFSMAN->m_bAllowMultitexture.Get(); }
	virtual void Do( RString &sMessageOut )
	{
		PREFSMAN->m_bAllowMultitexture.Set( !PREFSMAN->m_bAllowMultitexture );
		IDebugLine::Do( sMessageOut );
	}
};

class DebugLineScreenTestMode : public IDebugLine
{
	virtual RString GetDescription() { return SCREEN_TEST_MODE.GetValue(); }
	virtual bool IsEnabled() { return PREFSMAN->m_bScreenTestMode.Get(); }
	virtual void Do( RString &sMessageOut )
	{
		PREFSMAN->m_bScreenTestMode.Set( !PREFSMAN->m_bScreenTestMode );
		IDebugLine::Do( sMessageOut );
	}
};

class DebugLineClearMachineStats : public IDebugLine
{
	virtual RString GetDescription() { return CLEAR_MACHINE_STATS.GetValue(); }
	virtual RString GetValue() { return RString(); }
	virtual bool IsEnabled() { return true; }
	virtual void Do( RString &sMessageOut )
	{
		GameCommand gc;
		ClearMachineStats();
		IDebugLine::Do( sMessageOut );
	}
};

static HighScore MakeRandomHighScore( float fPercentDP )
{
	HighScore hs;
	hs.SetName( "FAKE" );
	hs.SetGrade( (Grade)SCALE( RandomInt(5), 0, 4, Grade_Tier01, Grade_Tier05 ) );
	hs.SetScore( RandomInt(100*1000) );
	hs.SetPercentDP( fPercentDP );
	hs.SetSurviveSeconds( randomf(30.0f, 100.0f) );
	PlayerOptions po;
	po.ChooseRandomModifiers();
	hs.SetModifiers( po.GetString() );
	hs.SetDateTime( DateTime::GetNowDateTime() );
	hs.SetPlayerGuid( Profile::MakeGuid() );
	hs.SetMachineGuid( Profile::MakeGuid() );
	hs.SetProductID( RandomInt(10) );
	FOREACH_TapNoteScore( tns )
		hs.SetTapNoteScore( tns, RandomInt(100) );
	FOREACH_HoldNoteScore( hns )
		hs.SetHoldNoteScore( hns, RandomInt(100) );
	RadarValues rv;
	FOREACH_RadarCategory( rc )
		rv.m_Values.f[rc] = randomf( 0, 1 );
	hs.SetRadarValues( rv );

	return hs;
}

static void FillProfileStats( Profile *pProfile )
{
	pProfile->InitSongScores(); 
	pProfile->InitCourseScores();

	static int s_iCount = 0;
	// Choose a percent for all scores.  This is useful for testing unlocks
	// where some elements are unlocked at a certain percent complete
	float fPercentDP = s_iCount ? randomf( 0.6f, 1.0f ) : 1.0f;
	s_iCount = (s_iCount+1)%2;


	int iCount = pProfile->IsMachine()? 
		PREFSMAN->m_iMaxHighScoresPerListForMachine.Get():
		PREFSMAN->m_iMaxHighScoresPerListForPlayer.Get();

	vector<Song*> vpAllSongs = SONGMAN->GetAllSongs();
	FOREACH( Song*, vpAllSongs, pSong )
	{
		vector<Steps*> vpAllSteps = (*pSong)->GetAllSteps();
		FOREACH( Steps*, vpAllSteps, pSteps )
		{
			pProfile->IncrementStepsPlayCount( *pSong, *pSteps );
			for( int i=0; i<iCount; i++ )
			{
				int iIndex = 0;
				pProfile->AddStepsHighScore( *pSong, *pSteps, MakeRandomHighScore(fPercentDP), iIndex );
			}
		}
	}
	
	vector<Course*> vpAllCourses;
	SONGMAN->GetAllCourses( vpAllCourses, true );
	FOREACH( Course*, vpAllCourses, pCourse )
	{
		vector<Trail*> vpAllTrails;
		(*pCourse)->GetAllTrails( vpAllTrails );
		FOREACH( Trail*, vpAllTrails, pTrail )
		{
			pProfile->IncrementCoursePlayCount( *pCourse, *pTrail );
			for( int i=0; i<iCount; i++ )
			{
				int iIndex = 0;
				pProfile->AddCourseHighScore( *pCourse, *pTrail, MakeRandomHighScore(fPercentDP), iIndex );
			}
		}
	}
}

class DebugLineFillMachineStats : public IDebugLine
{
	virtual RString GetDescription() { return FILL_MACHINE_STATS.GetValue(); }
	virtual RString GetValue() { return RString(); }
	virtual bool IsEnabled() { return true; }
	virtual void Do( RString &sMessageOut )
	{
		Profile* pProfile = PROFILEMAN->GetMachineProfile();
		FillProfileStats( pProfile );
		PROFILEMAN->SaveMachineProfile();
		IDebugLine::Do( sMessageOut );
	}
};

class DebugLineSendNotesEnded : public IDebugLine
{
	virtual RString GetDescription() { return SEND_NOTES_ENDED.GetValue(); }
	virtual RString GetValue() { return RString(); }
	virtual bool IsEnabled() { return true; }
	virtual void Do( RString &sMessageOut )
	{
		SCREENMAN->PostMessageToTopScreen( SM_NotesEnded, 0 );
		IDebugLine::Do( sMessageOut );
	}
};

class DebugLineReloadCurrentScreen : public IDebugLine
{
	virtual RString GetDescription() { return RELOAD.GetValue(); }
	virtual RString GetValue() { return SCREENMAN && SCREENMAN->GetTopScreen()? SCREENMAN->GetTopScreen()->GetName() : RString(); }
	virtual bool IsEnabled() { return true; }
	virtual void Do( RString &sMessageOut )
	{
		RString sScreenName = SCREENMAN->GetScreen(0)->GetName();
		SCREENMAN->PopAllScreens();

		SOUND->StopMusic();
		StepMania::ResetGame();

		SCREENMAN->SetNewScreen( sScreenName );
		IDebugLine::Do( sMessageOut );
		sMessageOut = "";
	}
};

class DebugLineRestartCurrentScreen : public IDebugLine
{
	virtual RString GetDescription() { return RESTART.GetValue(); }
	virtual RString GetValue() { return SCREENMAN && SCREENMAN->GetTopScreen()? SCREENMAN->GetTopScreen()->GetName() : RString(); }
	virtual bool IsEnabled() { return true; }
	virtual void Do( RString &sMessageOut )
	{
		SCREENMAN->GetTopScreen()->BeginScreen();
		IDebugLine::Do( sMessageOut );
		sMessageOut = "";
	}
};

class DebugLineReloadTheme : public IDebugLine
{
	virtual RString GetDescription() { return RELOAD_THEME_AND_TEXTURES.GetValue(); }
	virtual RString GetValue() { return RString(); }
	virtual bool IsEnabled() { return true; }
	virtual void Do( RString &sMessageOut )
	{
		THEME->ReloadMetrics();
		TEXTUREMAN->ReloadAll();
		NOTESKIN->RefreshNoteSkinData( GAMESTATE->m_pCurGame );
		CodeDetector::RefreshCacheItems();
		// HACK: Don't update text below.  Return immediately because this screen
		// was just destroyed as part of the theme reload.
		IDebugLine::Do( sMessageOut );
	}
};

class DebugLineWriteProfiles : public IDebugLine
{
	virtual RString GetDescription() { return WRITE_PROFILES.GetValue(); }
	virtual RString GetValue() { return RString(); }
	virtual bool IsEnabled() { return true; }
	virtual void Do( RString &sMessageOut )
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
	virtual RString GetDescription() { return WRITE_PREFERENCES.GetValue(); }
	virtual RString GetValue() { return RString(); }
	virtual bool IsEnabled() { return true; }
	virtual void Do( RString &sMessageOut )
	{
		PREFSMAN->SavePrefsToDisk();
		IDebugLine::Do( sMessageOut );
	}
};

class DebugLineMenuTimer : public IDebugLine
{
	virtual RString GetDescription() { return MENU_TIMER.GetValue(); }
	virtual RString GetValue() { return RString(); }
	virtual bool IsEnabled() { return PREFSMAN->m_bMenuTimer.Get(); }
	virtual void Do( RString &sMessageOut )
	{
		PREFSMAN->m_bMenuTimer.Set( !PREFSMAN->m_bMenuTimer );
		IDebugLine::Do( sMessageOut );
	}
};

class DebugLineFlushLog : public IDebugLine
{
	virtual RString GetDescription() { return FLUSH_LOG.GetValue(); }
	virtual RString GetValue() { return RString(); }
	virtual bool IsEnabled() { return true; }
	virtual void Do( RString &sMessageOut )
	{
		LOG->Flush();
		IDebugLine::Do( sMessageOut );
	}
};

class DebugLinePullBackCamera : public IDebugLine
{
	virtual RString GetDescription() { return PULL_BACK_CAMERA.GetValue(); }
	virtual RString GetValue() { return RString(); }
	virtual bool IsEnabled() { return g_fImageScaleDestination != 1; }
	virtual void Do( RString &sMessageOut )
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
	virtual RString GetDescription() { return VOLUME_UP.GetValue(); }
	virtual RString GetValue() { return ssprintf("%.0f%%",PREFSMAN->m_fSoundVolume.Get()*100); }
	virtual bool IsEnabled() { return true; }
	virtual void Do( RString &sMessageOut )
	{
		ChangeVolume( +0.1f );
		IDebugLine::Do( sMessageOut );
	}
};

class DebugLineVolumeDown : public IDebugLine
{
	virtual RString GetDescription() { return VOLUME_DOWN.GetValue(); }
	virtual RString GetValue() { return RString(); }
	virtual bool IsEnabled() { return true; }
	virtual void Do( RString &sMessageOut )
	{
		ChangeVolume( -0.1f );
		IDebugLine::Do( sMessageOut );
		sMessageOut += " - " + ssprintf("%.0f%%",PREFSMAN->m_fSoundVolume.Get()*100);
	}
};

class DebugLineForceCrash : public IDebugLine
{
	virtual RString GetDescription() { return FORCE_CRASH.GetValue(); }
	virtual RString GetValue() { return RString(); }
	virtual bool IsEnabled() { return false; }
	virtual void Do( RString &sMessageOut ) { FAIL_M("DebugLineCrash"); }
};

class DebugLineUptime : public IDebugLine
{
	virtual RString GetDescription() { return UPTIME.GetValue(); }
	virtual RString GetValue() { return SecondsToMMSSMsMsMs(RageTimer::GetTimeSinceStart()); }
	virtual bool IsEnabled() { return false; }
	virtual void Do( RString &sMessageOut ) {}
};

/* #ifdef out the lines below if you don't want them to appear on certain
 * platforms.  This is easier than #ifdefing the whole DebugLine definitions
 * that can span pages.
 */

DECLARE_ONE( DebugLineAutoplay );
DECLARE_ONE( DebugLineAssist );
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
DECLARE_ONE( DebugLineRestartCurrentScreen );
DECLARE_ONE( DebugLineReloadTheme );
DECLARE_ONE( DebugLineWriteProfiles );
DECLARE_ONE( DebugLineWritePreferences );
DECLARE_ONE( DebugLineMenuTimer );
DECLARE_ONE( DebugLineFlushLog );
DECLARE_ONE( DebugLinePullBackCamera );
DECLARE_ONE( DebugLineVolumeUp );
DECLARE_ONE( DebugLineVolumeDown );
DECLARE_ONE( DebugLineForceCrash );
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
