#include "global.h"
/*
-----------------------------------------------------------------------------
 Class: ScreenSelectStyle

 Desc: Testing the Screen class.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/


#include "ScreenSelectStyle.h"
#include "ScreenManager.h"
#include "PrefsManager.h"
#include "RageSoundManager.h"
#include "GameConstantsAndTypes.h"
#include "PrefsManager.h"
#include "GameManager.h"
#include "RageLog.h"
#include "AnnouncerManager.h"
#include "GameState.h"
#include "GameConstantsAndTypes.h"
#include "ThemeManager.h"


#define JOINT_PREMIUM_BANNER_X	THEME->GetMetricF("ScreenSelectStyle","JointPremiumBannerX")
#define JOINT_PREMIUM_BANNER_Y	THEME->GetMetricF("ScreenSelectStyle","JointPremiumBannerY")
#define ICONS_START_X			THEME->GetMetricF("ScreenSelectStyle","IconsStartX")
#define ICONS_SPACING_X			THEME->GetMetricF("ScreenSelectStyle","IconsSpacingX")
#define ICONS_START_Y			THEME->GetMetricF("ScreenSelectStyle","IconsStartY")
#define ICONS_SPACING_Y			THEME->GetMetricF("ScreenSelectStyle","IconsSpacingY")
#define EXPLANATION_X			THEME->GetMetricF("ScreenSelectStyle","ExplanationX")
#define EXPLANATION_Y			THEME->GetMetricF("ScreenSelectStyle","ExplanationY")
#define INFO_X					THEME->GetMetricF("ScreenSelectStyle","InfoX")
#define INFO_Y					THEME->GetMetricF("ScreenSelectStyle","InfoY")
#define PREVIEW_X				THEME->GetMetricF("ScreenSelectStyle","PreviewX")
#define PREVIEW_Y				THEME->GetMetricF("ScreenSelectStyle","PreviewY")
#define HELP_TEXT				THEME->GetMetric("ScreenSelectStyle","HelpText")
#define TIMER_SECONDS			THEME->GetMetricI("ScreenSelectStyle","TimerSeconds")
#define NEXT_SCREEN				PREFSMAN->m_bDDRExtremeDifficultySelect? THEME->GetMetric("ScreenSelectStyle","NextScreen") : "ScreenSelectDifficultyEX"



ScreenSelectStyle::ScreenSelectStyle()
{
	LOG->Trace( "ScreenSelectStyle::ScreenSelectStyle()" );

	// Reset the current style and game
	GAMESTATE->m_CurStyle = STYLE_INVALID;
	GAMESTATE->m_bPlayersCanJoin = true;

	GAMEMAN->GetStylesForGame( GAMESTATE->m_CurGame, m_aPossibleStyles );
	ASSERT( !m_aPossibleStyles.empty() );	// every game should have at least one Style, or else why have the Game? :-)
	m_iSelection = 0;

	unsigned i;
	for( i=0; i<m_aPossibleStyles.size(); i++ )
	{
		CString IconPath = THEME->GetPathToOptional("Graphics",ssprintf("select style icons %s",GAMESTATE->GetCurrentGameDef()->m_szName) );
		if(IconPath.empty())
		{
			BitmapText *b = new BitmapText;
			b->LoadFromFont( THEME->GetPathTo("Fonts","normal") );
			b->SetText(GAMEMAN->GetStyleDefForStyle( m_aPossibleStyles[i] )->m_szName);
			b->SetDiffuse(RageColor(.5,.5,.5,1));
			b->SetZoom(0.5f);
			m_sprIcon[i] = b;
			IconsAreText = true;
		}
		else
		{
			Sprite *s = new Sprite;
			s->Load( IconPath );
			s->StopAnimating();
			s->SetState( i );
			m_sprIcon[i] = s;
			IconsAreText = false;
		}

		m_sprIcon[i]->SetXY( ICONS_START_X + i*ICONS_SPACING_X, ICONS_START_Y + i*ICONS_SPACING_Y );
		this->AddChild( m_sprIcon[i] );
	}

	UpdateEnabledDisabled();

	m_sprExplanation.Load( THEME->GetPathTo("Graphics","select style explanation") );
	m_sprExplanation.SetXY( EXPLANATION_X, EXPLANATION_Y );
	this->AddChild( &m_sprExplanation );
	
	m_sprPreview.SetXY( PREVIEW_X, PREVIEW_Y );
	this->AddChild( &m_sprPreview );
	
	m_sprInfo.SetXY( INFO_X, INFO_Y );
	this->AddChild( &m_sprInfo );
	
	if( PREFSMAN->m_bJointPremium )
	{
		m_sprJointPremium.Load( THEME->GetPathTo("Graphics","select style joint premium banner") );
		m_sprJointPremium.SetXY( JOINT_PREMIUM_BANNER_X, JOINT_PREMIUM_BANNER_Y );
		this->AddChild( &m_sprJointPremium );
	}

	const GameDef* pGameDef = GAMESTATE->GetCurrentGameDef();
	const StyleDef* pStyleDef = GAMEMAN->GetStyleDefForStyle( m_aPossibleStyles[m_iSelection] );

	// Load dummy Sprites
	for( i=0; i<m_aPossibleStyles.size(); i++ )
	{
		m_sprDummyPreview[i].Load( THEME->GetPathToOptional("Graphics",ssprintf("select style preview %s %s",pGameDef->m_szName,pStyleDef->m_szName)) );
		m_sprDummyInfo[i].Load(    THEME->GetPathToOptional("Graphics",ssprintf("select style info %s %s",pGameDef->m_szName,pStyleDef->m_szName)) );
	}


	m_Menu.Load( 	
		THEME->GetPathTo("BGAnimations","select style"), 
		THEME->GetPathTo("Graphics","select style top edge"),
		HELP_TEXT, false, true, TIMER_SECONDS
		);
	this->AddChild( &m_Menu );

	m_soundChange.Load( THEME->GetPathTo("Sounds","select style change") );
	m_soundSelect.Load( THEME->GetPathTo("Sounds","menu start") );


	SOUNDMAN->PlayOnceFromDir( ANNOUNCER->GetPathTo("select style intro") );

	SOUNDMAN->PlayMusic( THEME->GetPathTo("Sounds","select style music") );

	AfterChange();
	TweenOnScreen();
	m_Menu.TweenOnScreenFromBlack( SM_None );
}


ScreenSelectStyle::~ScreenSelectStyle()
{
	LOG->Trace( "ScreenSelectStyle::~ScreenSelectStyle()" );
	for( unsigned i=0; i<m_aPossibleStyles.size(); i++ )
		delete m_sprIcon[i];
}

void ScreenSelectStyle::DrawPrimitives()
{
	m_Menu.DrawBottomLayer();
	Screen::DrawPrimitives();
	m_Menu.DrawTopLayer();
}

void ScreenSelectStyle::Input( const DeviceInput& DeviceI, const InputEventType type, const GameInput &GameI, const MenuInput &MenuI, const StyleInput &StyleI )
{
	LOG->Trace( "ScreenSelectStyle::Input()" );

	if( m_Menu.IsClosing() )
		return;

	Screen::Input( DeviceI, type, GameI, MenuI, StyleI );	// default input handler
}

void ScreenSelectStyle::HandleScreenMessage( const ScreenMessage SM )
{
	Screen::HandleScreenMessage( SM );

	switch( SM )
	{
	case SM_MenuTimer:
		MenuStart(PLAYER_INVALID);
		break;
	case SM_GoToPrevScreen:
		SOUNDMAN->StopMusic();
		SCREENMAN->SetNewScreen( "ScreenTitleMenu" );
		break;
	case SM_GoToNextScreen:
		SCREENMAN->SetNewScreen( NEXT_SCREEN );
		break;
	}
}

void ScreenSelectStyle::BeforeChange()
{
	m_sprIcon[m_iSelection]->SetEffectNone();
}

void ScreenSelectStyle::AfterChange()
{
	m_sprIcon[m_iSelection]->SetEffectGlowShift();

	const GameDef* pGameDef = GAMESTATE->GetCurrentGameDef();
	const StyleDef* pStyleDef = GAMEMAN->GetStyleDefForStyle( m_aPossibleStyles[m_iSelection] );

	// Tween Preview
	m_sprPreview.Load( THEME->GetPathToOptional("Graphics",ssprintf("select style preview %s %s",pGameDef->m_szName,pStyleDef->m_szName)) );

	m_sprPreview.StopTweening();
	m_sprPreview.SetGlow( RageColor(1,1,1,0) );
	m_sprPreview.SetDiffuse( RageColor(1,1,1,0) );

	m_sprPreview.BeginTweening( 0.25f );			// sleep

	m_sprPreview.BeginTweening( 0.2f );			// fade to white
	m_sprPreview.SetTweenGlow( RageColor(1,1,1,1) );
	m_sprPreview.SetTweenDiffuse( RageColor(1,1,1,0) );

	m_sprPreview.BeginTweening( 0.01f );			// turn color on
	m_sprPreview.SetTweenDiffuse( RageColor(1,1,1,1) );

	m_sprPreview.BeginTweening( 0.2f );			// fade to color
	m_sprPreview.SetTweenGlow( RageColor(1,1,1,0) );
	m_sprPreview.SetTweenDiffuse( RageColor(1,1,1,1) );


	// Tween Info
	m_sprInfo.Load( THEME->GetPathToOptional("Graphics",ssprintf("select style info %s %s",pGameDef->m_szName,pStyleDef->m_szName)) );
	m_sprInfo.StopTweening();
	m_sprInfo.SetZoomY( 0 );
	m_sprInfo.BeginTweening( 0.5f, Actor::TWEEN_BOUNCE_END );
	m_sprInfo.SetTweenZoomY( 1 );
}

void ScreenSelectStyle::MenuLeft( PlayerNumber pn )
{
	// search for a style to the left of the current selection that is enabled
	int iSwitchToStyleIndex = -1;	// -1 means none found
	for( int i=m_iSelection-1; i>=0; i-- )
	{
		if( IsEnabled(i) )
		{
			iSwitchToStyleIndex = i;
			break;
		}
	}

	if( iSwitchToStyleIndex == -1 )
		return;

	BeforeChange();
	m_iSelection = iSwitchToStyleIndex;
	m_soundChange.PlayRandom();
	AfterChange();
}


void ScreenSelectStyle::MenuRight( PlayerNumber pn )
{
	// search for a style to the right of the current selection that is enabled
	int iSwitchToStyleIndex = -1;	// -1 means none found
	for( unsigned i=m_iSelection+1; i<m_aPossibleStyles.size(); i++ )	
	{
		if( IsEnabled(i) )
		{
			iSwitchToStyleIndex = i;
			break;
		}
	}

	if( iSwitchToStyleIndex == -1 )
		return;

	BeforeChange();
	m_iSelection = iSwitchToStyleIndex;
	m_soundChange.PlayRandom();
	AfterChange();
}

void ScreenSelectStyle::MenuStart( PlayerNumber pn )
{
	if( pn!=PLAYER_INVALID  && !GAMESTATE->m_bSideIsJoined[pn] )
	{
		/* I think JP should allow playing two-pad singleplayer modes (doubles),
		 * but not two-pad two-player modes (battle).  (Battle mode isn't "joint".)
		 * That means we should leave player-entry logic alone and simply enable
		 * couples mode if JP is on and only one person has clicked in.  (However,
		 * that means we'll display couples even if we don't really know if we have
		 * a second pad, which is a little annoying.) 
		 *
		 * Also, credit deduction should be handled in StepMania.cpp (along with
		 * the coin logic) using GAMESTATE->m_bPlayersCanJoin, since there
		 * are other screens you can join (eg ScreenCaution). -glenn */
		if( PREFSMAN->m_CoinMode == PrefsManager::COIN_PAY )
		{
			if( !PREFSMAN->m_bJointPremium )
			{
				if( GAMESTATE->m_iCoins < PREFSMAN->m_iCoinsPerCredit )
				{
					/* Joint Premium is NOT enabled, and we do not have enough credits */
					return;
				}

				/* Joint Premium is NOT enabled, but we have enough credits. Pay up! */
				GAMESTATE->m_iCoins -= PREFSMAN->m_iCoinsPerCredit;
			}
		}

		/* If credits had to be used, it's already taken care of.. add the player */
		SOUNDMAN->PlayOnce( THEME->GetPathTo("Sounds","menu start") );
		GAMESTATE->m_bSideIsJoined[pn] = true;
		SCREENMAN->RefreshCreditsMessages();
		UpdateEnabledDisabled();
		return; // don't fall through
	}

	GAMESTATE->m_CurStyle = GetSelectedStyle();

	CString sCurStyleName = GAMESTATE->GetCurrentStyleDef()->m_szName;
	sCurStyleName.MakeLower();
	if(	     -1!=sCurStyleName.Find("single") )	SOUNDMAN->PlayOnceFromDir( ANNOUNCER->GetPathTo("select style comment single") );
	else if( -1!=sCurStyleName.Find("versus") )	SOUNDMAN->PlayOnceFromDir( ANNOUNCER->GetPathTo("select style comment versus") );
	else if( -1!=sCurStyleName.Find("double") )	SOUNDMAN->PlayOnceFromDir( ANNOUNCER->GetPathTo("select style comment double") );
	else if( -1!=sCurStyleName.Find("couple") )	SOUNDMAN->PlayOnceFromDir( ANNOUNCER->GetPathTo("select style comment couple") );
	else if( -1!=sCurStyleName.Find("solo") )	SOUNDMAN->PlayOnceFromDir( ANNOUNCER->GetPathTo("select style comment solo") );

	m_Menu.TweenOffScreenToMenu( SM_GoToNextScreen );
	GAMESTATE->m_bPlayersCanJoin = false;
	SCREENMAN->RefreshCreditsMessages();

	m_soundSelect.PlayRandom();

	m_Menu.StopTimer();

	TweenOffScreen();
}

void ScreenSelectStyle::MenuBack( PlayerNumber pn )
{
	SOUNDMAN->StopMusic();

	m_Menu.TweenOffScreenToBlack( SM_GoToPrevScreen, true );

//	m_Fade.CloseWipingLeft( SM_GoToPrevScreen );

//	TweenOffScreen();
}


void ScreenSelectStyle::TweenOnScreen() 
{
	for( unsigned i=0; i<m_aPossibleStyles.size(); i++ )
		m_sprIcon[i]->FadeOn( (m_aPossibleStyles.size()-i)*0.05f, "Left Accelerate", MENU_ELEMENTS_TWEEN_TIME );

	m_sprExplanation.FadeOn( 0, "Right Accelerate", MENU_ELEMENTS_TWEEN_TIME );
	m_sprJointPremium.FadeOn( 0, "FadeIn", MENU_ELEMENTS_TWEEN_TIME );

	// let AfterChange tween Preview and Info
}

void ScreenSelectStyle::TweenOffScreen()
{
	for( unsigned i=0; i<m_aPossibleStyles.size(); i++ )
		m_sprIcon[i]->FadeOff( 0, "FoldY", MENU_ELEMENTS_TWEEN_TIME );

	m_sprExplanation.FadeOff( 0, "FoldY", MENU_ELEMENTS_TWEEN_TIME );

	m_sprPreview.FadeOff( 0, "FoldY", MENU_ELEMENTS_TWEEN_TIME );

	m_sprInfo.FadeOff( 0, "FoldY", MENU_ELEMENTS_TWEEN_TIME );

	m_sprJointPremium.FadeOff( 0, "FadeOut", MENU_ELEMENTS_TWEEN_TIME );
}


bool ScreenSelectStyle::IsEnabled( int iStyleIndex )
{
	Style style = m_aPossibleStyles[iStyleIndex];

	int iNumSidesJoined = 0;
	for( int c=0; c<2; c++ )
		if( GAMESTATE->m_bSideIsJoined[c] )
			iNumSidesJoined++;	// left side, and right side

	switch( GAMEMAN->GetStyleDefForStyle(style)->m_StyleType )
	{
	case StyleDef::ONE_PLAYER_ONE_CREDIT:	return iNumSidesJoined==1;
	case StyleDef::ONE_PLAYER_TWO_CREDITS:	return iNumSidesJoined==2;
	case StyleDef::TWO_PLAYERS_TWO_CREDITS:	return iNumSidesJoined==2;
	default:	ASSERT(0);	return false;
	}
}

void ScreenSelectStyle::UpdateEnabledDisabled()
{
	unsigned i;
	/* XXX: If a player joins during the tween-in, this diffuse change
	 * will be undone by the tween.  Hmm. */
	for( i=0; i<m_aPossibleStyles.size(); i++ )
	{
		/* If the icon is text, use a dimmer diffuse, or we won't be
		 * able to see the glow. */
		if( IsEnabled(i) )
			m_sprIcon[i]->SetDiffuse( IconsAreText? RageColor(0.75f, 0.75f, 0.75f, 1):
													RageColor(1,1,1,1) );
		else
			m_sprIcon[i]->SetDiffuse( RageColor(0.25f,0.25f,0.25f,1) );
	}

	// Select first enabled style
	BeforeChange();

	int iSwitchToStyleIndex = -1;	// -1 means none found
	for( i=0; i<m_aPossibleStyles.size(); i++ )
	{
		if( IsEnabled(i) )
		{
			iSwitchToStyleIndex = i;
			break;
		}
	}
	ASSERT( iSwitchToStyleIndex != -1 );	// no styles are enabled.  We're stuck!  This should never happen

	m_iSelection = iSwitchToStyleIndex;
	AfterChange();
}

