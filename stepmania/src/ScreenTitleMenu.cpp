#include "global.h"
/*
-----------------------------------------------------------------------------
 Class: ScreenTitleMenu

 Desc: See header.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/

#include "ScreenTitleMenu.h"
#include "ScreenManager.h"
#include "GameConstantsAndTypes.h"
#include "RageUtil.h"
#include "StepMania.h"
#include "PrefsManager.h"
#include "RageLog.h"
#include "SongManager.h"
#include "AnnouncerManager.h"
#include "GameState.h"
#include "GameManager.h"
#include "InputMapper.h"
#include "ThemeManager.h"
#include "SDL_utils.h"
#include "RageSoundManager.h"


#define HELP_X						THEME->GetMetricF("ScreenTitleMenu","HelpX")
#define HELP_Y						THEME->GetMetricF("ScreenTitleMenu","HelpY")
#define CHOICES_X					THEME->GetMetricF("ScreenTitleMenu","ChoicesX")
#define CHOICES_START_Y				THEME->GetMetricF("ScreenTitleMenu","ChoicesStartY")
#define CHOICES_SPACING_Y			THEME->GetMetricF("ScreenTitleMenu","ChoicesSpacingY")
#define CHOICES_SHADOW_LENGTH		THEME->GetMetricF("ScreenTitleMenu","ChoicesShadowLength")
#define COLOR_NOT_SELECTED			THEME->GetMetricC("ScreenTitleMenu","ColorNotSelected")
#define COLOR_SELECTED				THEME->GetMetricC("ScreenTitleMenu","ColorSelected")
#define ZOOM_NOT_SELECTED			THEME->GetMetricF("ScreenTitleMenu","ZoomNotSelected")
#define ZOOM_SELECTED				THEME->GetMetricF("ScreenTitleMenu","ZoomSelected")
#define SECONDS_BETWEEN_COMMENTS	THEME->GetMetricF("ScreenTitleMenu","SecondsBetweenComments")
#define SECONDS_BEFORE_ATTRACT		THEME->GetMetricF("ScreenTitleMenu","SecondsBeforeAttract")
#define HELP_TEXT_HOME				THEME->GetMetric("ScreenTitleMenu","HelpTextHome")
#define HELP_TEXT_PAY				THEME->GetMetric("ScreenTitleMenu","HelpTextPay")
#define HELP_TEXT_FREE				THEME->GetMetric("ScreenTitleMenu","HelpTextFree")
#define NEXT_SCREEN					THEME->GetMetric("ScreenTitleMenu","NextScreen")

const ScreenMessage SM_PlayComment			=	ScreenMessage(SM_User+1);
const ScreenMessage SM_GoToAttractLoop		=	ScreenMessage(SM_User+13);

ScreenTitleMenu::ScreenTitleMenu()
{
	LOG->Trace( "ScreenTitleMenu::ScreenTitleMenu()" );

	GAMESTATE->m_bPlayersCanJoin = true;

	if( PREFSMAN->m_CoinMode!=PrefsManager::COIN_HOME  &&  PREFSMAN->m_bJointPremium )
	{		
		m_JointPremium.LoadFromAniDir( THEME->GetPathTo("BGAnimations","ScreenTitleMenu joint premium") );
		this->AddChild( &m_JointPremium );
	}

	switch( PREFSMAN->m_CoinMode )
	{
	case PrefsManager::COIN_HOME:
		// do nothing
		break;
	case PrefsManager::COIN_PAY:
		m_CoinMode.LoadFromAniDir( THEME->GetPathTo("BGAnimations","ScreenTitleMenu pay") );
		this->AddChild( &m_CoinMode );
		break;
	case PrefsManager::COIN_FREE:
		m_CoinMode.LoadFromAniDir( THEME->GetPathTo("BGAnimations","ScreenTitleMenu free") );
		this->AddChild( &m_CoinMode );
		break;
	default:
		ASSERT(0);
	}
	
	m_textHelp.LoadFromFont( THEME->GetPathTo("Fonts","ScreenTitleMenu help") );
	CString sHelpText;
	switch( PREFSMAN->m_CoinMode )
	{
	case PrefsManager::COIN_HOME:	sHelpText = HELP_TEXT_HOME;	break;
	case PrefsManager::COIN_PAY:	sHelpText = HELP_TEXT_PAY;	break;
	case PrefsManager::COIN_FREE:	sHelpText = HELP_TEXT_FREE;	break;
	default:			ASSERT(0);
	}
	m_textHelp.SetText( sHelpText );
	m_textHelp.SetXY( HELP_X, HELP_Y );
	m_textHelp.SetZoom( 0.5f );
	m_textHelp.SetEffectDiffuseBlink();
	m_textHelp.EnableShadow( true );
	m_textHelp.SetShadowLength( 2 );
	this->AddChild( &m_textHelp );

	switch( PREFSMAN->m_CoinMode )
	{
	case PrefsManager::COIN_HOME:
		int i;
		for( i=0; i<NUM_CHOICES; i++ )
		{
			m_textChoice[i].LoadFromFont( THEME->GetPathTo("Fonts","ScreenTitleMenu choices") );
			m_textChoice[i].SetText( ChoiceToString((Choice)i) );
			m_textChoice[i].SetXY( CHOICES_X, CHOICES_START_Y + i*CHOICES_SPACING_Y );
			m_textChoice[i].SetShadowLength( CHOICES_SHADOW_LENGTH );
			m_textChoice[i].EnableShadow( true );
			this->AddChild( &m_textChoice[i] );
		}	
		break;
	case PrefsManager::COIN_PAY:
		break;
	case PrefsManager::COIN_FREE:
		break;
	default:
		ASSERT(0);
	}


	SOUNDMAN->PlayOnceFromDir( ANNOUNCER->GetPathTo("title menu game name") );


	m_soundAttract.Load( ANNOUNCER->GetPathTo("title menu attract") );
	m_soundChange.Load( THEME->GetPathTo("Sounds","ScreenTitleMenu change") );	
	m_soundSelect.Load( THEME->GetPathTo("Sounds","Common start") );
	m_soundInvalid.Load( THEME->GetPathTo("Sounds","Common invalid") );

	m_Choice = CHOICE_GAME_START;

	for( int i=0; i<NUM_CHOICES; i++ )
		LoseFocus( i );
	GainFocus( m_Choice );

	SOUNDMAN->PlayMusic( THEME->GetPathTo("Sounds","ScreenTitleMenu music") );

	this->PostScreenMessage( SM_PlayComment, SECONDS_BETWEEN_COMMENTS);

	this->MoveToTail( &m_In );	// put it in the back so it covers up the stuff we just added
	this->MoveToTail( &m_Out );	// put it in the back so it covers up the stuff we just added
}

ScreenTitleMenu::~ScreenTitleMenu()
{
	LOG->Trace( "ScreenTitleMenu::~ScreenTitleMenu()" );
}


void ScreenTitleMenu::Input( const DeviceInput& DeviceI, const InputEventType type, const GameInput &GameI, const MenuInput &MenuI, const StyleInput &StyleI )
{
	LOG->Trace( "ScreenTitleMenu::Input( %d-%d )", DeviceI.device, DeviceI.button );	// debugging gameport joystick problem

	if( type != IET_FIRST_PRESS )
		return;

	/* If the CoinMode was changed, we need to reload this screen
	 * so that the right choices will show */
	if( ScreenAttract::ChangeCoinModeInput( DeviceI, type, GameI, MenuI, StyleI ) )
	{
		SCREENMAN->SetNewScreen( "ScreenTitleMenu" );
	}

	if( !MenuI.IsValid() )
		return;

	switch( MenuI.button )
	{
	case MENU_BUTTON_UP:
		if( PREFSMAN->m_CoinMode != PrefsManager::COIN_HOME )
			break;
		if( m_In.IsTransitioning() || m_Out.IsTransitioning() )
			break;
		TimeToDemonstration.GetDeltaTime();	/* Reset the demonstration timer when a key is pressed. */
		LoseFocus( m_Choice );
		if( m_Choice == 0 ) // wrap around
			m_Choice = (Choice)((int)NUM_CHOICES); 
		m_Choice = Choice( m_Choice-1 );
		m_soundChange.Play();
		GainFocus( m_Choice );
		break;
	case MENU_BUTTON_DOWN:
		if( PREFSMAN->m_CoinMode != PrefsManager::COIN_HOME )
			break;
		if( m_In.IsTransitioning() || m_Out.IsTransitioning() )
			break;
		TimeToDemonstration.GetDeltaTime();	/* Reset the demonstration timer when a key is pressed. */
		LoseFocus( m_Choice );
		if( m_Choice == (int)ScreenTitleMenu::NUM_CHOICES-1 ) 
			m_Choice = (Choice)-1; // wrap around
		m_Choice = Choice( m_Choice+1 );
		m_soundChange.Play();
		GainFocus( m_Choice );
		break;
	case MENU_BUTTON_BACK:
		if( m_In.IsTransitioning() || m_Out.IsTransitioning() )
			break;
		m_Out.StartTransitioning( SM_GoToAttractLoop );
		break;
	case MENU_BUTTON_START:
		if( m_In.IsTransitioning() )
			break;
		/* break if the choice is invalid */
		if( m_Choice == CHOICE_JUKEBOX  ||
			m_Choice == CHOICE_EDIT )
		{
			if( SONGMAN->GetNumSongs() == 0 )
			{
				m_soundInvalid.Play();
				SCREENMAN->SystemMessage( "No songs are installed" );
				return;
			}
		}
		if( m_Choice == CHOICE_EXIT )
		{
			LOG->Trace("CHOICE_EXIT: shutting down");
			ExitGame();
			return;
		}

		if( Screen::JoinInput( DeviceI, type, GameI, MenuI, StyleI ) )
			if( !m_Out.IsTransitioning() )
				m_Out.StartTransitioning( SM_GoToNextScreen );
	}

//	Screen::Input( DeviceI, type, GameI, MenuI, StyleI );
}

void ScreenTitleMenu::Update( float fDelta )
{
	if(TimeToDemonstration.PeekDeltaTime() >= SECONDS_BEFORE_ATTRACT)
	{
		this->PostScreenMessage( SM_GoToAttractLoop, 0 );
		TimeToDemonstration.GetDeltaTime();
	}
	Screen::Update(fDelta);
}


void ScreenTitleMenu::HandleScreenMessage( const ScreenMessage SM )
{
	switch( SM )
	{
	case SM_PlayComment:
		m_soundAttract.PlayRandom();
		this->PostScreenMessage( SM_PlayComment, SECONDS_BETWEEN_COMMENTS );
		break;
	case SM_GoToNextScreen:
		switch( m_Choice )
		{
		case CHOICE_GAME_START:
			SCREENMAN->SetNewScreen( NEXT_SCREEN );
			break;
		case CHOICE_SELECT_GAME:
			SCREENMAN->SetNewScreen( "ScreenSelectGame" );
			break;
		/* At request, moved this into the options/operator menu -- Miryokuteki */
		/*case CHOICE_MAP_KEY_JOY:
			SCREENMAN->SetNewScreen( "ScreenMapControllers" );
			break;*/
		case CHOICE_OPTIONS:
			SCREENMAN->SetNewScreen( "ScreenOptionsMenu" );
			break;
		case CHOICE_JUKEBOX:
			SCREENMAN->SetNewScreen( "ScreenJukeboxMenu" );
			break;
		#ifdef DEBUG
		case CHOICE_SANDBOX:
			SCREENMAN->SetNewScreen( "ScreenTest" );
			break;
		#endif
		case CHOICE_EDIT:
			SCREENMAN->SetNewScreen( "ScreenEditMenu" );
			break;
		case CHOICE_EXIT:
		default:
			RAGE_ASSERT_M(0, "CHOICE_EXIT reached?");	// should never get here
			break;
		}
		break;
	case SM_GoToAttractLoop:
		SCREENMAN->SetNewScreen( "ScreenCompany" );
		break;
	}
}


void ScreenTitleMenu::LoseFocus( int iChoiceIndex )
{
	m_textChoice[iChoiceIndex].SetEffectNone();
	m_textChoice[iChoiceIndex].StopTweening();
	m_textChoice[iChoiceIndex].BeginTweening( 0.3f );
	m_textChoice[iChoiceIndex].SetTweenZoom( ZOOM_NOT_SELECTED );
}

void ScreenTitleMenu::GainFocus( int iChoiceIndex )
{
	m_textChoice[iChoiceIndex].StopTweening();
	m_textChoice[iChoiceIndex].BeginTweening( 0.3f );
	m_textChoice[iChoiceIndex].SetTweenZoom( ZOOM_SELECTED );
	RageColor color1, color2;
	color1 = COLOR_SELECTED;
	color2 = color1 * 0.5f;
	color2.a = 1;
	m_textChoice[iChoiceIndex].SetEffectDiffuseShift( 0.5f, color1, color2 );
}

