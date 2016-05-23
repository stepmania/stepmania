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
#include "InputMapper.h"
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
#include "Song.h"
#include "ScreenSyncOverlay.h"
#include "ThemeMetric.h"
#include "XmlToLua.h"

static bool g_bIsDisplayed = false;
static bool g_bIsSlow = false;
static bool g_bIsHalt = false;
static RageTimer g_HaltTimer(RageZeroTimer);
static float g_fImageScaleCurrent = 1;
static float g_fImageScaleDestination = 1;

// DebugLine theming
static const ThemeMetric<RageColor>	BACKGROUND_COLOR	("ScreenDebugOverlay", "BackgroundColor");
static const ThemeMetric<RageColor>	LINE_ON_COLOR	("ScreenDebugOverlay", "LineOnColor");
static const ThemeMetric<RageColor>	LINE_OFF_COLOR	("ScreenDebugOverlay", "LineOffColor");
static const ThemeMetric<float>		LINE_START_Y	("ScreenDebugOverlay", "LineStartY");
static const ThemeMetric<float>		LINE_SPACING	("ScreenDebugOverlay", "LineSpacing");
static const ThemeMetric<float>		LINE_BUTTON_X	("ScreenDebugOverlay", "LineButtonX");
static const ThemeMetric<float>		LINE_FUNCTION_X	("ScreenDebugOverlay", "LineFunctionX");
static const ThemeMetric<float>		PAGE_START_X	("ScreenDebugOverlay", "PageStartX");
static const ThemeMetric<float>		PAGE_SPACING_X	("ScreenDebugOverlay", "PageSpacingX");

// self-registering debug lines
// We don't use SubscriptionManager, because we want to keep the line order.
static LocalizedString ON			( "ScreenDebugOverlay", "on" );
static LocalizedString OFF			( "ScreenDebugOverlay", "off" );
static LocalizedString MUTE_ACTIONS_ON ("ScreenDebugOverlay", "Mute actions on");
static LocalizedString MUTE_ACTIONS_OFF ("ScreenDebugOverlay", "Mute actions off");

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
	virtual RString GetDisplayTitle() = 0;
	virtual RString GetDisplayValue() { return IsEnabled() ? ON.GetValue():OFF.GetValue(); }
	virtual RString GetPageName() const { return "Main"; }
	virtual bool ForceOffAfterUse() const { return false; }
	virtual bool IsEnabled() = 0;
	virtual void DoAndLog( RString &sMessageOut )
	{
		RString s1 = GetDisplayTitle();
		RString s2 = GetDisplayValue();
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

	FOREACH( BitmapText*, m_vptextPages, p )
		SAFE_DELETE( *p );
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
	DeviceInput toggleMute;
	DeviceInput debugButton[MAX_DEBUG_LINES];
	DeviceInput gameplayButton[MAX_DEBUG_LINES];
	map<DeviceInput, int> pageButton;
	void Clear()
	{
		holdForDebug1.MakeInvalid();
		holdForDebug2.MakeInvalid();
		holdForSlow.MakeInvalid();
		holdForFast.MakeInvalid();
		toggleMute.MakeInvalid();
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
	IDebugLine::Type type = pLine->GetType();
	switch( type )
	{
	case IDebugLine::all_screens:
		return s;
	case IDebugLine::gameplay_only:
		return ssprintf( IN_GAMEPLAY.GetValue(), s.c_str() );
	default:
		FAIL_M(ssprintf("Invalid debug line type: %i", type));
	}
}

template<typename U, typename V>
static bool GetKeyFromMap( const map<U, V> &m, const V &val, U &key )
{
	for( typename map<U,V>::const_iterator iter = m.begin(); iter != m.end(); ++iter )
	{
		if( iter->second == val )
		{
			key = iter->first;
			return true;
		}
	}
	return false;
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
		g_Mappings.toggleMute = DeviceInput(DEVICE_KEYBOARD, KEY_PAUSE);

		/* TODO: Find a better way of indicating which option is which here.
		 * Maybe we should take a page from ScreenEdit's menus and make
		 * RowDefs()? */

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
		g_Mappings.debugButton[i++] = DeviceInput(DEVICE_KEYBOARD, KEY_Ca);
		g_Mappings.debugButton[i++] = DeviceInput(DEVICE_KEYBOARD, KEY_Cs);
		g_Mappings.debugButton[i++] = DeviceInput(DEVICE_KEYBOARD, KEY_Cd);
		g_Mappings.pageButton[DeviceInput(DEVICE_KEYBOARD, KEY_F5)] = 0;
		g_Mappings.pageButton[DeviceInput(DEVICE_KEYBOARD, KEY_F6)] = 1;
		g_Mappings.pageButton[DeviceInput(DEVICE_KEYBOARD, KEY_F7)] = 2;
		g_Mappings.pageButton[DeviceInput(DEVICE_KEYBOARD, KEY_F8)] = 3;
	}

	map<RString,int> iNextDebugButton;
	int iNextGameplayButton = 0;
	FOREACH( IDebugLine*, *g_pvpSubscribers, p )
	{
		RString sPageName = (*p)->GetPageName();

		DeviceInput di;
		switch( (*p)->GetType() )
		{
		case IDebugLine::all_screens:
			di = g_Mappings.debugButton[iNextDebugButton[sPageName]++];
			break;
		case IDebugLine::gameplay_only:
			di = g_Mappings.gameplayButton[iNextGameplayButton++];
			break;
		}
		(*p)->m_Button = di;

		if( find(m_asPages.begin(), m_asPages.end(), sPageName) == m_asPages.end() )
			m_asPages.push_back( sPageName );
	}

	m_iCurrentPage = 0;
	m_bForcedHidden = false;

	m_Quad.StretchTo( RectF( 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT ) );
	m_Quad.SetDiffuse( BACKGROUND_COLOR );
	this->AddChild( &m_Quad );

	// if you're going to add user commands, make sure to have the overrides
	// set after parsing the metrics. -aj
	m_textHeader.SetName( "HeaderText" );
	m_textHeader.LoadFromFont( THEME->GetPathF("ScreenDebugOverlay", "header") );
	LOAD_ALL_COMMANDS_AND_SET_XY_AND_ON_COMMAND( m_textHeader );
	m_textHeader.SetText( DEBUG_MENU );
	this->AddChild( &m_textHeader );

	FOREACH_CONST( RString, m_asPages, s )
	{
		int iPage = s - m_asPages.begin();

		DeviceInput di;
		bool b = GetKeyFromMap( g_Mappings.pageButton, iPage, di );
		ASSERT( b );

		RString sButton = INPUTMAN->GetDeviceSpecificInputString( di );

		BitmapText *p = new BitmapText;
		p->SetName( "PageText" );
		p->LoadFromFont( THEME->GetPathF("ScreenDebugOverlay", "page") );
		LOAD_ALL_COMMANDS_AND_ON_COMMAND( p );
		// todo: Y value is still hardcoded. -aj
		p->SetXY( PAGE_START_X+iPage*PAGE_SPACING_X, SCREEN_TOP+20 );
		p->SetText( *s + " (" + sButton + ")" );
		m_vptextPages.push_back( p );
		this->AddChild( p );
	}

	FOREACH_CONST( IDebugLine*, *g_pvpSubscribers, p )
	{
		{
			BitmapText *bt = new BitmapText;
			bt->SetName( "ButtonText" );
			bt->LoadFromFont( THEME->GetPathF("ScreenDebugOverlay", "line") );
			bt->SetHorizAlign( align_right );
			bt->SetText( "blah" );
			LOAD_ALL_COMMANDS_AND_ON_COMMAND( *bt );
			m_vptextButton.push_back( bt );
			this->AddChild( bt );
		}
		{
			BitmapText *bt = new BitmapText;
			bt->SetName( "FunctionText" );
			bt->LoadFromFont( THEME->GetPathF("ScreenDebugOverlay", "line") );
			bt->SetHorizAlign( align_left );
			bt->SetText( "blah" );
			LOAD_ALL_COMMANDS_AND_ON_COMMAND( *bt );
			m_vptextFunction.push_back( bt );
			this->AddChild( bt );
		}
	}

	this->SetVisible( false );
}

void ScreenDebugOverlay::Update( float fDeltaTime )
{
	{
		float fRate = 1;
		if( INPUTFILTER->IsBeingPressed(g_Mappings.holdForFast) )
		{
			if( INPUTFILTER->IsBeingPressed(g_Mappings.holdForSlow) )
				fRate = 0; // both; stop time
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

	this->SetVisible( g_bIsDisplayed && !m_bForcedHidden );
	if( !g_bIsDisplayed )
		return;

	UpdateText();
}

void ScreenDebugOverlay::UpdateText()
{
	FOREACH_CONST( RString, m_asPages, s )
	{
		int iPage = s - m_asPages.begin();
		m_vptextPages[iPage]->PlayCommand( (iPage == m_iCurrentPage) ? "GainFocus" :  "LoseFocus" );
	}

	// todo: allow changing of various spacing/location things -aj
	int iOffset = 0;
	FOREACH_CONST( IDebugLine*, *g_pvpSubscribers, p )
	{
		RString sPageName = (*p)->GetPageName();

		int i = p-g_pvpSubscribers->begin();

		float fY = LINE_START_Y + iOffset * LINE_SPACING;

		BitmapText &txt1 = *m_vptextButton[i];
		BitmapText &txt2 = *m_vptextFunction[i];
		if( sPageName != GetCurrentPageName() )
		{
			txt1.SetVisible( false );
			txt2.SetVisible( false );
			continue;
		}
		txt1.SetVisible( true );
		txt2.SetVisible( true );
		++iOffset;

		txt1.SetX( LINE_BUTTON_X );
		txt1.SetY( fY );

		txt2.SetX( LINE_FUNCTION_X );
		txt2.SetY( fY );

		RString s1 = (*p)->GetDisplayTitle();
		RString s2 = (*p)->GetDisplayValue();

		bool bOn = (*p)->IsEnabled();

		txt1.SetDiffuse( bOn ? LINE_ON_COLOR:LINE_OFF_COLOR );
		txt2.SetDiffuse( bOn ? LINE_ON_COLOR:LINE_OFF_COLOR );

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

template<typename U, typename V>
static bool GetValueFromMap( const map<U, V> &m, const U &key, V &val )
{
	typename map<U, V>::const_iterator it = m.find(key);
	if( it == m.end() )
		return false;
	val = it->second;
	return true;
}

bool ScreenDebugOverlay::Input( const InputEventPlus &input )
{
	if( input.DeviceI == g_Mappings.holdForDebug1 || 
		input.DeviceI == g_Mappings.holdForDebug2 )
	{
		bool bHoldingNeither =
			(!g_Mappings.holdForDebug1.IsValid() || !INPUTFILTER->IsBeingPressed(g_Mappings.holdForDebug1)) &&
			(!g_Mappings.holdForDebug2.IsValid() || !INPUTFILTER->IsBeingPressed(g_Mappings.holdForDebug2));
		bool bHoldingBoth =
			(!g_Mappings.holdForDebug1.IsValid() || INPUTFILTER->IsBeingPressed(g_Mappings.holdForDebug1)) &&
			(!g_Mappings.holdForDebug2.IsValid() || INPUTFILTER->IsBeingPressed(g_Mappings.holdForDebug2));
		if( bHoldingNeither )
			m_bForcedHidden = false;

		if( bHoldingBoth )
			g_bIsDisplayed = true;
		else
			g_bIsDisplayed = false;
	}
	if(input.DeviceI == g_Mappings.toggleMute)
	{
		PREFSMAN->m_MuteActions.Set(!PREFSMAN->m_MuteActions);
		SCREENMAN->SystemMessage(PREFSMAN->m_MuteActions ? MUTE_ACTIONS_ON.GetValue() : MUTE_ACTIONS_OFF.GetValue());
	}

	int iPage = 0;
	if( g_bIsDisplayed && GetValueFromMap(g_Mappings.pageButton, input.DeviceI, iPage) )
	{
		if( input.type != IET_FIRST_PRESS )
			return true; // eat the input but do nothing
		m_iCurrentPage = iPage;
		CLAMP( m_iCurrentPage, 0, (int) m_asPages.size()-1 );
		return true;
	}

	FOREACH_CONST( IDebugLine*, *g_pvpSubscribers, p )
	{
		RString sPageName = (*p)->GetPageName();

		int i = p-g_pvpSubscribers->begin();

		// Gameplay buttons are available only in gameplay. Non-gameplay buttons
		// are only available when the screen is displayed.
		IDebugLine::Type type = (*p)->GetType();
		switch( type )
		{
		case IDebugLine::all_screens:
			if( !g_bIsDisplayed )
				continue;
			if( sPageName != GetCurrentPageName() )
				continue;
			break;
		case IDebugLine::gameplay_only:
			if( !IsGameplay() )
				continue;
			break;
		default:
			FAIL_M(ssprintf("Invalid debug line type: %i", type));
		}

		if( input.DeviceI == (*p)->m_Button )
		{
			if( input.type != IET_FIRST_PRESS )
				return true; // eat the input but do nothing

			// do the action
			RString sMessage;
			(*p)->DoAndLog( sMessage );
			if( !sMessage.empty() )
				LOG->Trace("DEBUG: %s", sMessage.c_str() );
			if( (*p)->ForceOffAfterUse() )
				m_bForcedHidden = true;

			// update text to show the effect of what changed above
			UpdateText();

			// show what changed
			BitmapText &bt = *m_vptextButton[i];
			bt.FinishTweening();
			bt.PlayCommand("Toggled");

			return true;
		}
	}

	return Screen::Input(input);
}


// DebugLine helpers
static void SetSpeed()
{
	// PauseMusic( g_bIsHalt );
}

void ChangeVolume( float fDelta )
{
	Preference<float> *pRet = Preference<float>::GetPreferenceByName("SoundVolume");
	float fVol = pRet->Get();
	fVol += fDelta;
	CLAMP( fVol, 0.0f, 1.0f );
	pRet->Set( fVol );
	SOUNDMAN->SetMixVolume();
}

void ChangeVisualDelay( float fDelta )
{
	Preference<float> *pRet = Preference<float>::GetPreferenceByName("VisualDelaySeconds");
	float fSecs = pRet->Get();
	fSecs += fDelta;
	CLAMP( fSecs, -1.0f, 1.0f );
	pRet->Set( fSecs );
}

// DebugLines
static LocalizedString AUTO_PLAY		( "ScreenDebugOverlay", "AutoPlay" );
static LocalizedString ASSIST			( "ScreenDebugOverlay", "Assist" );
static LocalizedString AUTOSYNC		( "ScreenDebugOverlay", "Autosync" );
static LocalizedString COIN_MODE		( "ScreenDebugOverlay", "CoinMode" );
static LocalizedString HALT			( "ScreenDebugOverlay", "Halt" );
static LocalizedString LIGHTS_DEBUG	( "ScreenDebugOverlay", "Lights Debug" );
static LocalizedString MONKEY_INPUT	( "ScreenDebugOverlay", "Monkey Input" );
static LocalizedString RENDERING_STATS	( "ScreenDebugOverlay", "Rendering Stats" );
static LocalizedString VSYNC			( "ScreenDebugOverlay", "Vsync" );
static LocalizedString MULTITEXTURE	( "ScreenDebugOverlay", "Multitexture" );
static LocalizedString SCREEN_TEST_MODE	( "ScreenDebugOverlay", "Screen Test Mode" );
static LocalizedString SCREEN_SHOW_MASKS	( "ScreenDebugOverlay", "Show Masks" );
static LocalizedString PROFILE			( "ScreenDebugOverlay", "Profile" );
static LocalizedString CLEAR_PROFILE_STATS	( "ScreenDebugOverlay", "Clear Profile Stats" );
static LocalizedString FILL_PROFILE_STATS	( "ScreenDebugOverlay", "Fill Profile Stats" );
static LocalizedString SEND_NOTES_ENDED	( "ScreenDebugOverlay", "Send Notes Ended" );
static LocalizedString RESET_KEY_MAP ("ScreenDebugOverlay", "Reset key mapping to default");
static LocalizedString MUTE_ACTIONS ("ScreenDebugOverlay", "Mute actions");
static LocalizedString RELOAD			( "ScreenDebugOverlay", "Reload" );
static LocalizedString RESTART			( "ScreenDebugOverlay", "Restart" );
static LocalizedString SCREEN_ON		( "ScreenDebugOverlay", "Send On To Screen" );
static LocalizedString SCREEN_OFF		( "ScreenDebugOverlay", "Send Off To Screen" );
static LocalizedString RELOAD_OVERLAY_SCREENS( "ScreenDebugOverlay", "Reload Overlay Screens" );
static LocalizedString TOGGLE_ERRORS( "ScreenDebugOverlay", "Toggle Errors" );
static LocalizedString SHOW_RECENT_ERRORS("ScreenDebugOverlay", "Show Recent Errors");
static LocalizedString CLEAR_ERRORS( "ScreenDebugOverlay", "Clear Errors" );
static LocalizedString CONVERT_XML( "ScreenDebugOverlay", "Convert XML" );
static LocalizedString RELOAD_THEME_AND_TEXTURES( "ScreenDebugOverlay", "Reload Theme and Textures" );
static LocalizedString WRITE_PROFILES	( "ScreenDebugOverlay", "Write Profiles" );
static LocalizedString WRITE_PREFERENCES	( "ScreenDebugOverlay", "Write Preferences" );
static LocalizedString MENU_TIMER		( "ScreenDebugOverlay", "Menu Timer" );
static LocalizedString FLUSH_LOG		( "ScreenDebugOverlay", "Flush Log" );
static LocalizedString PULL_BACK_CAMERA	( "ScreenDebugOverlay", "Pull Back Camera" );
static LocalizedString VISUAL_DELAY_UP		( "ScreenDebugOverlay", "Visual Delay Up" );
static LocalizedString VISUAL_DELAY_DOWN	( "ScreenDebugOverlay", "Visual Delay Down" );
static LocalizedString VOLUME_UP		( "ScreenDebugOverlay", "Volume Up" );
static LocalizedString VOLUME_DOWN		( "ScreenDebugOverlay", "Volume Down" );
static LocalizedString UPTIME			( "ScreenDebugOverlay", "Uptime" );
static LocalizedString FORCE_CRASH		( "ScreenDebugOverlay", "Force Crash" );
static LocalizedString SLOW			( "ScreenDebugOverlay", "Slow" );
static LocalizedString CPU				( "ScreenDebugOverlay", "CPU" );
static LocalizedString SONG			( "ScreenDebugOverlay", "Song" );
static LocalizedString MACHINE			( "ScreenDebugOverlay", "Machine" );
static LocalizedString SYNC_TEMPO		( "ScreenDebugOverlay", "Tempo" );

class DebugLineAutoplay : public IDebugLine
{
	virtual RString GetDisplayTitle() { return AUTO_PLAY.GetValue() + " (+Shift = AI) (+Alt = hide)"; }
	virtual RString GetDisplayValue()
	{
		PlayerController pc = GamePreferences::m_AutoPlay.Get();
		switch( pc )
		{
		case PC_HUMAN:		return OFF.GetValue();	break;
		case PC_AUTOPLAY:	return ON.GetValue();	break;
		case PC_CPU:		return CPU.GetValue();	break;
		default:
			FAIL_M(ssprintf("Invalid PlayerController: %i", pc));
		}
	}
	virtual Type GetType() const { return IDebugLine::gameplay_only; }
	virtual bool IsEnabled() { return GamePreferences::m_AutoPlay.Get() != PC_HUMAN; }
	virtual void DoAndLog( RString &sMessageOut )
	{
		ASSERT( GAMESTATE->GetMasterPlayerNumber() != PLAYER_INVALID );
		PlayerController pc = GAMESTATE->m_pPlayerState[GAMESTATE->GetMasterPlayerNumber()]->m_PlayerController;
		bool bHoldingShift = 
			INPUTFILTER->IsBeingPressed( DeviceInput(DEVICE_KEYBOARD, KEY_LSHIFT) ) || 
			INPUTFILTER->IsBeingPressed( DeviceInput(DEVICE_KEYBOARD, KEY_RSHIFT) );
		if( bHoldingShift )
			pc = (pc==PC_CPU) ? PC_HUMAN : PC_CPU;
		else
			pc = (pc==PC_AUTOPLAY) ? PC_HUMAN : PC_AUTOPLAY;
		GamePreferences::m_AutoPlay.Set( pc );
		FOREACH_HumanPlayer(p)
			GAMESTATE->m_pPlayerState[p]->m_PlayerController = GamePreferences::m_AutoPlay;
		FOREACH_MultiPlayer(p)
			GAMESTATE->m_pMultiPlayerState[p]->m_PlayerController = GamePreferences::m_AutoPlay;

		// Hide Autoplay if Alt is held down
		bool bHoldingAlt = 
			INPUTFILTER->IsBeingPressed( DeviceInput(DEVICE_KEYBOARD, KEY_LALT) ) || 
			INPUTFILTER->IsBeingPressed( DeviceInput(DEVICE_KEYBOARD, KEY_RALT) );
		ScreenSyncOverlay::SetShowAutoplay( !bHoldingAlt );

		IDebugLine::DoAndLog( sMessageOut );
	}
};

class DebugLineAssist : public IDebugLine
{
	virtual RString GetDisplayTitle() { return ASSIST.GetValue(); }
	virtual Type GetType() const { return gameplay_only; }
	virtual RString GetDisplayValue() { 
		SongOptions so;
		so.m_bAssistClap = GAMESTATE->m_SongOptions.GetSong().m_bAssistClap;
		so.m_bAssistMetronome = GAMESTATE->m_SongOptions.GetSong().m_bAssistMetronome;
		if( so.m_bAssistClap || so.m_bAssistMetronome )
			return so.GetLocalizedString();
		else
			return OFF.GetValue();
	}
	virtual bool IsEnabled() { return GAMESTATE->m_SongOptions.GetSong().m_bAssistClap || GAMESTATE->m_SongOptions.GetSong().m_bAssistMetronome; }
	virtual void DoAndLog( RString &sMessageOut )
	{
		ASSERT( GAMESTATE->GetMasterPlayerNumber() != PLAYER_INVALID );
		bool bHoldingShift = INPUTFILTER->IsBeingPressed( DeviceInput(DEVICE_KEYBOARD, KEY_LSHIFT) );
		bool b;
		if( bHoldingShift )
			b = !GAMESTATE->m_SongOptions.GetSong().m_bAssistMetronome;
		else
			b = !GAMESTATE->m_SongOptions.GetSong().m_bAssistClap;
		if( bHoldingShift )
			SO_GROUP_ASSIGN( GAMESTATE->m_SongOptions, ModsLevel_Preferred, m_bAssistMetronome, b );
		else
			SO_GROUP_ASSIGN( GAMESTATE->m_SongOptions, ModsLevel_Preferred, m_bAssistClap, b );

		IDebugLine::DoAndLog( sMessageOut );
	}
};

class DebugLineAutosync : public IDebugLine
{
	virtual RString GetDisplayTitle() { return AUTOSYNC.GetValue(); }
	virtual RString GetDisplayValue()
	{ 
		AutosyncType type = GAMESTATE->m_SongOptions.GetSong().m_AutosyncType;
		switch( type )
		{
		case AutosyncType_Off: 	return OFF.GetValue();  		break;
		case AutosyncType_Song:	return SONG.GetValue(); 		break;
		case AutosyncType_Machine:	return MACHINE.GetValue(); 		break;
		case AutosyncType_Tempo:	return SYNC_TEMPO.GetValue();		break;
		default:
			FAIL_M(ssprintf("Invalid autosync type: %i", type));
		}
	}
	virtual Type GetType() const { return IDebugLine::gameplay_only; }
	virtual bool IsEnabled() { return GAMESTATE->m_SongOptions.GetSong().m_AutosyncType!=AutosyncType_Off; }
	virtual void DoAndLog( RString &sMessageOut )
	{
		int as = GAMESTATE->m_SongOptions.GetSong().m_AutosyncType + 1;
		bool bAllowSongAutosync = !GAMESTATE->IsCourseMode();
		if( !bAllowSongAutosync  && 
		  ( as == AutosyncType_Song || as == AutosyncType_Tempo ) )
			as = AutosyncType_Machine;
		wrap( as, NUM_AutosyncType );
		SO_GROUP_ASSIGN( GAMESTATE->m_SongOptions, ModsLevel_Song, m_AutosyncType, AutosyncType(as) );
		MESSAGEMAN->Broadcast( Message_AutosyncChanged );
		IDebugLine::DoAndLog( sMessageOut );
	}
};

class DebugLineCoinMode : public IDebugLine
{
	virtual RString GetDisplayTitle() { return COIN_MODE.GetValue(); }
	virtual RString GetDisplayValue() { return CoinModeToString(GAMESTATE->GetCoinMode()); }
	virtual bool IsEnabled() { return true; }
	virtual void DoAndLog( RString &sMessageOut )
	{
		if (GAMESTATE->GetCoinMode() == CoinMode_Home)
			GamePreferences::m_CoinMode.Set(CoinMode_Free);
		else if (GAMESTATE->GetCoinMode() == CoinMode_Free && !GAMESTATE->IsEventMode())
			GamePreferences::m_CoinMode.Set(CoinMode_Pay);
		else
			GamePreferences::m_CoinMode.Set(CoinMode_Home);
		SCREENMAN->RefreshCreditsMessages();
		IDebugLine::DoAndLog( sMessageOut );
	}
};

class DebugLineSlow : public IDebugLine
{
	virtual RString GetDisplayTitle() { return SLOW.GetValue(); }
	virtual bool IsEnabled() { return g_bIsSlow; }
	virtual void DoAndLog( RString &sMessageOut )
	{
		g_bIsSlow = !g_bIsSlow;
		SetSpeed();
		IDebugLine::DoAndLog( sMessageOut );
	}
};

class DebugLineHalt : public IDebugLine
{
	virtual RString GetDisplayTitle() { return HALT.GetValue(); }
	virtual bool IsEnabled() { return g_bIsHalt; }
	virtual void DoAndLog( RString &sMessageOut )
	{
		g_bIsHalt = !g_bIsHalt;
		g_HaltTimer.Touch();
		SetSpeed();
		IDebugLine::DoAndLog( sMessageOut );
	}
};

class DebugLineLightsDebug : public IDebugLine
{
	virtual RString GetDisplayTitle() { return LIGHTS_DEBUG.GetValue(); }
	virtual bool IsEnabled() { return PREFSMAN->m_bDebugLights.Get(); }
	virtual void DoAndLog( RString &sMessageOut )
	{
		PREFSMAN->m_bDebugLights.Set( !PREFSMAN->m_bDebugLights );
		IDebugLine::DoAndLog( sMessageOut );
	}
};

class DebugLineMonkeyInput : public IDebugLine
{
	virtual RString GetDisplayTitle() { return MONKEY_INPUT.GetValue(); }
	virtual bool IsEnabled() { return PREFSMAN->m_bMonkeyInput.Get(); }
	virtual void DoAndLog( RString &sMessageOut )
	{
		PREFSMAN->m_bMonkeyInput.Set( !PREFSMAN->m_bMonkeyInput );
		IDebugLine::DoAndLog( sMessageOut );
	}
};

class DebugLineStats : public IDebugLine
{
	virtual RString GetDisplayTitle() { return RENDERING_STATS.GetValue(); }
	virtual bool IsEnabled() { return PREFSMAN->m_bShowStats.Get(); }
	virtual void DoAndLog( RString &sMessageOut )
	{
		PREFSMAN->m_bShowStats.Set( !PREFSMAN->m_bShowStats );
		IDebugLine::DoAndLog( sMessageOut );
	}
};

class DebugLineVsync : public IDebugLine
{
	virtual RString GetDisplayTitle() { return VSYNC.GetValue(); }
	virtual bool IsEnabled() { return PREFSMAN->m_bVsync.Get(); }
	virtual void DoAndLog( RString &sMessageOut )
	{
		PREFSMAN->m_bVsync.Set( !PREFSMAN->m_bVsync );
		StepMania::ApplyGraphicOptions();
		IDebugLine::DoAndLog( sMessageOut );
	}
};

class DebugLineAllowMultitexture : public IDebugLine
{
	virtual RString GetDisplayTitle() { return MULTITEXTURE.GetValue(); }
	virtual bool IsEnabled() { return PREFSMAN->m_bAllowMultitexture.Get(); }
	virtual void DoAndLog( RString &sMessageOut )
	{
		PREFSMAN->m_bAllowMultitexture.Set( !PREFSMAN->m_bAllowMultitexture );
		IDebugLine::DoAndLog( sMessageOut );
	}
};

class DebugLineShowMasks : public IDebugLine
{
	virtual RString GetDisplayTitle() { return SCREEN_SHOW_MASKS.GetValue(); }
	virtual bool IsEnabled() { return GetPref()->Get(); }
	virtual RString GetPageName() const { return "Theme"; }
	virtual void DoAndLog( RString &sMessageOut )
	{
		GetPref()->Set( !GetPref()->Get() );
		IDebugLine::DoAndLog( sMessageOut );
	}

	Preference<bool> *GetPref()
	{
		return Preference<bool>::GetPreferenceByName("ShowMasks");
	}
};

static ProfileSlot g_ProfileSlot = ProfileSlot_Machine;
static bool IsSelectProfilePersistent()
{
	if( g_ProfileSlot == ProfileSlot_Machine )
		return true;
	return PROFILEMAN->IsPersistentProfile( (PlayerNumber) g_ProfileSlot );
}

class DebugLineProfileSlot : public IDebugLine
{
	virtual RString GetDisplayTitle() { return PROFILE.GetValue(); }
	virtual RString GetDisplayValue()
	{
		switch( g_ProfileSlot )
		{
			case ProfileSlot_Machine: return "Machine";
			case ProfileSlot_Player1: return "Player 1";
			case ProfileSlot_Player2: return "Player 2";
			default: return RString();
		}
	}
	virtual bool IsEnabled() { return IsSelectProfilePersistent(); }
	virtual RString GetPageName() const { return "Profiles"; }
	virtual void DoAndLog( RString &sMessageOut )
	{
		enum_add( g_ProfileSlot, +1 );
		if( g_ProfileSlot == NUM_ProfileSlot )
			g_ProfileSlot = ProfileSlot_Player1;

		IDebugLine::DoAndLog( sMessageOut );
	}
};


class DebugLineClearProfileStats : public IDebugLine
{
	virtual RString GetDisplayTitle() { return CLEAR_PROFILE_STATS.GetValue(); }
	virtual RString GetDisplayValue() { return RString(); }
	virtual bool IsEnabled() { return IsSelectProfilePersistent(); }
	virtual RString GetPageName() const { return "Profiles"; }
	virtual void DoAndLog( RString &sMessageOut )
	{
		Profile *pProfile = PROFILEMAN->GetProfile( g_ProfileSlot );
		pProfile->ClearStats();
		IDebugLine::DoAndLog( sMessageOut );
	}
};

static HighScore MakeRandomHighScore( float fPercentDP )
{
	HighScore hs;
	hs.SetName( "FAKE" );
	Grade g = (Grade)SCALE( RandomInt(6), 0, 4, Grade_Tier01, Grade_Tier06 );
	if( g == Grade_Tier06 )
		g = Grade_Failed;
	hs.SetGrade( g );
	hs.SetScore( RandomInt(100*1000) );
	hs.SetPercentDP( fPercentDP );
	hs.SetAliveSeconds( randomf(30.0f, 100.0f) );
	PlayerOptions po;
	po.ChooseRandomModifiers();
	hs.SetModifiers( po.GetString() );
	hs.SetDateTime( DateTime::GetNowDateTime() );
	hs.SetPlayerGuid( Profile::MakeGuid() );
	hs.SetMachineGuid( Profile::MakeGuid() );
	hs.SetProductID( RandomInt(10) );
	FOREACH_ENUM( TapNoteScore, tns )
		hs.SetTapNoteScore( tns, RandomInt(100) );
	FOREACH_ENUM( HoldNoteScore, hns )
		hs.SetHoldNoteScore( hns, RandomInt(100) );
	RadarValues rv;
	FOREACH_ENUM( RadarCategory, rc )
	{
		rv[rc] = randomf( 0, 1 );
	}
	hs.SetRadarValues( rv );

	return hs;
}

static void FillProfileStats( Profile *pProfile )
{
	pProfile->InitSongScores(); 
	pProfile->InitCourseScores();

	static int s_iCount = 0;
	// Choose a percent for all scores. This is useful for testing unlocks
	// where some elements are unlocked at a certain percent complete.
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
			if( rand() % 5 )
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
			if( rand() % 5 )
				pProfile->IncrementCoursePlayCount( *pCourse, *pTrail );
			for( int i=0; i<iCount; i++ )
			{
				int iIndex = 0;
				pProfile->AddCourseHighScore( *pCourse, *pTrail, MakeRandomHighScore(fPercentDP), iIndex );
			}
		}
	}

	SCREENMAN->ZeroNextUpdate();
}

class DebugLineFillProfileStats : public IDebugLine
{
	virtual RString GetDisplayTitle() { return FILL_PROFILE_STATS.GetValue(); }
	virtual RString GetDisplayValue() { return RString(); }
	virtual bool IsEnabled() { return IsSelectProfilePersistent(); }
	virtual RString GetPageName() const { return "Profiles"; }
	virtual void DoAndLog( RString &sMessageOut )
	{
		Profile* pProfile = PROFILEMAN->GetProfile( g_ProfileSlot );
		FillProfileStats( pProfile );
		IDebugLine::DoAndLog( sMessageOut );
	}
};

class DebugLineSendNotesEnded : public IDebugLine
{
	virtual RString GetDisplayTitle() { return SEND_NOTES_ENDED.GetValue(); }
	virtual RString GetDisplayValue() { return RString(); }
	virtual bool IsEnabled() { return true; }
	virtual void DoAndLog( RString &sMessageOut )
	{
		SCREENMAN->PostMessageToTopScreen( SM_NotesEnded, 0 );
		IDebugLine::DoAndLog( sMessageOut );
	}
};

class DebugLineResetKeyMapping : public IDebugLine
{
	virtual RString GetDisplayTitle() { return RESET_KEY_MAP.GetValue(); }
	virtual RString GetDisplayValue() { return RString(); }
	virtual bool IsEnabled() { return true; }
	virtual void DoAndLog( RString &sMessageOut )
	{
		INPUTMAPPER->ResetMappingsToDefault();
		INPUTMAPPER->SaveMappingsToDisk();
		IDebugLine::DoAndLog( sMessageOut );
	}
};

class DebugLineMuteActions : public IDebugLine
{
	virtual RString GetDisplayTitle() { return MUTE_ACTIONS.GetValue(); }
	virtual RString GetDisplayValue() { return RString(); }
	virtual bool IsEnabled() { return PREFSMAN->m_MuteActions; }
	virtual void DoAndLog( RString &sMessageOut )
	{
		PREFSMAN->m_MuteActions.Set(!PREFSMAN->m_MuteActions);
		SCREENMAN->SystemMessage(PREFSMAN->m_MuteActions ? MUTE_ACTIONS_ON.GetValue() : MUTE_ACTIONS_OFF.GetValue());
		IDebugLine::DoAndLog( sMessageOut );
	}
};

class DebugLineReloadCurrentScreen : public IDebugLine
{
	virtual RString GetDisplayTitle() { return RELOAD.GetValue(); }
	virtual RString GetDisplayValue() { return SCREENMAN && SCREENMAN->GetTopScreen()? SCREENMAN->GetTopScreen()->GetName() : RString(); }
	virtual bool IsEnabled() { return true; }
	virtual RString GetPageName() const { return "Theme"; }
	virtual void DoAndLog( RString &sMessageOut )
	{
		RString sScreenName = SCREENMAN->GetScreen(0)->GetName();
		SCREENMAN->PopAllScreens();

		SOUND->StopMusic();
		//StepMania::ResetGame();

		SCREENMAN->SetNewScreen( sScreenName );
		IDebugLine::DoAndLog( sMessageOut );
		sMessageOut = "";
	}
};

class DebugLineRestartCurrentScreen : public IDebugLine
{
	virtual RString GetDisplayTitle() { return RESTART.GetValue(); }
	virtual RString GetDisplayValue() { return SCREENMAN && SCREENMAN->GetTopScreen()? SCREENMAN->GetTopScreen()->GetName() : RString(); }
	virtual bool IsEnabled() { return true; }
	virtual bool ForceOffAfterUse() const { return true; }
	virtual RString GetPageName() const { return "Theme"; }
	virtual void DoAndLog( RString &sMessageOut )
	{
		SCREENMAN->GetTopScreen()->BeginScreen();
		IDebugLine::DoAndLog( sMessageOut );
		sMessageOut = "";
	}
};

class DebugLineCurrentScreenOn : public IDebugLine
{
	virtual RString GetDisplayTitle() { return SCREEN_ON.GetValue(); }
	virtual RString GetDisplayValue() { return SCREENMAN && SCREENMAN->GetTopScreen()? SCREENMAN->GetTopScreen()->GetName() : RString(); }
	virtual bool IsEnabled() { return true; }
	virtual bool ForceOffAfterUse() const { return true; }
	virtual RString GetPageName() const { return "Theme"; }
	virtual void DoAndLog( RString &sMessageOut )
	{
		SCREENMAN->GetTopScreen()->PlayCommand("On");
		IDebugLine::DoAndLog( sMessageOut );
		sMessageOut = "";
	}
};

class DebugLineCurrentScreenOff : public IDebugLine
{
	virtual RString GetDisplayTitle() { return SCREEN_OFF.GetValue(); }
	virtual RString GetDisplayValue() { return SCREENMAN && SCREENMAN->GetTopScreen()? SCREENMAN->GetTopScreen()->GetName() : RString(); }
	virtual bool IsEnabled() { return true; }
	virtual bool ForceOffAfterUse() const { return true; }
	virtual RString GetPageName() const { return "Theme"; }
	virtual void DoAndLog( RString &sMessageOut )
	{
		SCREENMAN->GetTopScreen()->PlayCommand("Off");
		IDebugLine::DoAndLog( sMessageOut );
		sMessageOut = "";
	}
};

class DebugLineReloadTheme : public IDebugLine
{
	virtual RString GetDisplayTitle() { return RELOAD_THEME_AND_TEXTURES.GetValue(); }
	virtual RString GetDisplayValue() { return RString(); }
	virtual bool IsEnabled() { return true; }
	virtual RString GetPageName() const { return "Theme"; }
	virtual void DoAndLog( RString &sMessageOut )
	{
		THEME->ReloadMetrics();
		TEXTUREMAN->ReloadAll();
		NOTESKIN->RefreshNoteSkinData( GAMESTATE->m_pCurGame );
		CodeDetector::RefreshCacheItems();
		// HACK: Don't update text below. Return immediately because this screen
		// was just destroyed as part of the theme reload.
		IDebugLine::DoAndLog( sMessageOut );
	}
};

class DebugLineReloadOverlayScreens : public IDebugLine
{
	virtual RString GetDisplayTitle() { return RELOAD_OVERLAY_SCREENS.GetValue(); }
	virtual RString GetDisplayValue() { return RString(); }
	virtual bool IsEnabled() { return true; }
	virtual RString GetPageName() const { return "Theme"; }
	virtual void DoAndLog( RString &sMessageOut )
	{
		SCREENMAN->ReloadOverlayScreensAfterInputFinishes();
		IDebugLine::DoAndLog(sMessageOut);
	}
};

class DebugLineToggleErrors : public IDebugLine
{
	virtual RString GetDisplayTitle() { return TOGGLE_ERRORS.GetValue(); }
	virtual RString GetDisplayValue() { return RString(); }
	virtual bool IsEnabled() { return PREFSMAN->m_show_theme_errors; }
	virtual RString GetPageName() const { return "Theme"; }
	virtual void DoAndLog( RString &sMessageOut )
	{
		PREFSMAN->m_show_theme_errors.Set(!PREFSMAN->m_show_theme_errors);
		IDebugLine::DoAndLog(sMessageOut);
	}
};

class DebugLineShowRecentErrors : public IDebugLine
{
	virtual RString GetDisplayTitle() { return SHOW_RECENT_ERRORS.GetValue(); }
	virtual RString GetDisplayValue() { return RString(); }
	virtual bool IsEnabled() { return true; }
	virtual RString GetPageName() const { return "Theme"; }
	virtual void DoAndLog( RString &sMessageOut )
	{
		Message msg("ToggleScriptError");
		MESSAGEMAN->Broadcast(msg);
		IDebugLine::DoAndLog(sMessageOut);
	}
};

class DebugLineClearErrors : public IDebugLine
{
	virtual RString GetDisplayTitle() { return CLEAR_ERRORS.GetValue(); }
	virtual RString GetDisplayValue() { return RString(); }
	virtual bool IsEnabled() { return true; }
	virtual RString GetPageName() const { return "Theme"; }
	virtual void DoAndLog( RString &sMessageOut )
	{
		Message msg("ClearScriptError");
		MESSAGEMAN->Broadcast(msg);
		IDebugLine::DoAndLog(sMessageOut);
	}
};

class DebugLineConvertXML : public IDebugLine
{
	virtual RString GetDisplayTitle() { return CONVERT_XML.GetValue(); }
	virtual RString GetDisplayValue() { return RString(); }
	virtual bool IsEnabled() { return true; }
	virtual RString GetPageName() const { return "Theme"; }
	virtual void DoAndLog( RString &sMessageOut )
	{
		Song* cur_song= GAMESTATE->m_pCurSong;
		if(cur_song)
		{
			convert_xmls_in_dir(cur_song->GetSongDir() + "/");
			IDebugLine::DoAndLog(sMessageOut);
		}
	}
};

class DebugLineWriteProfiles : public IDebugLine
{
	virtual RString GetDisplayTitle() { return WRITE_PROFILES.GetValue(); }
	virtual RString GetDisplayValue() { return RString(); }
	virtual bool IsEnabled() { return IsSelectProfilePersistent(); }
	virtual RString GetPageName() const { return "Profiles"; }
	virtual void DoAndLog( RString &sMessageOut )
	{
		// Also save bookkeeping and profile info for debugging
		// so we don't have to play through a whole song to get new output.
		if( g_ProfileSlot == ProfileSlot_Machine )
			GAMESTATE->SaveLocalData();
		else
		{
			PlayerNumber pn = (PlayerNumber) g_ProfileSlot;
			GAMESTATE->SaveCurrentSettingsToProfile(pn);
			GAMESTATE->SavePlayerProfile( pn );
		}
		IDebugLine::DoAndLog( sMessageOut );
	}
};

class DebugLineWritePreferences : public IDebugLine
{
	virtual RString GetDisplayTitle() { return WRITE_PREFERENCES.GetValue(); }
	virtual RString GetDisplayValue() { return RString(); }
	virtual bool IsEnabled() { return true; }
	virtual void DoAndLog( RString &sMessageOut )
	{
		PREFSMAN->SavePrefsToDisk();
		IDebugLine::DoAndLog( sMessageOut );
	}
};

class DebugLineMenuTimer : public IDebugLine
{
	virtual RString GetDisplayTitle() { return MENU_TIMER.GetValue(); }
	virtual RString GetDisplayValue() { return RString(); }
	virtual bool IsEnabled() { return PREFSMAN->m_bMenuTimer.Get(); }
	virtual void DoAndLog( RString &sMessageOut )
	{
		PREFSMAN->m_bMenuTimer.Set( !PREFSMAN->m_bMenuTimer );
		IDebugLine::DoAndLog( sMessageOut );
	}
};

class DebugLineFlushLog : public IDebugLine
{
	virtual RString GetDisplayTitle() { return FLUSH_LOG.GetValue(); }
	virtual RString GetDisplayValue() { return RString(); }
	virtual bool IsEnabled() { return true; }
	virtual void DoAndLog( RString &sMessageOut )
	{
		LOG->Flush();
		IDebugLine::DoAndLog( sMessageOut );
	}
};

class DebugLinePullBackCamera : public IDebugLine
{
	virtual RString GetDisplayTitle() { return PULL_BACK_CAMERA.GetValue(); }
	virtual RString GetDisplayValue() { return RString(); }
	virtual bool IsEnabled() { return g_fImageScaleDestination != 1; }
	virtual void DoAndLog( RString &sMessageOut )
	{
		if( g_fImageScaleDestination == 1 )
			g_fImageScaleDestination = 0.5f;
		else
			g_fImageScaleDestination = 1;
		IDebugLine::DoAndLog( sMessageOut );
	}
};

class DebugLineVolumeUp : public IDebugLine
{
	virtual RString GetDisplayTitle() { return VOLUME_UP.GetValue(); }
	virtual RString GetDisplayValue() { return ssprintf("%.0f%%", GetPref()->Get()*100); }
	virtual bool IsEnabled() { return true; }
	virtual void DoAndLog( RString &sMessageOut )
	{
		ChangeVolume( +0.1f );
		IDebugLine::DoAndLog( sMessageOut );
	}
	Preference<float> *GetPref()
	{
		return Preference<float>::GetPreferenceByName("SoundVolume");
	}
};

class DebugLineVolumeDown : public IDebugLine
{
	virtual RString GetDisplayTitle() { return VOLUME_DOWN.GetValue(); }
	virtual RString GetDisplayValue() { return RString(); }
	virtual bool IsEnabled() { return true; }
	virtual void DoAndLog( RString &sMessageOut )
	{
		ChangeVolume( -0.1f );
		IDebugLine::DoAndLog( sMessageOut );
		sMessageOut += " - " + ssprintf("%.0f%%",GetPref()->Get()*100);
	}
	Preference<float> *GetPref()
	{
		return Preference<float>::GetPreferenceByName("SoundVolume");
	}
};

class DebugLineVisualDelayUp : public IDebugLine
{
	virtual RString GetDisplayTitle() { return VISUAL_DELAY_UP.GetValue(); }
	virtual RString GetDisplayValue() { return ssprintf("%.03f",GetPref()->Get()); }
	virtual bool IsEnabled() { return true; }
	virtual void DoAndLog( RString &sMessageOut )
	{
		ChangeVisualDelay( +0.001f );
		IDebugLine::DoAndLog( sMessageOut );
	}
	Preference<float> *GetPref()
	{
		return Preference<float>::GetPreferenceByName("VisualDelaySeconds");
	}
};

class DebugLineVisualDelayDown : public IDebugLine
{
	virtual RString GetDisplayTitle() { return VISUAL_DELAY_DOWN.GetValue(); }
	virtual RString GetDisplayValue() { return RString(); }
	virtual bool IsEnabled() { return true; }
	virtual void DoAndLog( RString &sMessageOut )
	{
		ChangeVisualDelay( -0.001f );
		IDebugLine::DoAndLog( sMessageOut );
		sMessageOut += " - " + ssprintf("%.03f",GetPref()->Get());
	}
	Preference<float> *GetPref()
	{
		return Preference<float>::GetPreferenceByName("VisualDelaySeconds");
	}
};

class DebugLineForceCrash : public IDebugLine
{
	virtual RString GetDisplayTitle() { return FORCE_CRASH.GetValue(); }
	virtual RString GetDisplayValue() { return RString(); }
	virtual bool IsEnabled() { return false; }
	virtual void DoAndLog( RString &sMessageOut ) { FAIL_M("DebugLineCrash"); }
};

class DebugLineUptime : public IDebugLine
{
	virtual RString GetDisplayTitle() { return UPTIME.GetValue(); }
	virtual RString GetDisplayValue() { return SecondsToMMSSMsMsMs(RageTimer::GetTimeSinceStart()); }
	virtual bool IsEnabled() { return false; }
	virtual void DoAndLog( RString &sMessageOut ) {}
};

/* #ifdef out the lines below if you don't want them to appear on certain
 * platforms.  This is easier than #ifdefing the whole DebugLine definitions
 * that can span pages.
 */

#define DECLARE_ONE( x ) static x g_##x
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
DECLARE_ONE( DebugLineShowMasks );
DECLARE_ONE( DebugLineProfileSlot );
DECLARE_ONE( DebugLineClearProfileStats );
DECLARE_ONE( DebugLineFillProfileStats );
DECLARE_ONE( DebugLineSendNotesEnded );
DECLARE_ONE( DebugLineReloadCurrentScreen );
DECLARE_ONE( DebugLineRestartCurrentScreen );
DECLARE_ONE( DebugLineCurrentScreenOn );
DECLARE_ONE( DebugLineCurrentScreenOff );
DECLARE_ONE( DebugLineReloadTheme );
DECLARE_ONE( DebugLineReloadOverlayScreens );
DECLARE_ONE( DebugLineToggleErrors );
DECLARE_ONE( DebugLineShowRecentErrors );
DECLARE_ONE( DebugLineClearErrors );
DECLARE_ONE( DebugLineConvertXML );
DECLARE_ONE( DebugLineWriteProfiles );
DECLARE_ONE( DebugLineWritePreferences );
DECLARE_ONE( DebugLineMenuTimer );
DECLARE_ONE( DebugLineFlushLog );
DECLARE_ONE( DebugLinePullBackCamera );
DECLARE_ONE( DebugLineVolumeDown );
DECLARE_ONE( DebugLineVolumeUp );
DECLARE_ONE( DebugLineVisualDelayDown );
DECLARE_ONE( DebugLineVisualDelayUp );
DECLARE_ONE( DebugLineForceCrash );
DECLARE_ONE( DebugLineUptime );
DECLARE_ONE( DebugLineResetKeyMapping );
DECLARE_ONE( DebugLineMuteActions );


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
