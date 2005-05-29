#include "global.h"
#include "ScreenTitleMenu.h"
#include "ScreenAttract.h"
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
#include "GameSoundManager.h"
#include "CodeDetector.h"
#include "RageTextureManager.h"
#include "UnlockSystem.h"
#include "ProductInfo.h"
#include "LightsManager.h"
#include "CodeDetector.h"
#include "CommonMetrics.h"
#include "Game.h"
#include "ScreenOptionsMasterPrefs.h"


#define LOGO_ON_COMMAND				THEME->GetMetric("ScreenTitleMenu","LogoOnCommand")
#define LOGO_HOME_ON_COMMAND		THEME->GetMetric("ScreenTitleMenu","LogoHomeOnCommand")
#define VERSION_ON_COMMAND			THEME->GetMetric("ScreenTitleMenu","VersionOnCommand")
#define SONGS_ON_COMMAND			THEME->GetMetric("ScreenTitleMenu","SongsOnCommand")
#define MAX_STAGES_ON_COMMAND		THEME->GetMetric("ScreenTitleMenu","MaxStagesOnCommand")
#define MAX_STAGES_TEXT				THEME->GetMetric("ScreenTitleMenu","MaxStagesText")
#define LIFE_DIFFICULTY_ON_COMMAND	THEME->GetMetric("ScreenTitleMenu","LifeDifficultyOnCommand")
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
#define MENU_TEXT_ALIGN				THEME->GetMetricI("ScreenTitleMenu","MenuTextAlign")
#define SECONDS_BETWEEN_COMMENTS	THEME->GetMetricF("ScreenTitleMenu","SecondsBetweenComments")
#define SECONDS_BEFORE_ATTRACT		THEME->GetMetricF("ScreenTitleMenu","SecondsBeforeAttract")
#define HELP_TEXT( coin_mode )		THEME->GetMetric("ScreenTitleMenu","HelpText"+Capitalize(CoinModeToString(coin_mode)))
#define MENU_ITEM_CREATE			THEME->GetMetric("ScreenTitleMenu","MenuCommandOnCreate")
#define MENU_ITEM_SELECT_DELAY		THEME->GetMetricF("ScreenTitleMenu","MenuCommandSelectDelay")

#define NAME( sChoiceName )			THEME->GetMetric (m_sName,ssprintf("Name%s",sChoiceName.c_str()))

const ScreenMessage SM_PlayComment			=	ScreenMessage(SM_User+1);
const ScreenMessage SM_GoToAttractLoop		=	ScreenMessage(SM_User+13);

ScreenTitleMenu::ScreenTitleMenu( CString sClassName ) : ScreenSelect( sClassName )
{
	LOG->Trace( "ScreenTitleMenu::ScreenTitleMenu()" );

	// Don't show screen title menu (says "Press Start") 
	// if there are 0 credits and inserted and CoinMode is pay.
	if( PREFSMAN->GetCoinMode() == COIN_PAY  &&
		GAMESTATE->m_iCoins < PREFSMAN->m_iCoinsPerCredit )
	{
		SCREENMAN->SetNewScreen( INITIAL_SCREEN );
		return;
	}

	/* XXX We really need two common calls: 1, something run when exiting from gameplay
	 * (to do this reset), and 2, something run when entering gameplay, to apply default
	 * options.  Having special cases in attract screens and the title menu to reset
	 * things stinks ... */
	GAMESTATE->Reset();

	LIGHTSMAN->SetLightsMode( LIGHTSMODE_JOINING );	// do this after Reset!


	m_sprLogo.Load( THEME->GetPathG("ScreenLogo",GAMESTATE->GetCurrentGame()->m_szName) );
	m_sprLogo.Command( PREFSMAN->GetCoinMode()==COIN_HOME ? LOGO_HOME_ON_COMMAND : LOGO_ON_COMMAND );
	this->AddChild( &m_sprLogo );

	if( PREFSMAN->GetCoinMode() != COIN_HOME )
	{
		switch( PREFSMAN->GetPremium() )
		{
		case PrefsManager::DOUBLES_PREMIUM:
			m_Premium.LoadFromAniDir( THEME->GetPathToB("ScreenTitleMenu doubles premium") );
			this->AddChild( &m_Premium );
			break;
		case PrefsManager::JOINT_PREMIUM:
			m_Premium.LoadFromAniDir( THEME->GetPathToB("ScreenTitleMenu joint premium") );
			this->AddChild( &m_Premium );
			break;
		}
	}

	m_textVersion.LoadFromFont( THEME->GetPathToF("Common normal") );
	m_textVersion.Command( VERSION_ON_COMMAND );
	m_textVersion.SetText( PRODUCT_VER );
	this->AddChild( &m_textVersion );


	m_textSongs.LoadFromFont( THEME->GetPathToF("Common normal") );
	m_textSongs.Command( SONGS_ON_COMMAND );
	CString text = ssprintf("%d songs in %d groups, %d courses", SONGMAN->GetNumSongs(), SONGMAN->GetNumGroups(), SONGMAN->GetNumCourses() );
	if( PREFSMAN->m_bUseUnlockSystem )
		text += ssprintf(", %d unlocks", UNLOCKMAN->GetNumUnlocks() );
	m_textSongs.SetText( text );
	this->AddChild( &m_textSongs );

	m_textMaxStages.LoadFromFont( THEME->GetPathF(m_sName,"MaxStages") );
	m_textMaxStages.Command( MAX_STAGES_ON_COMMAND );
	CString sText = 
		PREFSMAN->m_bEventMode ?
		CString("event mode") :
		ssprintf( "%d %s%s max", PREFSMAN->m_iNumArcadeStages, MAX_STAGES_TEXT.c_str(), (PREFSMAN->m_iNumArcadeStages>1)?"s":"" );
	m_textMaxStages.SetText( sText );
	this->AddChild( &m_textMaxStages );

	m_textLifeDifficulty.LoadFromFont( THEME->GetPathF(m_sName,"LifeDifficulty") );
	m_textLifeDifficulty.Command( LIFE_DIFFICULTY_ON_COMMAND );
	int iLifeDifficulty;
	const CStringArray dummy;
	LifeDifficulty( iLifeDifficulty, true, dummy );	
	iLifeDifficulty++;	// LifeDifficulty returns an index
	m_textLifeDifficulty.SetText( ssprintf( "life difficulty %d", iLifeDifficulty ) );
	this->AddChild( &m_textLifeDifficulty );

	CString sCoinMode = CoinModeToString((CoinMode)PREFSMAN->GetCoinMode());
	m_CoinMode.LoadFromAniDir( THEME->GetPathToB("ScreenTitleMenu "+sCoinMode) );
	this->AddChild( &m_CoinMode );
	
	m_textHelp.LoadFromFont( THEME->GetPathToF("ScreenTitleMenu help") );
	m_textHelp.SetText( HELP_TEXT((CoinMode)PREFSMAN->GetCoinMode()) );
	m_textHelp.SetXY( HELP_X, HELP_Y );
	m_textHelp.SetZoom( 0.5f );
	m_textHelp.SetEffectDiffuseBlink();
	m_textHelp.SetShadowLength( 2 );
	this->AddChild( &m_textHelp );

	switch( PREFSMAN->GetCoinMode() )
	{
	case COIN_HOME:
		{
			for( unsigned i=0; i<m_aModeChoices.size(); i++ )
			{
				ModeChoice &mc = m_aModeChoices[i];

				m_textChoice[i].LoadFromFont( THEME->GetPathToF("ScreenTitleMenu choices") );
				m_textChoice[i].SetHorizAlign( (enum Actor::HorizAlign)MENU_TEXT_ALIGN );
				m_textChoice[i].SetText( NAME(mc.m_sName) );
				m_textChoice[i].SetXY( CHOICES_X, CHOICES_START_Y + i*CHOICES_SPACING_Y );
				m_textChoice[i].SetShadowLength( CHOICES_SHADOW_LENGTH );
				this->AddChild( &m_textChoice[i] );
			}	
		}
		break;
	case COIN_PAY:
	case COIN_FREE:
		break;
	default:
		ASSERT(0);
	}


//	m_AttractOut.Load( THEME->GetPathToB("ScreenTitleMenu out") );
//	this->AddChild( &m_AttractOut );

//	m_BeginOut.Load( THEME->GetPathToB("ScreenTitleMenu begin out") );
//	this->AddChild( &m_BeginOut );


	SOUND->PlayOnceFromAnnouncer( "title menu game name" );


	m_soundChange.Load( THEME->GetPathToS("ScreenTitleMenu change"),true );	

	m_Choice = 0;

	for( unsigned i=0; i<m_aModeChoices.size(); i++ )
		if( i != m_Choice )
		{
			m_textChoice[i].SetZoom( ZOOM_NOT_SELECTED );
			m_textChoice[i].Command( MENU_ITEM_CREATE );
		}
		else
			GainFocus( m_Choice );

	SOUND->PlayMusic( THEME->GetPathToS("ScreenTitleMenu music") );

	this->PostScreenMessage( SM_PlayComment, SECONDS_BETWEEN_COMMENTS);

	this->SortByDrawOrder();

//	this->MoveToTail( &m_AttractOut );	// put it in the back so it covers up the stuff we just added
}

ScreenTitleMenu::~ScreenTitleMenu()
{
	LOG->Trace( "ScreenTitleMenu::~ScreenTitleMenu()" );
}

void ScreenTitleMenu::UpdateSelectableChoices()
{

}

void ScreenTitleMenu::MoveCursor( bool up )
{
	if( PREFSMAN->GetCoinMode() != COIN_HOME )
		return;
//	if( m_BeginOut.IsTransitioning() )
//		return;
	
	TimeToDemonstration.GetDeltaTime();	/* Reset the demonstration timer when a key is pressed. */
	LoseFocus( m_Choice );
	m_Choice += (up? -1:+1);
	wrap( (int&)m_Choice, m_aModeChoices.size() );
	m_soundChange.Play();
	GainFocus( m_Choice );
}

void ScreenTitleMenu::Input( const DeviceInput& DeviceI, const InputEventType type, const GameInput &GameI, const MenuInput &MenuI, const StyleInput &StyleI )
{
	LOG->Trace( "ScreenTitleMenu::Input( %d-%d )", DeviceI.device, DeviceI.button );	// debugging gameport joystick problem

	if( type != IET_FIRST_PRESS )
		return;

	/* If the CoinMode was changed, we need to reload this screen
	 * so that the right m_aModeChoices will show */
	if( ScreenAttract::ChangeCoinModeInput( DeviceI, type, GameI, MenuI, StyleI ) )
	{
		SCREENMAN->SetNewScreen( "ScreenTitleMenu" );
		return;
	}


	if( m_In.IsTransitioning() || m_Back.IsTransitioning() ) /* not m_Out */
		return;

	if( MenuI.IsValid() )
	{
		switch( MenuI.button )
		{
		case MENU_BUTTON_UP:
		case MENU_BUTTON_LEFT:
			MoveCursor( true );
			break;
		case MENU_BUTTON_DOWN:
		case MENU_BUTTON_RIGHT:
			MoveCursor( false );
			break;
		case MENU_BUTTON_BACK:
			if( m_Out.IsTransitioning() )
				break;
			Back( SM_GoToAttractLoop );
			break;
		case MENU_BUTTON_START:
			/* return if the choice is invalid */
			const ModeChoice &mc = m_aModeChoices[m_Choice];
			CString why;
			if( !mc.IsPlayable( &why ) )
			{
				SCREENMAN->PlayInvalidSound();
				if( why != "" )
					SCREENMAN->SystemMessage( why );
				return;
			}

			if( !Screen::JoinInput( DeviceI, type, GameI, MenuI, StyleI ) )
				return;

			if( !m_Out.IsTransitioning() )
				StartTransitioning( SM_GoToNextScreen );
		}
	}

	// detect codes
	if( CodeDetector::EnteredCode(GameI.controller,CodeDetector::CODE_NEXT_THEME) ||
		CodeDetector::EnteredCode(GameI.controller,CodeDetector::CODE_NEXT_THEME2) )
	{
		THEME->NextTheme();
		ApplyGraphicOptions();	// update window title and icon
		SCREENMAN->SystemMessage( "Theme: "+THEME->GetCurThemeName() );
		SCREENMAN->SetNewScreen( "ScreenTitleMenu" );
		TEXTUREMAN->DoDelayedDelete();
	}
	if( CodeDetector::EnteredCode(GameI.controller,CodeDetector::CODE_NEXT_ANNOUNCER) ||
		CodeDetector::EnteredCode(GameI.controller,CodeDetector::CODE_NEXT_ANNOUNCER2) )
	{
		ANNOUNCER->NextAnnouncer();
		CString sName = ANNOUNCER->GetCurAnnouncerName();
		if( sName=="" ) sName = "(none)";
		SCREENMAN->SystemMessage( "Announcer: "+sName );
		SCREENMAN->SetNewScreen( "ScreenTitleMenu" );
	}
	if( CodeDetector::EnteredCode(GameI.controller,CodeDetector::CODE_NEXT_GAME) ||
		CodeDetector::EnteredCode(GameI.controller,CodeDetector::CODE_NEXT_GAME2) )
	{
		vector<const Game*> vGames;
		GAMEMAN->GetEnabledGames( vGames );
		ASSERT( !vGames.empty() );
		vector<const Game*>::iterator iter = find(vGames.begin(),vGames.end(),GAMESTATE->m_pCurGame);
		ASSERT( iter != vGames.end() );

		iter++;	// move to the next game

		// wrap
		if( iter == vGames.end() )
			iter = vGames.begin();

		GAMESTATE->m_pCurGame = *iter;

		/* Reload the theme if it's changed, but don't back to the initial screen. */
		ResetGame( false );

		SCREENMAN->SystemMessage( CString("Game: ") + GAMESTATE->GetCurrentGame()->m_szName );
		SCREENMAN->SetNewScreen( "ScreenTitleMenu" );
	}

	ScreenSelect::Input( DeviceI, type, GameI, MenuI, StyleI );
}

void ScreenTitleMenu::Update( float fDelta )
{
	// time out on this screen and go to the attract sequence
	if( !IsTransitioning() && TimeToDemonstration.PeekDeltaTime() >= SECONDS_BEFORE_ATTRACT)
	{
		// don't time out on this screen is coin mode is pay.  
		// If we're here, then there's a credit in the machine.
		if( PREFSMAN->GetCoinMode() == COIN_PAY )
			;	// do nothing
		else
		{
			this->PostScreenMessage( SM_GoToAttractLoop, 0 );
			TimeToDemonstration.GetDeltaTime();
		}
	}

	Screen::Update(fDelta);
}


void ScreenTitleMenu::HandleScreenMessage( const ScreenMessage SM )
{
	switch( SM )
	{
	case SM_PlayComment:
		SOUND->PlayOnceFromAnnouncer( "title menu attract" );
		this->PostScreenMessage( SM_PlayComment, SECONDS_BETWEEN_COMMENTS );
		break;
	case SM_GoToNextScreen:
		/* XXX: Bad hack: we only want to set default options if we're actually
		 * going into the game.  We don't want to set it if we're going into the
		 * editor menu, since that never resets options (so it'll maintain changes
		 * when entering and exiting the editor), and the editor probably shouldn't
		 * use default options. */
		if( m_Choice == 0 )
		{
			// apply default options
			FOREACH_PlayerNumber( p )
				GAMESTATE->m_PlayerOptions[p].FromString( PREFSMAN->m_sDefaultModifiers );
			GAMESTATE->m_SongOptions.FromString( PREFSMAN->m_sDefaultModifiers );
		}
		m_aModeChoices[m_Choice].ApplyToAllPlayers();
		break;
	case SM_GoToAttractLoop:
		SCREENMAN->SetNewScreen( INITIAL_SCREEN );
		break;
	}
}


void ScreenTitleMenu::LoseFocus( int iChoiceIndex )
{
	m_textChoice[iChoiceIndex].SetEffectNone();
	m_textChoice[iChoiceIndex].StopTweening();
	m_textChoice[iChoiceIndex].BeginTweening( 0.3f );
	m_textChoice[iChoiceIndex].SetZoom( ZOOM_NOT_SELECTED );
}

void ScreenTitleMenu::GainFocus( int iChoiceIndex )
{
	m_textChoice[iChoiceIndex].StopTweening();
	m_textChoice[iChoiceIndex].BeginTweening( 0.3f );
	m_textChoice[iChoiceIndex].SetZoom( ZOOM_SELECTED );
	RageColor color1, color2;
	color1 = COLOR_SELECTED;
	color2 = color1 * 0.5f;
	color2.a = 1;
	m_textChoice[iChoiceIndex].SetEffectDiffuseShift( 0.5f, color1, color2 );
}

/*
 * (c) 2001-2004 Chris Danford
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
