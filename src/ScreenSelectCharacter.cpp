#include "global.h"
#include "ScreenSelectCharacter.h"
#include "ScreenManager.h"
#include "GameSoundManager.h"
#include "RageUtil.h"
#include "RageLog.h"
#include "ThemeManager.h"
#include "AnnouncerManager.h"
#include "GameState.h"
#include "Character.h"
#include "PrefsManager.h"
#include "RageTextureManager.h"
#include "CharacterManager.h"
#include "InputEventPlus.h"

#include <vector>


#define TITLE_ON_COMMAND( p )				THEME->GetMetricA("ScreenSelectCharacter",ssprintf("TitleP%dOnCommand",p+1))
#define TITLE_OFF_COMMAND( p )				THEME->GetMetricA("ScreenSelectCharacter",ssprintf("TitleP%dOffCommand",p+1))
#define CARD_ON_COMMAND( p )				THEME->GetMetricA("ScreenSelectCharacter",ssprintf("CardP%dOnCommand",p+1))
#define CARD_OFF_COMMAND( p )				THEME->GetMetricA("ScreenSelectCharacter",ssprintf("CardP%dOffCommand",p+1))
#define CARD_ARROWS_ON_COMMAND( p )			THEME->GetMetricA("ScreenSelectCharacter",ssprintf("CardArrowsP%dOnCommand",p+1))
#define CARD_ARROWS_OFF_COMMAND( p )		THEME->GetMetricA("ScreenSelectCharacter",ssprintf("CardArrowsP%dOffCommand",p+1))
#define EXPLANATION_ON_COMMAND				THEME->GetMetricA("ScreenSelectCharacter","ExplanationOnCommand")
#define EXPLANATION_OFF_COMMAND				THEME->GetMetricA("ScreenSelectCharacter","ExplanationOffCommand")
#define ATTACK_FRAME_ON_COMMAND( p )		THEME->GetMetricA("ScreenSelectCharacter",ssprintf("AttackFrameP%dOnCommand",p+1))
#define ATTACK_FRAME_OFF_COMMAND( p )		THEME->GetMetricA("ScreenSelectCharacter",ssprintf("AttackFrameP%dOffCommand",p+1))
#define ATTACK_ICON_WIDTH					THEME->GetMetricF("ScreenSelectCharacter","AttackIconWidth")
#define ATTACK_ICON_HEIGHT					THEME->GetMetricF("ScreenSelectCharacter","AttackIconHeight")
#define ATTACK_ICONS_START_X( p )			THEME->GetMetricF("ScreenSelectCharacter",ssprintf("AttackIconsP%dStartX",p+1))
#define ATTACK_ICONS_START_Y( p )			THEME->GetMetricF("ScreenSelectCharacter",ssprintf("AttackIconsP%dStartY",p+1))
#define ATTACK_ICONS_SPACING_X				THEME->GetMetricF("ScreenSelectCharacter","AttackIconsSpacingX")
#define ATTACK_ICONS_SPACING_Y				THEME->GetMetricF("ScreenSelectCharacter","AttackIconsSpacingY")
#define ATTACK_ICONS_ON_COMMAND( p )		THEME->GetMetricA("ScreenSelectCharacter",ssprintf("AttackIconsP%dOnCommand",p+1))
#define ATTACK_ICONS_OFF_COMMAND( p )		THEME->GetMetricA("ScreenSelectCharacter",ssprintf("AttackIconsP%dOffCommand",p+1))
#define TIMER_SECONDS						THEME->GetMetricI("ScreenSelectCharacter","TimerSeconds")
#define ICON_WIDTH							THEME->GetMetricF("ScreenSelectCharacter","IconWidth")
#define ICON_HEIGHT							THEME->GetMetricF("ScreenSelectCharacter","IconHeight")
#define ICONS_ON_COMMAND( p )				THEME->GetMetricA("ScreenSelectCharacter",ssprintf("IconsP%dOnCommand",p+1))
#define ICONS_OFF_COMMAND( p )				THEME->GetMetricA("ScreenSelectCharacter",ssprintf("IconsP%dOffCommand",p+1))

#define LEVEL_CURSOR_X( p, l )	( ICONS_START_X(p)+ICONS_SPACING_X*((NUM_ATTACKS_PER_LEVEL-1)/2.f) )
#define LEVEL_CURSOR_Y( p, l )	( ICONS_START_Y(p)+ICONS_SPACING_Y*l )

const PlayerNumber	CPU_PLAYER[NUM_PLAYERS] = { PLAYER_2, PLAYER_1 };



REGISTER_SCREEN_CLASS( ScreenSelectCharacter );


void ScreenSelectCharacter::Init()
{
	ScreenWithMenuElements::Init();

	std::vector<Character*> apCharacters;
	CHARMAN->GetCharacters( apCharacters );
	if( apCharacters.empty() )
	{
		this->PostScreenMessage( SM_GoToNextScreen, 0 );
		return;
	}

	switch( GAMESTATE->m_PlayMode )
	{
	// For Rave/Battle mode, we force the players to select characters
	// (by not returning in this switch)
	case PLAY_MODE_BATTLE:
	case PLAY_MODE_RAVE:
		break;

	default:
		/* Non Rave/Battle mode, just skip this screen if disabled. */
		if(	PREFSMAN->m_ShowDancingCharacters != SDC_Select )
		{
			this->PostScreenMessage( SM_GoToNextScreen, 0 );
			return;
		}
	}

	FOREACH_PlayerNumber( p )
	{
		m_iSelectedCharacter[p] = 0;
		if( GAMESTATE->IsHumanPlayer(p) )
			m_SelectionRow[p] = CHOOSING_HUMAN_CHARACTER;
	}


	FOREACH_EnabledPlayer( p )
	{
		m_sprTitle[p].Load( THEME->GetPathG("ScreenSelectCharacter","title 2x2") );
		m_sprTitle[p].SetState( GAMESTATE->IsHumanPlayer(p) ? p : 2+p );
		m_sprTitle[p].StopAnimating();
		m_sprTitle[p].RunCommands( TITLE_ON_COMMAND(p) );

		this->AddChild( &m_sprTitle[p] );

		m_sprCard[p].RunCommands( CARD_ON_COMMAND(p) );
		this->AddChild( &m_sprCard[p] );

		m_sprCardArrows[p].Load( THEME->GetPathG("ScreenSelectCharacter","card arrows") );
		m_sprCardArrows[p].RunCommands( CARD_ARROWS_ON_COMMAND(p) );
		this->AddChild( &m_sprCardArrows[p] );

		for( unsigned i=0; i<MAX_CHAR_ICONS_TO_SHOW; i++ )
		{
			m_sprIcons[p][i].ScaleToClipped( ICON_WIDTH, ICON_HEIGHT );
			this->AddChild( &m_sprIcons[p][i] );
		}

		if(GAMESTATE->m_PlayMode == PLAY_MODE_BATTLE || GAMESTATE->m_PlayMode == PLAY_MODE_RAVE)
		{
			m_sprAttackFrame[p].Load( THEME->GetPathG("ScreenSelectCharacter","attack frame 1x2") );
			m_sprAttackFrame[p].StopAnimating();
			m_sprAttackFrame[p].SetState( p );
			m_sprAttackFrame[p].RunCommands( ATTACK_FRAME_ON_COMMAND(p) );
			this->AddChild( &m_sprAttackFrame[p] );

			for( int i=0; i<NUM_ATTACK_LEVELS; i++ )
				for( int j=0; j<NUM_ATTACKS_PER_LEVEL; j++ )
				{
					float fX = ATTACK_ICONS_START_X(p) + ATTACK_ICONS_SPACING_X*j;
					float fY = ATTACK_ICONS_START_Y(p) + ATTACK_ICONS_SPACING_Y*i;
					m_AttackIcons[p][i][j].SetXY( fX, fY );
					m_AttackIcons[p][i][j].RunCommands( ATTACK_ICONS_ON_COMMAND(p) );
					this->AddChild( &m_AttackIcons[p][i][j] );
				}
		}
	}

	m_sprExplanation.Load( THEME->GetPathG("ScreenSelectCharacter","explanation") );
	m_sprExplanation.RunCommands( EXPLANATION_ON_COMMAND );
	this->AddChild( &m_sprExplanation );


	m_soundChange.Load( THEME->GetPathS("ScreenSelectCharacter","change"), true );

	SOUND->PlayOnceFromDir( ANNOUNCER->GetPathTo("select group intro") );

	FOREACH_PlayerNumber( p )
	{
		if( GAMESTATE->IsHumanPlayer(p) )
		{
			AfterRowChange( p );
			AfterValueChange( p );
		}

		for( unsigned i=0; i<MAX_CHAR_ICONS_TO_SHOW; i++ )
			m_sprIcons[p][i].RunCommands( ICONS_ON_COMMAND(p) );
	}
}


ScreenSelectCharacter::~ScreenSelectCharacter()
{
	LOG->Trace( "ScreenSelectCharacter::~ScreenSelectCharacter()" );

}


bool ScreenSelectCharacter::Input( const InputEventPlus &input )
{
	LOG->Trace( "ScreenSelectCharacter::Input()" );

	if( IsTransitioning() )
		return false;

	return ScreenWithMenuElements::Input( input );	// default input handler
}

void ScreenSelectCharacter::HandleScreenMessage( const ScreenMessage SM )
{
	if( SM == SM_BeginFadingOut )
	{
		StartTransitioningScreen( SM_GoToNextScreen );
	}
	else if( SM == SM_MenuTimer )
	{
		MakeSelection(PLAYER_1);
		if( !AllAreFinishedChoosing() )
			ResetTimer();
	}
	else
	{
		ScreenWithMenuElements::HandleScreenMessage( SM );
	}
}

PlayerNumber ScreenSelectCharacter::GetAffectedPlayerNumber( PlayerNumber pn )
{
	switch( m_SelectionRow[pn] )
	{
	case CHOOSING_HUMAN_CHARACTER:
		return pn;
	case CHOOSING_CPU_CHARACTER:
		return CPU_PLAYER[pn];
	case FINISHED_CHOOSING:
		return pn;
	default:
		FAIL_M(ssprintf("Invalid character selection state: %i", m_SelectionRow[pn]));
	}
}

void ScreenSelectCharacter::BeforeRowChange( PlayerNumber pn )
{
	PlayerNumber pnAffected = GetAffectedPlayerNumber(pn);
	switch( m_SelectionRow[pn] )
	{
		case CHOOSING_CPU_CHARACTER:
		case CHOOSING_HUMAN_CHARACTER:
			m_sprCardArrows[pnAffected].StopEffect();
			break;
		default: break;
	}
}

void ScreenSelectCharacter::AfterRowChange( PlayerNumber pn )
{
	PlayerNumber pnAffected = GetAffectedPlayerNumber(pn);
	switch( m_SelectionRow[pn] )
	{
		case CHOOSING_CPU_CHARACTER:
		case CHOOSING_HUMAN_CHARACTER:
			m_sprCardArrows[pnAffected].SetEffectGlowShift(1.0f, RageColor(1,1,1,0.2f), RageColor(1,1,1,0.8f));
			break;
		default: break;
	}
}

void ScreenSelectCharacter::AfterValueChange( PlayerNumber pn )
{
	PlayerNumber pnAffected = GetAffectedPlayerNumber(pn);
	switch( m_SelectionRow[pn] )
	{
	case CHOOSING_CPU_CHARACTER:
	case CHOOSING_HUMAN_CHARACTER:
		{
			std::vector<Character*> apCharacters;
			CHARMAN->GetCharacters( apCharacters );
			Character* pChar = apCharacters[ m_iSelectedCharacter[pnAffected] ];
			m_sprCard[pnAffected].UnloadTexture();
			m_sprCard[pnAffected].Load( pChar->GetCardPath() );

			if(GAMESTATE->m_PlayMode == PLAY_MODE_BATTLE || GAMESTATE->m_PlayMode == PLAY_MODE_RAVE)
				for( int i=0; i<NUM_ATTACK_LEVELS; i++ )
					for( int j=0; j<NUM_ATTACKS_PER_LEVEL; j++ )
					{
						m_AttackIcons[pnAffected][i][j].Load( "ScreenSelectCharacter" );
						m_AttackIcons[pnAffected][i][j].Set( pChar->m_sAttacks[i][j] );
					}

			int c = m_iSelectedCharacter[pnAffected] - MAX_CHAR_ICONS_TO_SHOW/2;
			wrap( c, apCharacters.size() );

			for( unsigned i=0; i<MAX_CHAR_ICONS_TO_SHOW; i++ )
			{
				c++;
				wrap( c, apCharacters.size() );
				Character* pCharacter = apCharacters[c];
				Banner &banner = m_sprIcons[pnAffected][i];
				banner.LoadIconFromCharacter( pCharacter );
				float fX = (pnAffected==PLAYER_1) ? 320-ICON_WIDTH : 320+ICON_WIDTH;
				float fY = SCALE( i, 0.f, MAX_CHAR_ICONS_TO_SHOW-1.f, 240-(MAX_CHAR_ICONS_TO_SHOW/2*ICON_HEIGHT), 240+(MAX_CHAR_ICONS_TO_SHOW/2*ICON_HEIGHT));
				banner.SetXY( fX, fY );
			}
		}
		break;
	case FINISHED_CHOOSING:
		;	// do nothing
		break;
	default:
		FAIL_M(ssprintf("Invalid character selection state: %i", m_SelectionRow[pn]));
	}
}


bool ScreenSelectCharacter::MenuLeft( const InputEventPlus &input )
{
	return MenuUp( input );
}

bool ScreenSelectCharacter::MenuRight( const InputEventPlus &input )
{
	return MenuDown( input );
}

bool ScreenSelectCharacter::MenuUp( const InputEventPlus &input )
{
	Move( input.pn, -1 );
	return true;
}


bool ScreenSelectCharacter::MenuDown( const InputEventPlus &input )
{
	Move( input.pn, +1 );
	return true;
}

void ScreenSelectCharacter::Move( PlayerNumber pn, int deltaValue )
{
	PlayerNumber pnAffected = GetAffectedPlayerNumber(pn);
	switch( m_SelectionRow[pn] )
	{
		case CHOOSING_CPU_CHARACTER:
		case CHOOSING_HUMAN_CHARACTER:
		{
			std::vector<Character*> apCharacters;
			CHARMAN->GetCharacters( apCharacters );
			m_iSelectedCharacter[pnAffected] += deltaValue;
			wrap( m_iSelectedCharacter[pnAffected], apCharacters.size() );
			AfterValueChange(pn);
			m_soundChange.Play(true);
			break;
		}
		default: break;
	}
}

bool ScreenSelectCharacter::AllAreFinishedChoosing() const
{
	FOREACH_HumanPlayer( p )
		if( m_SelectionRow[p] != FINISHED_CHOOSING )
			return false;
	return true;
}

bool ScreenSelectCharacter::MenuStart( const InputEventPlus &input )
{
	MakeSelection( input.pn );
	return true;
}

void ScreenSelectCharacter::MakeSelection( PlayerNumber pn )
{
	if( m_SelectionRow[pn] == FINISHED_CHOOSING )
		return;


	// change row
	BeforeRowChange(pn);
	switch( m_SelectionRow[pn] )
	{
		case CHOOSING_HUMAN_CHARACTER:
			m_SelectionRow[pn] = GAMESTATE->AnyPlayersAreCpu() ? CHOOSING_CPU_CHARACTER : FINISHED_CHOOSING;
			break;
		case CHOOSING_CPU_CHARACTER:
			m_SelectionRow[pn] = FINISHED_CHOOSING;
			break;
		default: break;
	}
	AfterRowChange(pn);
	AfterValueChange(pn);
	SCREENMAN->PlayStartSound();

	if( AllAreFinishedChoosing() )
	{
		FOREACH_PlayerNumber( p )
		{
			std::vector<Character*> apCharacters;
			CHARMAN->GetCharacters( apCharacters );
			Character* pChar = apCharacters[ m_iSelectedCharacter[p] ];
			GAMESTATE->m_pCurCharacters[p] = pChar;
		}

		StopTimer();
		this->PostScreenMessage( SM_BeginFadingOut, 0 );
	}
}

bool ScreenSelectCharacter::MenuBack( const InputEventPlus &input )
{
	Cancel( SM_GoToPrevScreen );
	return true;
}

void ScreenSelectCharacter::TweenOffScreen()
{
	ScreenWithMenuElements::TweenOffScreen();

	FOREACH_PlayerNumber( p )
	{
		m_sprCard[p].RunCommands( CARD_OFF_COMMAND(p) );
		m_sprTitle[p].RunCommands( TITLE_OFF_COMMAND(p) );
		m_sprCardArrows[p].RunCommands( CARD_ARROWS_OFF_COMMAND(p) );
		if(GAMESTATE->m_PlayMode == PLAY_MODE_BATTLE || GAMESTATE->m_PlayMode == PLAY_MODE_RAVE)
		{
			m_sprAttackFrame[p].RunCommands( ATTACK_FRAME_OFF_COMMAND(p) );
			for( int i=0; i<NUM_ATTACK_LEVELS; i++ )
				for( int j=0; j<NUM_ATTACKS_PER_LEVEL; j++ )
					m_AttackIcons[p][i][j].RunCommands( ATTACK_ICONS_OFF_COMMAND(p) );
		}
		for( unsigned i=0; i<MAX_CHAR_ICONS_TO_SHOW; i++ )
			m_sprIcons[p][i].RunCommands( ICONS_OFF_COMMAND(p) );
	}
	m_sprExplanation.RunCommands( EXPLANATION_OFF_COMMAND );
}

/*
 * (c) 2003-2004 Chris Danford
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
