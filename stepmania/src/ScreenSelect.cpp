#include "global.h"
/*
-----------------------------------------------------------------------------
 Class: ScreenSelect

 Desc: See Header.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/


#include "ScreenSelect.h"
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
#include "ModeChoice.h"


#define CHOICES_TYPE			THEME->GetMetricI(m_sClassName,"ChoicesType")
#define CHOICES					THEME->GetMetric (m_sClassName,"Choices")
#define HELP_TEXT				THEME->GetMetric (m_sClassName,"HelpText")
#define TIMER_SECONDS			THEME->GetMetricI(m_sClassName,"TimerSeconds")
#define NEXT_SCREEN				THEME->GetMetric (m_sClassName,"NextScreen")
#define NEXT_SCREEN_ARCADE		THEME->GetMetric (m_sClassName,"NextScreenArcade")
#define NEXT_SCREEN_ONI			THEME->GetMetric (m_sClassName,"NextScreenOni")
#define NEXT_SCREEN_BATTLE		THEME->GetMetric (m_sClassName,"NextScreenBattle")


ScreenSelect::ScreenSelect( CString sClassName )
{
	LOG->Trace( "ScreenSelect::ScreenSelect()" );

	m_sClassName = sClassName;


	m_type = (ChoicesType)CHOICES_TYPE;
	m_sClassName = sClassName;


	GAMESTATE->m_bPlayersCanJoin = (m_type==CHOICES_STYLE)||(m_type==CHOICES_MODE);


	switch( m_type )
	{
	case CHOICES_STYLE:
		{
			vector<Style> aStyles;
			GAMEMAN->GetStylesForGame( GAMESTATE->m_CurGame, aStyles );
			ASSERT( !aStyles.empty() );	// every game should have at least one Style, or else why have the Game? :-)

			for( unsigned s=0; s<aStyles.size(); s++ )
			{
				Style style = aStyles[s];
				const StyleDef* pStyleDef = GAMEMAN->GetStyleDefForStyle(aStyles[s]);

				int iNumSidesJoinedToPlay;
				switch( pStyleDef->m_StyleType )
				{
				case StyleDef::ONE_PLAYER_ONE_CREDIT:	iNumSidesJoinedToPlay = 1;	break;
				case StyleDef::TWO_PLAYERS_TWO_CREDITS:	iNumSidesJoinedToPlay = 2;	break;
				case StyleDef::ONE_PLAYER_TWO_CREDITS:	iNumSidesJoinedToPlay = 2;	break;
				default:	ASSERT(0);	iNumSidesJoinedToPlay = 1;	
				}
				
				ModeChoice mc = {
					GAMESTATE->m_CurGame,
					style,
					PLAY_MODE_INVALID,
					DIFFICULTY_INVALID,
					"",
					iNumSidesJoinedToPlay			
				};
				strcpy( mc.name, pStyleDef->m_szName );
				m_aModeChoices.push_back( mc );
			}
		}
		break;
	case CHOICES_MODE:
		{
			CStringArray asChoices;
			split( CHOICES, ",", asChoices );

			for( unsigned c=0; c<asChoices.size(); c++ )
			{
				ModeChoice mc = {	// fill this in below
					GAMESTATE->m_CurGame,
					STYLE_INVALID,
					PLAY_MODE_INVALID,
					DIFFICULTY_INVALID,
					"",
					1 };

				CStringArray asBits;
				split( asChoices[c], "-", asBits );

				if( asBits.size() != 3 )
					RageException::Throw( 
						"The mode choice '%s' is invalid.  Mode choices must be in the format "
						"'style-playmode-difficulty'", asChoices[c].GetString() );

				mc.game = GAMESTATE->m_CurGame;

				mc.style = GAMEMAN->GameAndStringToStyle( mc.game, asBits[0] );
				if( mc.style == STYLE_INVALID )
					RageException::Throw( 
						"The specified style '%s' in the mode choice '%s' is invalid.", asBits[0].GetString(), asChoices[c].GetString() );
		
				mc.pm = StringToPlayMode( asBits[1] );
				if( mc.pm == PLAY_MODE_INVALID )
					RageException::Throw( 
						"The specified play mode '%s' in the mode choice '%s' is invalid.", asBits[1].GetString(), asChoices[c].GetString() );

				mc.dc = StringToDifficulty( asBits[2] );
				if( mc.dc == DIFFICULTY_INVALID )
					RageException::Throw( 
						"The specified difficulty '%s' in the mode choice '%s' is invalid.", asBits[2].GetString(), asChoices[c].GetString() );

				strcpy( mc.name, asChoices[c] );
				
				const StyleDef* pStyleDef = GAMEMAN->GetStyleDefForStyle(mc.style);
				switch( pStyleDef->m_StyleType )
				{
				case StyleDef::ONE_PLAYER_ONE_CREDIT:
					mc.numSidesJoinedToPlay = 1;
					break;
				case StyleDef::TWO_PLAYERS_TWO_CREDITS:
				case StyleDef::ONE_PLAYER_TWO_CREDITS:
					mc.numSidesJoinedToPlay = 2;
					break;
				default:
					ASSERT(0);
				}
	
				m_aModeChoices.push_back( mc );
			}
		}
		break;
	case CHOICES_DIFFICULTY:
		{
			if( GAMESTATE->m_CurStyle == STYLE_INVALID )
				RageException::Throw( "The current Style has not been set.  A theme must set the style before it can display a difficulty selection screen." );

			CStringArray asBits;
			split( CHOICES, ",", asBits );

			unsigned choice;

			for( choice=0; choice<asBits.size(); choice++ )
			{
				Difficulty dc = StringToDifficulty(asBits[choice]);
				PlayMode pm = StringToPlayMode(asBits[choice]);
				
				if( dc!=DIFFICULTY_INVALID )	// valid difficulty
				{
					ModeChoice mc = {
						GAMESTATE->m_CurGame,
						GAMESTATE->m_CurStyle,
						PLAY_MODE_ARCADE,
						dc,
						"",
						GAMESTATE->GetNumSidesJoined() };
					strcpy( mc.name, DifficultyToString(dc) );
					m_aModeChoices.push_back( mc );
				}
				else if( pm!=PLAY_MODE_INVALID )	// valid play mode
				{
					ModeChoice mc = {
						GAMESTATE->m_CurGame,
						GAMESTATE->m_CurStyle,
						pm,
						DIFFICULTY_MEDIUM,
						"",
						GAMESTATE->GetNumSidesJoined() };
					strcpy( mc.name, PlayModeToString(pm) );
					m_aModeChoices.push_back( mc );
				}
				else
					RageException::Throw( "Invalid Choice%d value '%s'.", choice+1, asBits[choice].GetString() );
			}
		}
		break;
	case CHOICES_GAME:
		ASSERT(0);	// TODO
		break;
	}
	

	for( unsigned c=0; c<m_aModeChoices.size(); c++ )
	{
		CString sBGAnimationDir = THEME->GetPathToOptional("BGAnimations",ssprintf("%s %s",m_sClassName.GetString(),m_aModeChoices[c].name));
		if( sBGAnimationDir == "" )
			sBGAnimationDir = THEME->GetPathTo("BGAnimations",m_sClassName.GetString());
		m_BGAnimations[c].LoadFromAniDir( sBGAnimationDir );
	}

	m_Menu.Load( 	
		"",		// don't load any BGAnimation.  We have our own.
		THEME->GetPathTo("Graphics",m_sClassName+" top edge"),
		HELP_TEXT, false, true, TIMER_SECONDS
		);
	this->AddChild( &m_Menu );

	SOUNDMAN->PlayOnceFromDir( ANNOUNCER->GetPathTo(m_sClassName+" intro") );

	SOUNDMAN->PlayMusic( THEME->GetPathTo("Sounds",m_sClassName+" music") );

	switch( m_type )
	{
	case CHOICES_STYLE:
	case CHOICES_MODE:
		// This is probably the first screen in the menu sequence.
		m_Menu.TweenOnScreenFromBlack( SM_None );
		break;
	case CHOICES_DIFFICULTY:
		// This is probably not the first screen in the menu sequence.
		m_Menu.TweenOnScreenFromMenu( SM_None );
		break;
	}
}


ScreenSelect::~ScreenSelect()
{
	LOG->Trace( "ScreenSelect::~ScreenSelect()" );
}

void ScreenSelect::Update( float fDelta )
{
	Screen::Update( fDelta );
	m_BGAnimations[ this->GetSelectionIndex(GAMESTATE->m_MasterPlayerNumber) ].Update( fDelta );
}

void ScreenSelect::DrawPrimitives()
{
	m_BGAnimations[ this->GetSelectionIndex(GAMESTATE->m_MasterPlayerNumber) ].Draw();
	m_Menu.DrawBottomLayer();
	Screen::DrawPrimitives();
	m_Menu.DrawTopLayer();
}

void ScreenSelect::Input( const DeviceInput& DeviceI, const InputEventType type, const GameInput &GameI, const MenuInput &MenuI, const StyleInput &StyleI )
{
	LOG->Trace( "ScreenSelect::Input()" );

	if( MenuI.IsValid() && MenuI.button==MENU_BUTTON_START )
	{
		PlayerNumber pn = MenuI.player;
		switch( m_type )
		{
		case CHOICES_STYLE:
		case CHOICES_MODE:
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
				this->UpdateSelectableChoices();
				return;
			}
			break;
		}
	}

	if( m_Menu.IsClosing() )
		return;

	Screen::Input( DeviceI, type, GameI, MenuI, StyleI );	// default input handler
}

void ScreenSelect::HandleScreenMessage( const ScreenMessage SM )
{
	switch( SM )
	{
	case SM_AllDoneChoosing:		
		{
			for( int p=0; p<NUM_PLAYERS; p++ )
				if( GAMESTATE->IsPlayerEnabled((PlayerNumber)p) )
					GAMESTATE->ApplyModeChoice( m_aModeChoices[this->GetSelectionIndex((PlayerNumber)p)], (PlayerNumber)p );

			GAMESTATE->m_bPlayersCanJoin = false;
			SCREENMAN->RefreshCreditsMessages();

			m_Menu.StopTimer();

			m_Menu.TweenOffScreenToMenu( SM_GoToNextScreen );
		}
		break;
	case SM_MenuTimer:
		{
			for( int p=0; p<NUM_PLAYERS; p++ )
				if( GAMESTATE->IsPlayerEnabled(p) )
					MenuStart( (PlayerNumber)p );
		}
		break;
	case SM_GoToPrevScreen:
		SOUNDMAN->StopMusic();
		SCREENMAN->SetNewScreen( "ScreenTitleMenu" );
		break;
	case SM_GoToNextScreen:
		switch( m_type )
		{
		case CHOICES_STYLE:
			// if they only chose a style, then the play mode isn't set.
			SCREENMAN->SetNewScreen( NEXT_SCREEN );
			break;
		case CHOICES_MODE:
		case CHOICES_DIFFICULTY:
			// go to a specific screen depending on the play mode
			switch( GAMESTATE->m_PlayMode )
			{
			case PLAY_MODE_ARCADE:	SCREENMAN->SetNewScreen( NEXT_SCREEN_ARCADE );	break;
			case PLAY_MODE_NONSTOP:
			case PLAY_MODE_ONI:
			case PLAY_MODE_ENDLESS:	SCREENMAN->SetNewScreen( NEXT_SCREEN_ONI );		break;
			case PLAY_MODE_BATTLE:	SCREENMAN->SetNewScreen( NEXT_SCREEN_BATTLE );	break;
			default:	ASSERT(0);
			}
			break;
		}
		break;
	}
}

void ScreenSelect::MenuBack( PlayerNumber pn )
{
	SOUNDMAN->StopMusic();

	m_Menu.TweenOffScreenToBlack( SM_GoToPrevScreen, true );
}
